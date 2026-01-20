#include "tv/version.hpp"

namespace tv {
VersionInfo version_info() {
  VersionInfo v;
  v.name = "ticketverify";
  v.version = "0.1.0";
  v.build = "git:dev"; // MVP: override via build system later if needed
  return v;
}
} // namespace tv

