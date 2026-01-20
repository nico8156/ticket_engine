#include "tv/cli.hpp"
#include "tv/version.hpp"
#include <sstream>

namespace tv {

static bool is_flag(const std::string& s, const char* flag) { return s == flag; }

static std::optional<Locale> parse_locale(const std::string& s) {
  if (s == "auto") return Locale::Auto;
  if (s == "fr_FR") return Locale::FrFR;
  return std::nullopt;
}

static std::optional<Domain> parse_domain(const std::string& s) {
  if (s == "auto") return Domain::Auto;
  if (s == "cafe") return Domain::Cafe;
  if (s == "resto") return Domain::Resto;
  return std::nullopt;
}

CliParseResult parse_args(const std::vector<std::string>& args) {
  CliParseResult res;

  for (std::size_t i = 0; i < args.size(); i++) {
    const auto& a = args[i];

    if (is_flag(a, "--help") || is_flag(a, "-h")) {
      res.show_help = true;
      continue;
    }
    if (is_flag(a, "--version")) {
      res.show_version = true;
      continue;
    }
    if (is_flag(a, "--debug")) {
      res.options.debug = true;
      continue;
    }

    auto need_value = [&](const char* flag) -> std::optional<std::string> {
      if (i + 1 >= args.size()) {
        res.error = std::string("Missing value for ") + flag;
        return std::nullopt;
      }
      return args[++i];
    };

    if (a == "--schema") {
      auto v = need_value("--schema");
      if (!v) break;
      if (*v != "v1") {
        res.error = "Unsupported schema: " + *v;
        break;
      }
      res.options.schema = *v;
      continue;
    }

    if (a == "--format") {
      auto v = need_value("--format");
      if (!v) break;
      if (*v != "json") {
        res.error = "Unsupported format: " + *v;
        break;
      }
      res.options.format = OutputFormat::Json;
      continue;
    }

    if (a == "--locale") {
      auto v = need_value("--locale");
      if (!v) break;
      auto loc = parse_locale(*v);
      if (!loc) {
        res.error = "Unsupported locale: " + *v;
        break;
      }
      res.options.locale = *loc;
      continue;
    }

    if (a == "--domain") {
      auto v = need_value("--domain");
      if (!v) break;
      auto d = parse_domain(*v);
      if (!d) {
        res.error = "Unsupported domain: " + *v;
        break;
      }
      res.options.domain = *d;
      continue;
    }

    if (a == "--max-lines") {
      auto v = need_value("--max-lines");
      if (!v) break;
      try {
        int n = std::stoi(*v);
        if (n <= 0) throw std::runtime_error("non-positive");
        res.options.max_lines = static_cast<std::uint32_t>(n);
      } catch (...) {
        res.error = "Invalid --max-lines: " + *v;
        break;
      }
      continue;
    }

    // Unknown argument
    if (!a.empty() && a[0] == '-') {
      res.error = "Unknown argument: " + a;
      break;
    } else {
      res.error = "Unexpected positional argument: " + a;
      break;
    }
  }

  return res;
}

std::string help_text() {
  std::ostringstream oss;
  oss
    << "ticketverify - TicketVerify Engine (OCR text -> JSON)\n\n"
    << "Usage:\n"
    << "  cat ocr.txt | ticketverify [options]\n\n"
    << "Options:\n"
    << "  --schema v1              Output JSON schema version (default: v1)\n"
    << "  --format json            Output format (default: json)\n"
    << "  --locale fr_FR|auto      Locale hint (default: auto)\n"
    << "  --domain cafe|resto|auto Domain hint (default: auto)\n"
    << "  --max-lines N            Limit number of OCR lines read (default: 4000)\n"
    << "  --debug                  Verbose logs to stderr\n"
    << "  --version                Print version\n"
    << "  --help                   Print help\n";
  return oss.str();
}

std::string version_text() {
  auto v = version_info();
  std::ostringstream oss;
  oss << v.name << " " << v.version << " (" << v.build << ")";
  return oss.str();
}

} // namespace tv

