#include "tv/cli.hpp"
#include "tv/engine.hpp"
#include "tv/json.hpp"

#include <iostream>
#include <iterator>
#include <string>
#include <vector>

static std::vector<std::string> collect_args(int argc, char** argv) {
  std::vector<std::string> a;
  a.reserve((argc > 1) ? (argc - 1) : 0);
  for (int i = 1; i < argc; i++) a.emplace_back(argv[i]);
  return a;
}

static std::string read_all_stdin_limited_lines(std::uint32_t max_lines, bool* truncated) {
  *truncated = false;

  std::string input(
    (std::istreambuf_iterator<char>(std::cin)),
    std::istreambuf_iterator<char>()
  );

  // Enforce max_lines by truncating after N lines
  std::uint32_t lines = 0;
  std::size_t cut_pos = input.size();
  for (std::size_t i = 0; i < input.size(); i++) {
    if (input[i] == '\n') {
      lines++;
      if (lines >= max_lines) { cut_pos = i + 1; break; }
    }
  }
  if (cut_pos < input.size()) {
    input.resize(cut_pos);
    *truncated = true;
  }
  return input;
}

int main(int argc, char** argv) {
  auto args = collect_args(argc, argv);
  auto parsed = tv::parse_args(args);

  if (parsed.show_help) {
    std::cout << tv::help_text();
    return 0;
  }
  if (parsed.show_version) {
    std::cout << tv::version_text() << "\n";
    return 0;
  }
  if (parsed.error) {
    std::cerr << "Error: " << *parsed.error << "\n\n" << tv::help_text();
    return 2;
  }

  bool truncated = false;
  std::string ocr_text = read_all_stdin_limited_lines(parsed.options.max_lines, &truncated);

  // Empty input => exit 3 (contract)
  bool any_non_ws = false;
  for (char c : ocr_text) {
    if (!std::isspace(static_cast<unsigned char>(c))) { any_non_ws = true; break; }
  }
  if (!any_non_ws) {
    std::cerr << "Error: empty/whitespace input\n";
    return 3;
  }

  if (parsed.options.debug && truncated) {
    std::cerr << "[debug] input truncated to max_lines=" << parsed.options.max_lines << "\n";
  }

  try {
    auto out = tv::run(ocr_text, parsed.options);
    std::cout << tv::to_json_v1(out) << "\n";

    // Exit code mapping
    switch (out.status) {
      case tv::Status::Ok: return 0;
      case tv::Status::Partial: return 0; // MVP: still success, caller checks status/confidence
      case tv::Status::Reject: return 0;  // same: JSON is still valid
      case tv::Status::Error: return 5;
    }
    return 5;
  } catch (const std::exception& e) {
    std::cerr << "Internal error: " << e.what() << "\n";
    return 5;
  } catch (...) {
    std::cerr << "Internal error: unknown exception\n";
    return 5;
  }
}

