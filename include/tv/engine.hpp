#pragma once
#include "tv/model.hpp"
#include <string_view>

namespace tv {

// Main pipeline (MVP stub inside for now).
EngineOutput run(std::string_view ocr_text, const Options& opt);

} // namespace tv

