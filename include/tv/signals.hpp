#pragma once
#include "tv/model.hpp"
#include <string_view>

namespace tv {
Signals detect_signals(std::string_view normalized_text);
}
