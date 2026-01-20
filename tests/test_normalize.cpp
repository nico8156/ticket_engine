#include <catch2/catch_test_macros.hpp>
#include "tv/normalize.hpp"

TEST_CASE("normalize_ocr drops CR, trims and collapses spaces") {
  auto n = tv::normalize_ocr("  CAFE   DE  LA  PLACE \r\nTOTAL  4,00   €  \n\n");

  REQUIRE(n.text == "CAFE DE LA PLACE \nTOTAL 4,00 €");

  // On vérifie aussi qu'on trace les étapes (utile en debug)
  REQUIRE(n.applied.size() >= 3);
}

