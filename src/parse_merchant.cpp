#include "tv/parse_merchant.hpp"
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace tv {

static bool has_lower(const std::string &s) {
  for (char c : s)
    if (std::islower((unsigned char)c))
      return true;
  return false;
}

static std::string trim(const std::string &s) {
  auto b = s.find_first_not_of(" \t");
  if (b == std::string::npos)
    return {};
  auto e = s.find_last_not_of(" \t");
  return s.substr(b, e - b + 1);
}

static bool has_digit(const std::string &s) {
  for (char c : s)
    if (std::isdigit((unsigned char)c))
      return true;
  return false;
}

static std::string normalize_spaces(const std::string &s) {
  std::string out;
  bool prev_space = false;
  for (char c : s) {
    if (std::isspace((unsigned char)c)) {
      if (!prev_space)
        out.push_back(' ');
      prev_space = true;
    } else {
      out.push_back(c);
      prev_space = false;
    }
  }
  return trim(out);
}

static bool is_generic_line(const std::string &line) {
  static const std::vector<std::string> blacklist = {
      "MERCI", "TICKET", "CLIENT", "TOTAL",     "A PAYER", "NET",
      "TVA",   "SIRET",  "CB",     "BIENVENUE", "BONJOUR", "AU REVOIR"};
  for (const auto &w : blacklist) {
    if (line.find(w) != std::string::npos)
      return true;
  }
  return false;
}

static double letter_ratio(const std::string &s) {
  int letters = 0, digits = 0;
  for (char c : s) {
    if (std::isalpha((unsigned char)c))
      letters++;
    else if (std::isdigit((unsigned char)c))
      digits++;
  }
  int total = letters + digits;
  if (total == 0)
    return 0.0;
  return static_cast<double>(letters) / total;
}

// Accept a short first line only if it looks like a business keyword
// to avoid picking cities like "RENNES".
static bool is_business_keyword_shortline(const std::string &line) {
  // keep it tiny (MVP), enough for your real ticket
  static const std::vector<std::string> keywords = {
      "CAFE", "CAFÉ", "BAR", "RESTO", "RESTAURANT", "BRASSERIE"};
  for (const auto &k : keywords) {
    if (line.find(k) != std::string::npos)
      return true;
  }
  return false;
}

void parse_merchant(std::string_view text, ParsedTicket &ticket) {
  std::vector<std::string> lines;
  {
    std::string cur;
    for (char c : text) {
      if (c == '\n') {
        auto t = trim(cur);
        if (!t.empty())
          lines.push_back(t);
        cur.clear();
      } else {
        cur.push_back(c);
      }
    }
    auto t = trim(cur);
    if (!t.empty())
      lines.push_back(t);
  }

  const size_t MAX_LINES = 5;
  std::string merged;
  double best_score = 0.0;

  for (size_t i = 0; i < lines.size() && i < MAX_LINES; ++i) {
    const auto &line = lines[i];

    if (is_generic_line(line))
      continue;
    if (has_digit(line))
      continue;

    double score = letter_ratio(line);
    if (score < 0.8)
      continue;

    const bool short_line = line.size() < 8;

    // Default rule: ignore short lines (avoid cities like "RENNES")
    // Exception: accept short line only if it is a business keyword AND
    // it can be merged with the next line.
    if (short_line) {
      if (!is_business_keyword_shortline(line))
        continue;
      if (i + 1 >= lines.size() || (i + 1) >= MAX_LINES)
        continue;

      const auto &next = lines[i + 1];
      if (is_generic_line(next))
        continue;
      if (has_digit(next))
        continue;
      if (next.size() < 3)
        continue;
      if (letter_ratio(next) < 0.8)
        continue;
      if (has_lower(next))
        continue; // stop slogans like "café de quartier"
    }

    // start candidate from this line
    std::string candidate = line;

    // merge next consecutive plausible lines (max 2 lines total)
    int merged_lines = 1;
    for (size_t j = i + 1; j < lines.size() && j < MAX_LINES; ++j) {
      if (merged_lines >= 2)
        break;

      const auto &next = lines[j];
      if (is_generic_line(next))
        break;
      if (has_digit(next))
        break;
      if (next.size() < 3)
        break;
      if (letter_ratio(next) < 0.8)
        break;
      if (has_lower(next))
        break; // don't merge slogans / mixed-case lines

      candidate += " ";
      candidate += next;
      merged_lines++;
    }

    candidate = normalize_spaces(candidate);
    double cand_score = letter_ratio(candidate);

    if (cand_score > best_score) {
      best_score = cand_score;
      merged = candidate;
    }
  }

  if (!merged.empty()) {
    ticket.merchant.value = merged;
    ticket.merchant.confidence = std::min(0.95, 0.6 + best_score * 0.35);
    ticket.merchant.source = "heuristic:merged_header_lines";
  }
}

} // namespace tv
