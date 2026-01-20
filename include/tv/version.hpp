#pragma once
#include <string>

namespace tv {
struct VersionInfo {
  std::string name;
  std::string version;
  std::string build; // e.g. "git:abcdef"
};

VersionInfo version_info();
} // namespace tv

