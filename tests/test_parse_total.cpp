#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "tv/model.hpp"
#include "tv/parse_total.hpp"

TEST_CASE("parse_total extracts TOTAL with comma decimals") {
  tv::ParsedTicket t;
  tv::parse_total("CAFE\nTOTAL 4,00 €\n", t);

  REQUIRE(t.total.value.has_value());
  REQUIRE(t.total.value->currency == "EUR");
  REQUIRE(t.total.value->value == Catch::Approx(4.0));
  REQUIRE(t.total.confidence > 0.5);
}

TEST_CASE("parse_total extracts NET A PAYER") {
  tv::ParsedTicket t;
  tv::parse_total("NET A PAYER 8,20\n", t);

  REQUIRE(t.total.value.has_value());
  REQUIRE(t.total.value->value == Catch::Approx(8.2));
}

