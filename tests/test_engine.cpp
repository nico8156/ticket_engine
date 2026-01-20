#include <catch2/catch_all.hpp>

#include "tv/engine.hpp"
#include "tv/model.hpp"

TEST_CASE("engine returns ok when total and merchant are found") {
  tv::Options opt;
  auto out = tv::run("CAFE DE LA PLACE\nTOTAL 4,00 €\n", opt);

  REQUIRE(out.ticket.total.value.has_value());
  REQUIRE(out.ticket.merchant.value.has_value());

  REQUIRE(out.status == tv::Status::Ok);
  REQUIRE(out.confidence >= 0.75);
}
