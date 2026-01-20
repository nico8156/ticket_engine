#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace tv {

enum class Domain { Auto, Cafe, Resto };
enum class Locale { Auto, FrFR };
enum class OutputFormat { Json };

struct Options {
  std::string schema = "v1";
  OutputFormat format = OutputFormat::Json;
  Locale locale = Locale::Auto;
  Domain domain = Domain::Auto;
  bool debug = false;
  std::uint32_t max_lines = 4000;
};

enum class Status { Ok, Partial, Reject, Error };

inline std::string status_to_string(Status s) {
  switch (s) {
    case Status::Ok: return "ok";
    case Status::Partial: return "partial";
    case Status::Reject: return "reject";
    case Status::Error: return "error";
  }
  return "error";
}

template <typename T>
struct Field {
  std::optional<T> value;
  double confidence = 0.0;      // 0..1
  std::string source = "none";  // e.g. "line:12", "regex:TOTAL"
};

struct Money {
  double value = 0.0;
  std::string currency = "EUR";
};

struct Warning {
  std::string code;
  std::string message;
  std::string severity; // "low"|"medium"|"high"
};

struct Signals {
  bool has_tva = false;
  bool has_siret = false;
  bool has_card_keywords = false;
};

struct Item {
  std::string label;
  double qty = 1.0;
  std::optional<double> unit_price;
  std::optional<double> total;
  double confidence = 0.0;
  std::string source = "none";
};

struct ParsedTicket {
  Field<std::string> merchant;
  Field<std::string> datetime_iso; // MVP: keep as ISO string
  Field<Money> total;

  std::vector<Item> items;
  Signals signals;
  std::vector<Warning> warnings;
};

struct InputMeta {
  std::string locale;   // "fr_FR" | "auto"
  std::string domain;   // "cafe" | "resto" | "auto"
  std::uint32_t chars = 0;
  std::uint32_t lines = 0;
  std::string hash;     // "sha256:..." (optional MVP)
};

struct TimingMs {
  int total = 0;
  int parse = 0;
  int score = 0;
};

struct EngineOutput {
  std::string schema = "ticketverify.v1";
  Status status = Status::Partial;
  double confidence = 0.0;

  InputMeta input;
  ParsedTicket ticket;
  TimingMs timing;

  // For debugging / transparency (MVP: just preview)
  std::string normalized_text_preview;
  std::vector<std::string> normalization_applied;

  // For fatal errors (Status::Error)
  std::optional<std::string> error_message;
};

} // namespace tv

