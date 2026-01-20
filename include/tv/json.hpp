#pragma once
#include "tv/model.hpp"
#include <string>

namespace tv {

// Serialize EngineOutput as JSON string (single-line).
std::string to_json_v1(const EngineOutput& out);

} // namespace tv

