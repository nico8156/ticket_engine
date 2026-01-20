#include <catch2/catch_all.hpp>
#include <fstream>
#include <sstream>

#include "tv/engine.hpp"

static std::string load_fixture(const std::string &path) {
  std::ifstream f(path);
  std::stringstream ss;
  ss << f.rdbuf();
  return ss.str();
}

TEST_CASE("engine parses a real cafe receipt OCR") {
  tv::Options opt;
  auto text = load_fixture("tests/fixtures/receipt_real_001.txt");
  auto out = tv::run(text, opt);

  REQUIRE(out.status == tv::Status::Ok);

  REQUIRE(out.ticket.merchant.value.has_value());
  REQUIRE(out.ticket.merchant.value->size() >= 5);

  REQUIRE(out.ticket.total.value.has_value());
  REQUIRE(out.ticket.total.value->value == Catch::Approx(31.70));

  REQUIRE(out.ticket.signals.has_card_keywords == true);
  REQUIRE(out.ticket.signals.has_tva == true);
  REQUIRE(out.ticket.signals.has_siret == true);
}
