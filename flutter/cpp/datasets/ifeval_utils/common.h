#ifndef MLPERF_DATASETS_IFEVAL_UTILS_COMMON_H_
#define MLPERF_DATASETS_IFEVAL_UTILS_COMMON_H_

#include <algorithm>
#include <array>
#include <cctype>
#include <iomanip>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace mlperf {
namespace mobile {
namespace ifeval {

inline std::string ltrim(std::string s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));
  return s;
}
inline std::string rtrim(std::string s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); })
              .base(),
          s.end());
  return s;
}
inline std::string trim(std::string s) { return rtrim(ltrim(std::move(s))); }

inline std::string tolower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return s;
}

inline bool ends_with(const std::string& s, const std::string& suf) {
  if (s.size() < suf.size()) return false;
  std::string a = tolower(s.substr(s.size() - suf.size()));
  std::string b = tolower(suf);
  return a == b;
}

inline bool contains_string(const std::string& text,
                            const std::string& substring) {
  std::string h = tolower(text), n = tolower(substring);
  return h.find(n) != std::string::npos;
}

inline bool contains_word(const std::string& text, const std::string& word) {
  if (word.empty()) return false;

  auto to_lower_ascii = [](std::string s) {
    for (char& c : s) c = std::tolower(static_cast<unsigned char>(c));
    return s;
  };
  auto is_word_char = [](unsigned char c) {
    return std::isalnum(c) || c == '_';  // match std::regex \b notion of "word"
  };

  std::string t = to_lower_ascii(text);
  std::string w = to_lower_ascii(word);

  // Scan all occurrences of w in t and check word boundaries
  std::size_t pos = 0;
  while ((pos = t.find(w, pos)) != std::string::npos) {
    const bool left_ok =
        (pos == 0) || !is_word_char(static_cast<unsigned char>(t[pos - 1]));
    const std::size_t end = pos + w.size();
    const bool right_ok =
        (end == t.size()) || !is_word_char(static_cast<unsigned char>(t[end]));
    if (left_ok && right_ok) return true;
    ++pos;  // continue searching (overlapping-safe)
  }
  return false;
}

inline bool contains_none(const std::string& text,
                          const std::vector<std::string>& words) {
  for (const auto& w : words)
    if (contains_word(text, w)) return false;
  return true;
}

inline std::string remove_font_modifiers(const std::string& s) {
  std::string out;
  out.reserve(s.size());

  // bool inBacktick = false;
  for (std::size_t i = 0; i < s.size(); ++i) {
    char c = s[i];

    // toggle backtick code span
    if (c == '`') {
      // inBacktick = !inBacktick;
      continue;  // drop the backtick itself
    }

    // skip emphasis/strong/strike/escape chars as long as they're not preceeded
    // by an escape character
    if ((c == '*' || c == '_' || c == '~' || c == '\\') && s[i - 1] != '\\')
      continue;

    // remove heading markers (#) at line starts
    if ((c == '#') && (i == 0 || s[i - 1] == '\n')) continue;

    // drop leading '>' in blockquotes
    if ((c == '>') && (i == 0 || s[i - 1] == '\n')) continue;

    out.push_back(c);
  }
  return out;
}

inline std::string remove_first_line(const std::string& s) {
  std::size_t pos = s.find('\n');
  return (pos == std::string::npos) ? std::string{} : s.substr(pos + 1);
  // If there is no newline, removing the first line yields empty.
}

inline std::string remove_last_line(const std::string& s) {
  std::size_t pos = s.rfind('\n');
  return (pos == std::string::npos) ? std::string{} : s.substr(0, pos);
  // If there is no newline, removing the last line yields empty.
}

// Returns the 8 transformations as an array of strings.
// Index is a bitmask over {font_mod (bit0), remove_first (bit1), remove_last
// (bit2)}.

// 000 (0) nothing
// 001 (1) font
// 010 (2) fl
// 011 (3) font & fl
// 100 (4) ll
// 101 (5) ll & font
// 110 (6) fl & ll
// 111 (7) all
inline std::array<std::string, 8> transform_response(const std::string& resp) {
  std::array<std::string, 8> out{};
  for (int mask = 0; mask < 8; ++mask) {
    std::string t = resp;
    if (mask & 0b001) t = remove_font_modifiers(t);
    if (mask & 0b010) t = remove_first_line(t);
    if (mask & 0b100) t = remove_last_line(t);
    out[mask] = std::move(t);
  }
  return out;
}

}  // namespace ifeval
}  // namespace mobile
}  // namespace mlperf
#endif  // MLPERF_DATASETS_IFEVAL_UTILS_COMMON_H_
