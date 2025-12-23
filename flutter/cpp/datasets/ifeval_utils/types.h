#ifndef MLPERF_DATASETS_IFEVAL_UTILS_TYPES_H_
#define MLPERF_DATASETS_IFEVAL_UTILS_TYPES_H_

#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>

#include "compact_lang_det.h"
#include "flutter/cpp/datasets/ifeval_utils/common.h"
#include "flutter/cpp/datasets/ifeval_utils/json.h"

namespace mlperf {
namespace mobile {
namespace ifeval {

enum InstructionGroup {
  CHANGE_CASE,
  COMBINATION,
  DETECTABLE_CONTENT,
  DETECTABLE_FORMAT,
  KEYWORDS,
  LANGUAGE,
  LENGTH_CONSTRAINTS,
  PUNCTUATION,
  STARTEND
};

enum Relation { AT_LEAST, LESS_THAN };

inline bool compare(size_t value, size_t threshold, Relation rel) {
  if (rel == AT_LEAST) return value >= threshold;
  return value < threshold;  // LESS_THAN
}

class Instruction {
 public:
  virtual ~Instruction() = default;
  virtual constexpr InstructionGroup Group() = 0;

  bool IsFollowed(const std::string& resp, bool loose = false) const {
    // For strict checks, just verify the response itself
    if (!loose) return verify_(resp);

    auto transformations = transform_response(resp);
    for (std::string transformation : transformations) {
      if (verify_(transformation)) return true;
    }
    return false;
  }

 private:
  virtual bool verify_(const std::string& resp) const = 0;
};

/* ---------- CHANGE_CASE ---------- */

class CapitalWordFrequency : public Instruction {
 public:
  CapitalWordFrequency(int capital_frequency, Relation capital_relation)
      : threshold_(capital_frequency), rel_(capital_relation) {}

  constexpr InstructionGroup Group() override { return CHANGE_CASE; }

 private:
  int threshold_;
  Relation rel_;

  static bool IsAllCapsToken(std::string_view t) {
    // trim leading/trailing punctuation (keep '-' and '\'' because they appear
    // inside words)
    auto is_trim = [](unsigned char c) {
      return !(std::isalnum(c) || c == '-' || c == '\'');
    };
    size_t b = 0, e = t.size();
    while (b < e && is_trim((unsigned char)t[b])) ++b;
    while (e > b && is_trim((unsigned char)t[e - 1])) --e;
    if (b >= e) return false;

    bool seen_alpha = false;
    for (size_t i = b; i < e; ++i) {
      unsigned char c = (unsigned char)t[i];
      if (std::isalpha(c)) {
        seen_alpha = true;
        if (std::islower(c))
          return false;  // any lowercase letter breaks ALL-CAPS
      }
      // digits, '-', '\'' are allowed and ignored for casing
    }
    return seen_alpha;  // at least one letter, and no lowercase letters
  }

  static size_t CountAllCapsWords(const std::string& resp) {
    size_t count = 0;
    std::istringstream is(resp);
    std::string tok;
    while (is >> tok) {
      if (IsAllCapsToken(tok)) ++count;
    }
    return count;
  }

  virtual bool verify_(const std::string& resp) const override {
    size_t words = CountAllCapsWords(resp);
    return compare(words, threshold_, rel_);
  }
};

class EnglishCapital : public Instruction {
 public:
  EnglishCapital() = default;
  constexpr InstructionGroup Group() override { return CHANGE_CASE; }

 private:
  virtual bool verify_(const std::string& resp) const override {
    return std::all_of(resp.begin(), resp.end(), [](unsigned char c) {
      return !std::isalpha(c) || std::isupper(c);
    });
  }
};

class EnglishLowercase : public Instruction {
 public:
  EnglishLowercase() = default;
  constexpr InstructionGroup Group() override { return CHANGE_CASE; }

 private:
  virtual bool verify_(const std::string& resp) const override {
    return std::all_of(resp.begin(), resp.end(), [](unsigned char c) {
      return !std::isalpha(c) || std::islower(c);
    });
  }
};

/* ---------- COMBINATION ---------- */

class RepeatPrompt : public Instruction {
 public:
  explicit RepeatPrompt(std::string prompt_to_repeat)
      : prompt_(std::move(prompt_to_repeat)) {}
  constexpr InstructionGroup Group() override { return COMBINATION; }

 private:
  std::string prompt_;
  virtual bool verify_(const std::string& resp) const override {
    return starts_with(resp, prompt_, 3);
  }
};

class TwoResponses : public Instruction {
 public:
  TwoResponses() = default;
  constexpr InstructionGroup Group() override { return COMBINATION; }

 private:
  virtual bool verify_(const std::string& resp) const override {
    std::size_t count = 0;
    std::size_t pos = resp.find("******");
    while (pos != std::string::npos) {
      if (pos == 0 ||
          pos == resp.size() - 6) {  // ignore indicators at the start and end
        pos = resp.find("******", pos + 6);
        continue;
      }
      if (++count > 1) return false;       // more than one occurrence
      pos = resp.find("******", pos + 6);  // disallow overlapping matches
    }
    return count > 0;
  }
};

/* ------- DETECTABLE_CONTENT ------- */

class NumberPlaceholders : public Instruction {
 public:
  explicit NumberPlaceholders(int num_placeholders) : n_(num_placeholders) {}
  constexpr InstructionGroup Group() override { return DETECTABLE_CONTENT; }

 private:
  int n_;
  virtual bool verify_(const std::string& resp) const override {
    std::size_t count = 0, pos = 0;
    while (pos < resp.length() &&
           (int)count < n_) {  // no need to keep looking if the requirement is
      // already satisfied
      std::size_t open = resp.find('[', pos);
      if (open == std::string::npos) break;
      std::size_t close = resp.find(']', open + 1);
      if (close == std::string::npos) break;

      if (close > open + 1) ++count;  // non-empty inner
      pos = close + 1;                // continue after this closing bracket
    }
    return (int)count >= n_;
  }
};

class Postscript : public Instruction {
 public:
  explicit Postscript(std::string postscript_marker)
      : marker_(std::move(postscript_marker)) {}
  constexpr InstructionGroup Group() override { return DETECTABLE_CONTENT; }

 private:
  std::string marker_;
  virtual bool verify_(const std::string& resp) const override {
    return contains_string(resp, marker_);
  }
};

/* ------- DETECTABLE_FORMAT -------- */

class ConstrainedResponse : public Instruction {
 public:
  ConstrainedResponse() = default;
  constexpr InstructionGroup Group() override { return DETECTABLE_FORMAT; }

 private:
  // TODO constexpr?
  const std::string aYes = "My answer is yes.";
  const std::string aNo = "My answer is no.";
  const std::string aMaybe = "My answer is maybe.";
  const unsigned sizeThreshold = 3;
  virtual bool verify_(const std::string& resp) const override {
    return (resp.find(aYes) != std::string::npos &&
            resp.size() <= sizeThreshold + aYes.size()) ||
           (resp.find(aNo) != std::string::npos &&
            resp.size() <= sizeThreshold + aNo.size()) ||
           (resp.find(aMaybe) != std::string::npos &&
            resp.size() <= sizeThreshold + aMaybe.size());
  }
};

class JsonFormat : public Instruction {
 public:
  JsonFormat() = default;
  constexpr InstructionGroup Group() override { return DETECTABLE_FORMAT; }

 private:
  virtual bool verify_(const std::string& resp) const override {
    std::string t = resp;
    if (t.empty()) return false;
    if (t[0] == '`') {
      size_t first = t.find('\n');
      size_t last = t.rfind('\n');

      if (first != std::string::npos && last != std::string::npos &&
          last > first)
        t = t.substr(first + 1, last - first - 1);
    }
    crow::json::rvalue jv = crow::json::load(t);
    return jv.is_valid();
  }
};

class MultipleSections : public Instruction {
 public:
  MultipleSections(int num_sections, std::string section_spliter)
      : n_(num_sections), sep_(std::move(section_spliter)) {}
  constexpr InstructionGroup Group() override { return DETECTABLE_FORMAT; }

 private:
  int n_;
  std::string sep_;
  static int CountNonEmpty(const std::vector<std::string>& v) {
    int c = 0;
    for (auto& p : v)
      if (!trim(p).empty()) ++c;
    return c;
  }

  static bool isnum(const std::string text, size_t pos) {
    unsigned char c = text[pos];
    return (c >= '0' && c <= '9') || c == 'I' || c == 'V' || c == 'X';
  }

  inline std::vector<std::string> SplitByDelim(const std::string& s,
                                               const std::string& delim) const {
    if (delim.empty()) return {s};
    std::vector<std::string> parts;
    size_t start = s.find(delim, start);
    while (true) {
      if (start == std::string::npos) break;
      size_t pos = s.find(delim, start + delim.size());
      if (pos == std::string::npos) {
        parts.push_back(s.substr(start));
        break;
      }
      if (!isnum(s, pos + delim.size() +
                        1)) {  // just a word, not "Section X", ignore and move
                               // on to the next one
        start = pos;
        continue;
      }
      parts.push_back(s.substr(start, pos - start));
      start = pos;
    }
    return parts;
  }
  virtual bool verify_(const std::string& resp) const override {
    auto parts = SplitByDelim(resp, sep_);
    int count = CountNonEmpty(parts);
    if (resp.find("******") != std::string::npos)
      count /= 2;  // If 2 responses are given, divide by 2 so we get the result
                   // for each response
    return count == n_;
  }
};

class NumberBulletLists : public Instruction {
 public:
  explicit NumberBulletLists(int num_bullets) : n_(num_bullets) {}
  constexpr InstructionGroup Group() override { return DETECTABLE_FORMAT; }

 private:
  int n_;

  inline std::vector<std::string> SplitLines(const std::string& s) const {
    std::vector<std::string> out;
    std::string cur;
    std::istringstream is(s);
    while (std::getline(is, cur)) out.push_back(cur);
    return out;
  }

  virtual bool verify_(const std::string& resp) const override {
    size_t count = 0;
    for (const auto& line : SplitLines(resp)) {
      std::string t = trim(line);
      if (t.rfind("* ", 0) == 0 || t.rfind("- ", 0) == 0) {
        ++count;
        continue;
      }
    }
    return (int)count == n_;
  }
};

class NumberHighlightedSections : public Instruction {
 public:
  explicit NumberHighlightedSections(int num_highlights) : n_(num_highlights) {}
  constexpr InstructionGroup Group() override { return DETECTABLE_FORMAT; }

 private:
  int n_;
  virtual bool verify_(const std::string& resp) const override {
    std::size_t count = 0;
    std::size_t pos = 0;

    while (true) {
      // find opening '*'
      std::size_t open = resp.find('*', pos);
      if (open == std::string::npos) break;

      // need at least one non-*\r\n char after the opener
      if (open + 1 >= resp.size()) break;
      char next = resp[open + 1];
      if (next == '*' || next == '\n' || next == '\r') {
        pos = open + 1;  // not a valid start; try from the next '*'
        continue;
      }

      // find the first '*' or newline after the opener
      std::size_t stop = resp.find_first_of("*\r\n", open + 1);
      if (stop == std::string::npos) break;

      if (resp[stop] == '*') {
        // we have "*...*" with no '*' or newline inside
        ++count;
        pos = stop + 1;  // continue after the closing '*'
      } else {
        // newline encountered before a closing '*': this opener can't match
        pos = stop + 1;  // continue scanning after the newline
      }
    }
    return (int)count >= n_;
  }
};

class Title : public Instruction {
 public:
  Title() = default;
  constexpr InstructionGroup Group() override { return DETECTABLE_FORMAT; }

 private:
  virtual bool verify_(const std::string& resp) const override {
    std::size_t pos_open = resp.find("<<");
    // TODO should an empty title be allowed?
    return (pos_open != std::string::npos) &&
           (resp.find(">>", pos_open + 2) !=
            std::string::npos);  // found "<<" with a following ">>"
  }
};

/* -------------- KEYWORDS -------------- */

class Existence : public Instruction {
 public:
  explicit Existence(std::vector<std::string> keywords)
      : kws_(std::move(keywords)) {}
  constexpr InstructionGroup Group() override { return KEYWORDS; }

 private:
  std::vector<std::string> kws_;
  virtual bool verify_(const std::string& resp) const override {
    for (const auto& k : kws_)
      if (!contains_word(resp, k)) return false;
    return true;
  }
};

class ForbiddenWords : public Instruction {
 public:
  explicit ForbiddenWords(std::vector<std::string> forbidden_words)
      : bad_(std::move(forbidden_words)) {}
  constexpr InstructionGroup Group() override { return KEYWORDS; }

 private:
  std::vector<std::string> bad_;
  virtual bool verify_(const std::string& resp) const override {
    return contains_none(resp, bad_);
  }
};

class Frequency : public Instruction {
 public:
  Frequency(int frequency, std::string keyword, Relation relation)
      : n_(frequency), kw_(std::move(keyword)), rel_(relation) {}
  constexpr InstructionGroup Group() override { return KEYWORDS; }

 private:
  int n_;
  std::string kw_;
  Relation rel_;

  static inline std::string RegexEscape(const std::string& s) {
    auto is_meta = [](unsigned char ch) {
      switch (ch) {
        case '^':
        case '$':
        case '.':
        case '|':
        case '?':
        case '*':
        case '+':
        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}':
        case '\\':
          return true;
        default:
          return false;
      }
    };

    std::string out;
    out.reserve(s.size() * 2);
    for (unsigned char c : s) {
      if (is_meta(c)) out.push_back('\\');
      out.push_back(static_cast<char>(c));
    }
    return out;
  }

  // Build a regex that matches the keyword with custom token boundaries.
  // Left boundary is (^|[^A-Za-z0-9_]) to avoid lookbehind.
  // Right boundary uses a lookahead (?=$|[^A-Za-z0-9_]).
  static inline std::regex MakeKeywordRegex(const std::string& keyword) {
    const std::string kw = RegexEscape(keyword);
    const std::string pat =
        "(^|[^A-Za-z0-9_])"  // left boundary (consumes 1 char or start)
        "(?:" +
        kw +
        ")"                     // keyword literal
        "(?=$|[^A-Za-z0-9_])";  // right boundary (zero-width lookahead)
    return std::regex(pat, std::regex::icase);
  }

  static inline std::size_t CountKeywordOccurrences(
      const std::string& text, const std::string& keyword) {
    const std::regex rx = MakeKeywordRegex(keyword);
    std::size_t count = 0;
    for (auto it = std::sregex_iterator(text.begin(), text.end(), rx),
              end = std::sregex_iterator();
         it != end; ++it) {
      ++count;
    }
    return count;
  }

  virtual bool verify_(const std::string& resp) const override {
    const std::size_t count = CountKeywordOccurrences(resp, kw_);
    return compare(count, (size_t)n_, rel_);
  }
};

class LetterFrequency : public Instruction {
 public:
  LetterFrequency(int let_frequency, char letter, Relation let_relation)
      : n_(let_frequency), letter_(letter), rel_(let_relation) {}
  constexpr InstructionGroup Group() override { return KEYWORDS; }

 private:
  int n_;
  char letter_;
  Relation rel_;
  static size_t CountLetterICase(const std::string& s, char letter) {
    size_t c = 0;
    char lower = std::tolower((unsigned char)letter);
    for (unsigned char ch : s)
      if (std::tolower(ch) == lower) ++c;
    return c;
  }
  virtual bool verify_(const std::string& resp) const override {
    size_t c = CountLetterICase(resp, letter_);
    return compare(c, (size_t)n_, rel_);
  }
};

/* -------------- LANGUAGE -------------- */

class ResponseLanguage : public Instruction {
 public:
  explicit ResponseLanguage(std::string language)
      : lang_(std::move(language)) {}
  constexpr InstructionGroup Group() override { return LANGUAGE; }

 private:
  std::string lang_;

  inline bool LanguageHeuristic(const std::string& text,
                                const std::string& lang) const {
    bool is_reliable = true;
    std::string detected_lang(CLD2::LanguageCode(
        CLD2::DetectLanguage(text.c_str(), text.size(), true, &is_reliable)));
    return detected_lang == lang;
  }

  virtual bool verify_(const std::string& resp) const override {
    return LanguageHeuristic(resp, lang_);
  }
};

/* ----------- LENGTH_CONSTRAINTS ----------- */

class NthParagraphFirstWord : public Instruction {
 public:
  NthParagraphFirstWord(int nth_paragraph, std::string first_word,
                        int num_paragraphs)
      : nth_(nth_paragraph),
        first_(std::move(first_word)),
        total_(num_paragraphs) {}
  constexpr InstructionGroup Group() override { return LENGTH_CONSTRAINTS; }

 private:
  int nth_;
  std::string first_;
  int total_;

  static std::string FirstWord(const std::string& s) {
    std::istringstream is(s);
    std::string w;
    std::string fw;
    is >> w;
    w = tolower(w);
    for (char c : w)
      if (std::isalpha(c) && !std::isspace(c)) fw.push_back(c);
    return fw;
  }

  static inline std::vector<std::string> SplitParagraphs(const std::string& s) {
    // paragraphs separated only by the literal delimiter "\n\n"
    std::vector<std::string> paras;
    std::size_t start = 0;
    while (true) {
      std::size_t pos = s.find("\n\n", start);
      if (pos == std::string::npos) {
        std::string chunk = s.substr(start);
        if (!chunk.empty()) paras.push_back(rtrim(chunk));
        break;
      }
      std::string chunk = s.substr(start, pos - start);
      if (!chunk.empty()) paras.push_back(rtrim(chunk));
      start = pos + 2;  // skip the delimiter
    }
    return paras;
  }

  virtual bool verify_(const std::string& resp) const override {
    auto paras = SplitParagraphs(resp);
    if ((int)paras.size() != total_) return false;
    if (nth_ <= 0 || nth_ > (int)paras.size()) return false;
    auto target = trim(paras[nth_ - 1]);
    if (target.empty()) return false;
    return FirstWord(target) == tolower(first_);
  }
};

class NumberParagraphs : public Instruction {
 public:
  explicit NumberParagraphs(int num_paragraphs) : n_(num_paragraphs) {}
  constexpr InstructionGroup Group() override { return LENGTH_CONSTRAINTS; }

 private:
  unsigned n_;
  static constexpr unsigned threshold =
      5;  // to allow 5 characters at the very start or end of the response
  virtual bool verify_(const std::string& resp) const override {
    std::size_t count = 0, pos = 0;
    while ((pos = resp.find("***", pos)) != std::string::npos) {
      if (pos >= threshold && pos <= resp.size() - (3 + threshold)) ++count;
      pos += 4;  // advance by 3 for non-overlapping matches
    }
    return count == n_ - 1;  // since *** is a saparator, the actual count is 1
                             // more than the number of separators
  }
};

class NumberSentences : public Instruction {
 public:
  NumberSentences(int num_sentences, Relation relation)
      : n_(num_sentences), rel_(relation) {}
  constexpr InstructionGroup Group() override { return LENGTH_CONSTRAINTS; }

 private:
  int n_;
  Relation rel_;

  inline std::string word_before_dot(const std::string& s, size_t i) const {
    size_t start = i;
    while (start > 0 && std::isalpha((unsigned char)s[start - 1]))
      start--;
    return s.substr(start, i - start);
  }

  inline std::string word_after_dot(const std::string& s, size_t i) const {
    size_t end = i + 1;
    while (end < s.size() && std::isalpha((unsigned char)s[end]))
      end++;
    return s.substr(i + 1, end - (i + 1));
  }

  inline bool is_letter(char c) const { return std::isalpha(c); }

  inline bool is_digit(char c) const { return c >= '0' && c <= '9'; }

  inline bool is_mark(char c) const { return c == '.' || c == '!' || c == '?'; }

  bool is_initialism(const std::string& s, size_t i) const {
    size_t j = i;
    unsigned count = 0;

    while (j > 0 && std::isupper((unsigned char)s[j - 1])) {
      if (j + 1 < s.size() && s[j] == '.') {
        count++;
        j -= 2;
      } else {
        break;
      }
    }

    // check if followed by another X. for first '.'
    if (count == 1) {
      if (i + 2 < s.size() && std::isupper((unsigned char)s[i + 1]) && s[i + 2] == '.') {
        count = 2;
      }
    }

    return count >= 2;
  }

  bool is_latin_abbrev(const std::string& s, size_t i) const {
    if (i < 3) return false;
    return std::islower((unsigned char)s[i - 3]) &&
           s[i - 2] == '.' &&
           std::islower((unsigned char)s[i - 1]) &&
           s[i] == '.';
  }

  bool is_title_abbrev(const std::string& s, size_t i) const {
    static const std::unordered_set<std::string> titles = {
        "Mr", "Mrs", "Ms", "Dr", "Prof", "Sr", "Jr"
    };

    std::string word = word_before_dot(s, i);
    return !word.empty() && titles.count(word) != 0;
  }

  bool is_enumeration_prefix(const std::string& s, size_t i) const {
    if (i == 0) return false;

            // Must be followed by space or newline
    if (i + 1 >= s.size() || (s[i + 1] != ' ' && s[i + 1] != '\n'))
      return false;

    size_t start = i - 1;

    // ---- Numeric enumeration: 1. / 10. ----
    if (is_digit(s[start])) {
      while (start > 0 && is_digit(s[start - 1])) start--;
    }

    // TODO roman numerals maybe?
    // ---- Letter enumeration: a. / A. ----
    else if (is_letter(s[start]) && start > 0 && is_letter(s[start - 1]))
        return false;

    // General check
    if (start == 0) return true;

    char prev = s[start - 1];
    if (prev == ' ' || prev == '\n' || is_mark(prev))
      return true;

    return false;
  }

  bool is_domain_suffix(const std::string& s, size_t i) const {
    static const std::unordered_set<std::string> tlds = {
        "com", "net", "org", "io", "gov", "edu", "me"
    };

    if (i + 1 >= s.size()) return false;

    std::string suffix = word_after_dot(s, i);
    return tlds.count(suffix) != 0;
  }


  bool is_decimal_point(const std::string& s, size_t i) const {
    // digit '.' digit
    if (i == 0 || i + 1 >= s.size()) return false;
    return is_digit(s[i - 1]) && is_digit(s[i + 1]);
  }

  bool is_abbreviation(const std::string& s, size_t i) const {
    return is_initialism(s, i) || is_latin_abbrev(s, i) || is_title_abbrev(s, i);
  }

  bool abbreviation_blocks_sentence(const std::string& s, size_t i) const {
    if (!is_abbreviation(s, i)) return false;

            // skip spaces
    size_t j = i + 1;
    while (j < s.size() && s[j] == ' ') j++;

            // If next token is lowercase, it's mid-sentence
    if (j < s.size() && std::islower((unsigned char)s[j]))
      return true;

    return false;
  }

  bool ends_sentence(const std::string& s, size_t i) const {
    char c = s[i];

    if (!is_mark(c)) return false;

    // collapse runs ?!...
    if (i + 1 < s.size() && is_mark(s[i + 1]))
      return false;

    if (c == '.') {
      if (is_decimal_point(s, i)) return false;
      if (is_enumeration_prefix(s, i)) return false;
      if (abbreviation_blocks_sentence(s, i)) return false;
      if (is_domain_suffix(s, i)) return false;
    }

    return true;
  }

  virtual bool verify_(const std::string& resp) const override {
    size_t count = 0;

    for (size_t i = 0; i < resp.size(); i++) {
      if (ends_sentence(resp, i)) count++;
    }
    return compare(count, (size_t)n_, rel_);
  }
};

class NumberWords : public Instruction {
 public:
  NumberWords(int num_words, Relation relation)
      : n_(num_words), rel_(relation) {}
  constexpr InstructionGroup Group() override { return LENGTH_CONSTRAINTS; }

 private:
  int n_;
  Relation rel_;
  virtual bool verify_(const std::string& resp) const override {
    size_t count = 0;
    bool in_word = false;
    for (unsigned char c : resp) {
      if (std::isalnum(c)) {
        if (!in_word) {
          in_word = true;
          ++count;
        }
      } else
        in_word = false;
    }
    return compare(count, (size_t)n_, rel_);
  }
};

/* -------------- PUNCTUATION -------------- */

class NoComma : public Instruction {
 public:
  NoComma() = default;
  constexpr InstructionGroup Group() override { return PUNCTUATION; }

 private:
  virtual bool verify_(const std::string& resp) const override {
    return resp.find(',') == std::string::npos;
  }
};

/* ---------------- STARTEND ---------------- */

class EndChecker : public Instruction {
 public:
  explicit EndChecker(std::string end_phrase) : end_(std::move(end_phrase)) {}
  constexpr InstructionGroup Group() override { return STARTEND; }

 private:
  std::string end_;
  virtual bool verify_(const std::string& resp) const override {
    return ends_with(resp, end_, 3);
  }
};

class Quotation : public Instruction {
 public:
  Quotation() = default;
  constexpr InstructionGroup Group() override { return STARTEND; }

 private:
  virtual bool verify_(const std::string& resp) const override {
    return resp.size() >= 2 && resp.front() == '"' && resp.back() == '"';
  }
};

struct Sample {
  int key;
  std::string prompt;
  std::vector<int> input_tokens;
  std::vector<std::unique_ptr<Instruction>> instructions;
};

}  // namespace ifeval
}  // namespace mobile
}  // namespace mlperf
#endif  // MLPERF_DATASETS_IFEVAL_UTILS_TYPES_H_
