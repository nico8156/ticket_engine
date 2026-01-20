#include "tv/model.hpp"
#include "tv/signals.hpp"
#include <catch2/catch_all.hpp>

TEST_CASE("signals detects SIRET") {
  tv::Signals s = tv::detect_signals("SIRET 90888159000015\n");
  REQUIRE(s.has_siret == true);
}
TEST_CASE("detect_signals finds card payment keywords") {
  auto s = tv::detect_signals("Paiements\nCarte Bancaire (B)\n31,70 €\n");
  REQUIRE(s.has_card_keywords == true);
}
TEST_CASE("detect_signals finds TVA") {
  auto s = tv::detect_signals("TVA 10.0% (8)\n");
  REQUIRE(s.has_tva == true);
}

TEST_CASE("detect_signals finds SIRET") {
  auto s = tv::detect_signals("SIRET 90888159000015\n");
  REQUIRE(s.has_siret == true);
}
