#pragma once
#include "tv/model.hpp"
#include <string_view>

namespace tv {
void parse_merchant(std::string_view normalized_text, ParsedTicket &ticket);
}
