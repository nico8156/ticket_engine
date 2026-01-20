#include "tv/model.hpp"
#include "tv/signals.hpp"
#include <catch2/catch_all.hpp>

TEST_CASE("signals detects SIRET") {
  tv::Signals s = tv::detect_signals("SIRET 90888159000015\n");
  REQUIRE(s.has_siret == true);
}
