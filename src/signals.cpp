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

  return s;
}

} // namespace tv
