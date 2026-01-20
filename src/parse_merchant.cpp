#include "tv/parse_merchant.hpp"
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace tv {

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

void parse_merchant(std::string_view text, ParsedTicket &ticket) {
  std::vector<std::string> lines;
  std::string current;

  for (char c : text) {
    if (c == '\n') {
      if (!current.empty())
        lines.push_back(current);
      current.clear();
    } else {
      current.push_back(c);
    }
  }
  if (!current.empty())
    lines.push_back(current);

  // MVP: inspect only the first few lines
  const size_t MAX_LINES = 5;
  double best_score = 0.0;
  std::string best_line;

  for (size_t i = 0; i < lines.size() && i < MAX_LINES; ++i) {
    const auto &line = lines[i];

    if (line.size() < 8)
      continue;
    if (is_generic_line(line))
      continue;

    double score = letter_ratio(line);
    if (score > best_score) {
      best_score = score;
      best_line = line;
    }
  }

  if (!best_line.empty()) {
    ticket.merchant.value = best_line;
    ticket.merchant.confidence = std::min(0.9, 0.5 + best_score * 0.4);
    ticket.merchant.source = "heuristic:header_lines";
  }
}

} // namespace tv
