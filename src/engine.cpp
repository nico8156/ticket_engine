#include "tv/engine.hpp"
#include "tv/normalize.hpp"
#include "tv/parse_merchant.hpp"
#include "tv/parse_total.hpp"
#include "tv/signals.hpp"
#include "tv/version.hpp"
#include <cctype>
#include <chrono>
#include <sstream>
#include <string>

namespace tv {

static std::string locale_to_string(Locale l) {
  switch (l) {
  case Locale::Auto:
    return "auto";
  case Locale::FrFR:
    return "fr_FR";
  }
  return "auto";
}

static std::string domain_to_string(Domain d) {
  switch (d) {
  case Domain::Auto:
    return "auto";
  case Domain::Cafe:
    return "cafe";
  case Domain::Resto:
    return "resto";
  }
  return "auto";
}

static std::uint32_t count_lines_limited(std::string_view s,
                                         std::uint32_t max_lines) {
  std::uint32_t lines = 0;
  for (char c : s) {
    if (c == '\n') {
      lines++;
      if (lines >= max_lines)
        return max_lines;
    }
  }
  // count last line if non-empty
  if (!s.empty() && s.back() != '\n')
    lines++;
  return lines;
}

static std::string preview(std::string_view s, std::size_t max_chars = 400) {
  std::string out;
  out.reserve((std::min)(s.size(), max_chars));
  for (std::size_t i = 0; i < s.size() && i < max_chars; i++)
    out.push_back(s[i]);
  if (s.size() > max_chars)
    out += "...";
  return out;
}

EngineOutput run(std::string_view ocr_text, const Options &opt) {
  using clock = std::chrono::steady_clock;
  auto t0 = clock::now();

  EngineOutput out;
  out.schema = "ticketverify." + opt.schema;

  out.input.locale = locale_to_string(opt.locale);
  out.input.domain = domain_to_string(opt.domain);
  out.input.chars = static_cast<std::uint32_t>(ocr_text.size());
  out.input.lines = count_lines_limited(ocr_text, opt.max_lines);

  // Guard: empty/whitespace input -> reject/invalid input handled by main (exit
  // code 3)
  bool any_non_ws = false;
  for (char c : ocr_text) {
    if (!std::isspace(static_cast<unsigned char>(c))) {
      any_non_ws = true;
      break;
    }
  }
  if (!any_non_ws) {
    out.status = Status::Reject;
    out.confidence = 0.0;
    out.normalized_text_preview = "";
    out.normalization_applied = {"input_whitespace_only"};
    out.timing.total = 0;
    return out;
  }
  // normalizing oct text

  auto norm = normalize_ocr(ocr_text);
  out.normalized_text_preview = preview(norm.text);
  out.normalization_applied = norm.applied;

  out.ticket.signals = detect_signals(norm.text);

  // TOTAL parsing
  parse_total(norm.text, out.ticket);
  // MERCHANT parsing
  parse_merchant(norm.text, out.ticket);

  const bool has_total = out.ticket.total.value.has_value();
  const bool has_merchant = out.ticket.merchant.value.has_value();
  if (has_total && has_merchant) {
    out.status = Status::Ok;
    out.confidence = 0.80;
  } else if (has_total) {
    out.status = Status::Partial;
    out.confidence = 0.65;
  } else {
    out.status = Status::Reject;
    out.confidence = 0.15;
    out.ticket.warnings.push_back(
        {"TOTAL_NOT_FOUND", "No total amount found.", "medium"});
  }
  auto t1 = clock::now();
  out.timing.total =
      (int)std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0)
          .count();
  out.timing.parse = out.timing.total;
  out.timing.score = 0;

  return out;
}

} // namespace tv
