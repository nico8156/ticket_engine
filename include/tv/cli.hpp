#pragma once
#include "tv/model.hpp"
#include <optional>
#include <string>
#include <vector>

namespace tv {

struct CliParseResult {
  Options options;
  bool show_help = false;
  bool show_version = false;
  std::optional<std::string> error; // if present => usage error
};

CliParseResult parse_args(const std::vector<std::string> &args);

std::string help_text();
std::string version_text();

} // namespace tv
