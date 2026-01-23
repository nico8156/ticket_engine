#include "tv/parse_total.hpp"
#include <regex>
#include <string>

namespace tv {

static std::optional<double> parse_amount(const std::string &s) {
  std::string x;
  x.reserve(s.size());
  for (char c : s) {
    if (c == ' ')
      continue;
    x.push_back(c == ',' ? '.' : c);
  }

  auto dot = x.find('.');
  if (dot != std::string::npos) {
    auto decimals = x.size() - dot - 1;
    if (decimals == 1)
      x.push_back('0');
  }

  try {
    size_t idx = 0;
    double v = std::stod(x, &idx);
    if (idx == 0)
      return std::nullopt;
    return v;
  } catch (...) {
    return std::nullopt;
  }
}

void parse_total(std::string_view text, ParsedTicket &ticket) {
  static const std::regex re(
      R"((TOTAL(\s+TTC)?|NET\s+A\s+PAYER|A\s+PAYER)\s*[:\-]?\s*(?:€|EUR)?\s*([0-9]{1,3}(?:[ .][0-9]{3})*(?:[.,][0-9]{1,2})?))",
      std::regex::icase);

  using It = std::string_view::const_iterator;
  std::match_results<It> m;

  if (std::regex_search(text.begin(), text.end(), m, re)) {
    std::string amount_str = m[3].str();
    auto amount = parse_amount(amount_str);
    if (amount) {
      Money money;
      money.value = *amount;
      money.currency = "EUR";

      ticket.total.value = money;
      ticket.total.confidence = 0.85;
      ticket.total.source = "regex:TOTAL";
    }
  }
}

} // namespace tv
