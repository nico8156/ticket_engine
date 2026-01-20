#include "tv/normalize.hpp"
#include <cctype>

namespace tv {

static inline bool is_space(unsigned char c) { return std::isspace(c) != 0; }

NormalizedText normalize_ocr(std::string_view input) {
  NormalizedText out;

  // 1) Drop '\r' (CR) -> normalize newlines
  std::string s;
  s.reserve(input.size());
  for (char ch : input) {
    if (ch == '\r') continue;
    s.push_back(ch);
  }
  out.applied.push_back("drop_cr");

  // 2) Trim (leading/trailing whitespace)
  std::size_t start = 0;
  while (start < s.size() && is_space((unsigned char)s[start])) start++;

  std::size_t end = s.size();
  while (end > start && is_space((unsigned char)s[end - 1])) end--;

  s = s.substr(start, end - start);
  out.applied.push_back("trim");

  // 3) Collapse whitespace runs into single spaces, but keep '\n' as line separators
  std::string t;
  t.reserve(s.size());
  bool in_space = false;

  for (char ch : s) {
    if (ch == '\n') {
      in_space = false;
      t.push_back('\n');
      continue;
    }
    if (is_space((unsigned char)ch)) {
      if (!in_space) t.push_back(' ');
      in_space = true;
    } else {
      in_space = false;
      t.push_back(ch);
    }
  }
  out.applied.push_back("collapse_spaces");

  // 4) Replace NBSP (0xA0) with normal space (common in OCR/text copy)
  for (char& ch : t) {
    if ((unsigned char)ch == 0xA0) ch = ' ';
  }
  out.applied.push_back("nbsp_to_space");

  out.text = std::move(t);
  return out;
}

} // namespace tv

