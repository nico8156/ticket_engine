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

TEST_CASE("[engine_real_receipt][real_receipt] engine parses a real cafe "
          "receipt OCR") {
  tv::Options opt;
  auto text = load_fixture("../../tests/fixtures/receipt_real_001.txt");
  auto out = tv::run(text, opt);
  INFO("text" << text);
  INFO("status=" << static_cast<int>(out.status));
  INFO("confidence=" << out.confidence);

  INFO("merchant=" << (out.ticket.merchant.value ? *out.ticket.merchant.value
                                                 : "<none>"));
  if (out.ticket.total.value) {
    INFO("total=" << out.ticket.total.value->value);
  } else {
    INFO("total=<none>");
  }
  INFO("fixture_size=" << text.size());
  REQUIRE(text.size() > 50);

  REQUIRE(out.ticket.total.value.has_value());
  REQUIRE(out.ticket.merchant.value.has_value());

  REQUIRE(out.ticket.merchant.value.has_value());
  REQUIRE(out.ticket.merchant.value->size() >= 5);

  REQUIRE(out.ticket.total.value.has_value());
  REQUIRE(out.ticket.total.value->value == Catch::Approx(31.70));

  REQUIRE(out.ticket.signals.has_card_keywords == true);
  REQUIRE(out.ticket.signals.has_tva == true);
  REQUIRE(out.ticket.signals.has_siret == true);
}
