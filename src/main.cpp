#include "tv/cli.hpp"
#include "tv/engine.hpp"
#include "tv/json.hpp"

#include <iostream>
#include <iterator>
#include <string>
#include <vector>

static std::string json_escape(const std::string &s) {
  std::string out;
  out.reserve(s.size() + 8);
  for (unsigned char c : s) {
    switch (c) {
    case '\\':
      out += "\\\\";
      break;
    case '"':
      out += "\\\"";
      break;
    case '\n':
      out += "\\n";
      break;
    case '\r':
      out += "\\r";
      break;
    case '\t':
      out += "\\t";
      break;
    default:
      if (c < 0x20) {
        out += ' ';
      } // MVP: neutralise contrôles
      else
        out += static_cast<char>(c);
    }
  }
  return out;
}

static void print_json_error(const std::string &code,
                             const std::string &message) {
  std::cout << "{\"ok\":false,\"error\":{\"code\":\"" << json_escape(code)
            << "\",\"message\":\"" << json_escape(message) << "\"}}";
}

static std::vector<std::string> collect_args(int argc, char **argv) {
  std::vector<std::string> a;
  a.reserve((argc > 1) ? (argc - 1) : 0);
  for (int i = 1; i < argc; i++)
    a.emplace_back(argv[i]);
  return a;
}
static std::string read_stdin_limited(std::uint32_t max_lines,
                                      std::size_t max_bytes,
                                      bool *truncated_lines, bool *too_large) {
  *truncated_lines = false;
  *too_large = false;

  std::string input;
  input.reserve(4096);

  std::uint32_t lines = 0;
  char buf[4096];

  while (std::cin.good()) {
    std::cin.read(buf, sizeof(buf));
    std::streamsize got = std::cin.gcount();
    if (got <= 0)
      break;

    for (std::streamsize i = 0; i < got; i++) {
      if (input.size() >= max_bytes) {
        *too_large = true;
        return input;
      }
      char c = buf[i];
      input.push_back(c);

      if (c == '\n') {
        lines++;
        if (lines >= max_lines) {
          *truncated_lines = true;
          return input;
        }
      }
    }
  }
  return input;
}
int main(int argc, char **argv) {
  auto args = collect_args(argc, argv);
  auto parsed = tv::parse_args(args);

  if (parsed.show_help) {
    std::cerr << tv::help_text() << "\n";
    return 0;
  }
  if (parsed.show_version) {
    std::cerr << tv::version_text() << "\n";
    return 0;
  }
  if (parsed.error) {
    if (parsed.options.debug) {
      std::cerr << "[debug] arg error: " << *parsed.error << "\n";
    }
    print_json_error("ARGS_INVALID", *parsed.error);
    return 2;
  }

  bool truncated = false;
  bool too_large = false;
  constexpr std::size_t MAX_BYTES = 2 * 1024 * 1024; // 2MB MVP

  std::string ocr_text = read_stdin_limited(parsed.options.max_lines, MAX_BYTES,
                                            &truncated, &too_large);

  auto is_all_ws = [](const std::string &s) {
    for (unsigned char c : s)
      if (!std::isspace(c))
        return false;
    return true;
  };

  if (too_large) {
    if (parsed.options.debug)
      std::cerr << "[debug] input too large (max_bytes=" << MAX_BYTES << ")\n";
    print_json_error("INPUT_TOO_LARGE", "stdin exceeds max size");
    return 2;
  }

  if (ocr_text.empty() || is_all_ws(ocr_text)) {
    if (parsed.options.debug)
      std::cerr << "[debug] empty/whitespace input\n";
    print_json_error("INPUT_EMPTY", "stdin is empty");
    return 2;
  }

  if (parsed.options.debug && truncated) {
    std::cerr << "[debug] input truncated to max_lines="
              << parsed.options.max_lines << "\n";
  }
  try {
    auto out = tv::run(ocr_text, parsed.options);

    if (out.status == tv::Status::Error) {
      if (parsed.options.debug)
        std::cerr << "[debug] engine returned Status::Error\n";
      print_json_error("INTERNAL", "engine error");
      return 3;
    }

    std::cout << tv::to_json_v1(out) << "\n";
    return 0;

  } catch (const std::exception &e) {
    if (parsed.options.debug)
      std::cerr << "[debug] exception: " << e.what() << "\n";
    print_json_error("INTERNAL", "unexpected error");
    return 3;
  } catch (...) {
    if (parsed.options.debug)
      std::cerr << "[debug] unknown exception\n";
    print_json_error("INTERNAL", "unexpected error");
    return 3;
  }
}
