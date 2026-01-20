#include <catch2/catch_all.hpp>

#include "tv/model.hpp"
#include "tv/parse_merchant.hpp"

TEST_CASE("parse_merchant picks the first plausible header line") {
  tv::ParsedTicket t;
  tv::parse_merchant("CAFE DE LA PLACE\nTOTAL 4,00 €\n", t);

  REQUIRE(t.merchant.value.has_value());
  REQUIRE(*t.merchant.value == "CAFE DE LA PLACE");
  REQUIRE(t.merchant.confidence > 0.5);
}

TEST_CASE("parse_merchant skips generic lines like MERCI / TICKET / TOTAL") {
  tv::ParsedTicket t;
  tv::parse_merchant("MERCI\nTICKET CLIENT\nCAFE DE LA PLACE\nTOTAL 4,00 €\n",
                     t);

  REQUIRE(t.merchant.value.has_value());
  REQUIRE(*t.merchant.value == "CAFE DE LA PLACE");
}
TEST_CASE(
    "parse_merchant prefers the most letter-heavy line in the header block") {
  tv::ParsedTicket t;
  tv::parse_merchant("12 RUE DE PARIS\n"
                     "CAFE DE LA PLACE\n"
                     "35000 RENNES\n"
                     "TOTAL 4,00 €\n",
                     t);

  REQUIRE(t.merchant.value.has_value());
  REQUIRE(*t.merchant.value == "CAFE DE LA PLACE");
}
TEST_CASE("parse_merchant ignores postal codes and addresses when a plausible "
          "name exists") {
  tv::ParsedTicket t;
  tv::parse_merchant("35000 RENNES\n"
                     "12 RUE DE PARIS\n"
                     "CAFE DE LA PLACE\n"
                     "TOTAL 4,00 €\n",
                     t);

  REQUIRE(t.merchant.value.has_value());
  REQUIRE(*t.merchant.value == "CAFE DE LA PLACE");
}
TEST_CASE(
    "parse_merchant does not pick city-only line over the merchant name") {
  tv::ParsedTicket t;
  tv::parse_merchant("RENNES\n"
                     "CAFE DE LA PLACE\n"
                     "TOTAL 4,00 €\n",
                     t);

  REQUIRE(t.merchant.value.has_value());
  REQUIRE(*t.merchant.value == "CAFE DE LA PLACE");
}
TEST_CASE("parse_merchant ignores generic greetings") {
  tv::ParsedTicket t;
  tv::parse_merchant("BIENVENUE\n"
                     "CAFE DE LA PLACE\n"
                     "TOTAL 4,00 €\n",
                     t);

  REQUIRE(t.merchant.value.has_value());
  REQUIRE(*t.merchant.value == "CAFE DE LA PLACE");
}
TEST_CASE("parse_merchant can merge consecutive header lines into a single "
          "merchant name") {
  tv::ParsedTicket t;
  tv::parse_merchant("CAFÉ\n"
                     "DE QUARTIER\n"
                     "GANG café de quartier\n"
                     "8 Rue de la Parcheminerie\n"
                     "TOTAL 31,70 €\n",
                     t);

  REQUIRE(t.merchant.value.has_value());
  REQUIRE(*t.merchant.value == "CAFÉ DE QUARTIER");
}
