#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace tv {

struct NormalizedText {
  std::string text;
  std::vector<std::string> applied;
};

NormalizedText normalize_ocr(std::string_view input);

} // namespace tv

