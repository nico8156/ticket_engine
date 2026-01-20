#include "tv/json.hpp"
#include "tv/version.hpp"
#include <nlohmann/json.hpp>

namespace tv {
using nlohmann::json;

static json field_string(const Field<std::string>& f) {
  json j = json::object();
  if (f.value) j["value"] = *f.value;
  j["confidence"] = f.confidence;
  j["source"] = f.source;
  return j;
}

static json field_money(const Field<Money>& f) {
  json j = json::object();
  if (f.value) {
    j["value"] = f.value->value;
    j["currency"] = f.value->currency;
  }
  j["confidence"] = f.confidence;
  j["source"] = f.source;
  return j;
}

std::string to_json_v1(const EngineOutput& out) {
  auto v = version_info();

  json j;
  j["schema"] = out.schema;

  j["engine"] = {
    {"name", v.name},
    {"version", v.version},
    {"build", v.build}
  };

  j["input"] = {
    {"locale", out.input.locale},
    {"domain", out.input.domain},
    {"chars", out.input.chars},
    {"lines", out.input.lines}
  };
  if (!out.input.hash.empty()) j["input"]["hash"] = out.input.hash;

  json result;
  result["status"] = status_to_string(out.status);
  result["confidence"] = out.confidence;

  json fields = json::object();
  if (out.ticket.merchant.value || out.ticket.merchant.confidence > 0.0) {
    fields["merchant"] = field_string(out.ticket.merchant);
  }
  if (out.ticket.datetime_iso.value || out.ticket.datetime_iso.confidence > 0.0) {
    fields["datetime"] = field_string(out.ticket.datetime_iso);
  }
  if (out.ticket.total.value || out.ticket.total.confidence > 0.0) {
    fields["total"] = field_money(out.ticket.total);
  }

  result["fields"] = fields;

  // items (optional)
  if (!out.ticket.items.empty()) {
    json items = json::array();
    for (const auto& it : out.ticket.items) {
      json ji;
      ji["label"] = it.label;
      ji["qty"] = it.qty;
      if (it.unit_price) ji["unit_price"] = *it.unit_price;
      if (it.total) ji["total"] = *it.total;
      ji["confidence"] = it.confidence;
      ji["source"] = it.source;
      items.push_back(ji);
    }
    result["items"] = items;
  }

  // signals
  result["signals"] = {
    {"has_tva", out.ticket.signals.has_tva},
    {"has_siret", out.ticket.signals.has_siret},
    {"has_card_keywords", out.ticket.signals.has_card_keywords}
  };

  // warnings
  if (!out.ticket.warnings.empty()) {
    json warnings = json::array();
    for (const auto& w : out.ticket.warnings) {
      warnings.push_back({
        {"code", w.code},
        {"message", w.message},
        {"severity", w.severity}
      });
    }
    result["warnings"] = warnings;
  }

  j["result"] = result;

  // raw (MVP preview only)
  j["raw"] = {
    {"normalized_text_preview", out.normalized_text_preview},
    {"normalization", {{"applied", out.normalization_applied}}}
  };

  j["timing_ms"] = {
    {"total", out.timing.total},
    {"parse", out.timing.parse},
    {"score", out.timing.score}
  };

  if (out.error_message) {
    j["error"] = { {"message", *out.error_message} };
  }

  // Single-line JSON
  return j.dump();
}

} // namespace tv

