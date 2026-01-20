#include "tv/signals.hpp"
#include <regex>

namespace tv {

Signals detect_signals(std::string_view text) {
  Signals s{};

  // SIRET: 14 digits (often grouped, but OCR here is contiguous)
  static const std::regex re_siret(R"(\bSIRET\b[^0-9]*([0-9]{14})\b)",
                                   std::regex::icase);
  std::cmatch m;
  if (std::regex_search(text.begin(), text.end(), m, re_siret)) {
    s.has_siret = true;
  }
  static const std::regex re_card(
      R"(\b(CB|CARTE\s+BANCAIRE|VISA|MASTERCARD|AMEX)\b)", std::regex::icase);

  if (std::regex_search(text.begin(), text.end(), re_card)) {
    s.has_card_keywords = true;
  }
  static const std::regex re_tva(R"(\bTVA\b\.?)", std::regex::icase);
  if (std::regex_search(text.begin(), text.end(), re_tva)) {
    s.has_tva = true;
  }

  return s;
}

} // namespace tv
