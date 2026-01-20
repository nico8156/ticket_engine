#include <catch2/catch_all.hpp>

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

TEST_CASE("parse_total extracts amount with thousands separator") {
  tv::ParsedTicket t;
  tv::parse_total("CAFE\nTOTAL 1 234,56 €\n", t);

  REQUIRE(t.total.value.has_value());
  REQUIRE(t.total.value->currency == "EUR");
  REQUIRE(t.total.value->value == Catch::Approx(1234.56));
}
TEST_CASE("parse_total extracts amount when euro symbol is before the number") {
  tv::ParsedTicket t;
  tv::parse_total("CAFE\nTOTAL €4,00\n", t);

  REQUIRE(t.total.value.has_value());
  REQUIRE(t.total.value->currency == "EUR");
  REQUIRE(t.total.value->value == Catch::Approx(4.0));
}
TEST_CASE("parse_total accepts one-decimal OCR amounts and normalizes to two "
          "decimals") {
  tv::ParsedTicket t;
  tv::parse_total("TOTAL 4,5 €\n", t);

  REQUIRE(t.total.value.has_value());
  REQUIRE(t.total.value->currency == "EUR");
  REQUIRE(t.total.value->value == Catch::Approx(4.5));
}
TEST_CASE("parse_total accepts integer amounts without decimals") {
  tv::ParsedTicket t;
  tv::parse_total("TOTAL 4 €\n", t);

  REQUIRE(t.total.value.has_value());
  REQUIRE(t.total.value->value == Catch::Approx(4.0));
}
