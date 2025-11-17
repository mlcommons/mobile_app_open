/*BSD 3-Clause License

Copyright (c) 2014-2017, ipkn
              2020-2025, CrowCpp
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the author nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/
#pragma once

#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <cstdint>
#include <cstdlib>

namespace crow {
namespace json {

enum class type {
  Null,
  False,
  True,
  Number,
  String,
  List,
  Object
};

enum class num_type {
  Null,
  Signed_integer,
  Unsigned_integer,
  Floating_point
};

class rvalue {
 public:
  using list   = std::vector<rvalue>;
  using object = std::map<std::string, rvalue>;

  rvalue() : t_(type::Null), nt_(num_type::Null), error_(true) {} // default = invalid

  explicit operator bool() const noexcept { return !error_; }

  type      t()  const noexcept { return t_; }
  num_type  nt() const noexcept { return nt_; }

          // Accessors (no bounds checking, for brevity)
  const list&   as_list()   const { return list_; }
  const object& as_object() const { return object_; }
  const std::string& as_string() const { return str_; }

  int64_t   as_i() const { return i_; }
  uint64_t  as_u() const { return u_; }
  double    as_d() const { return d_; }

          // Convenience index operators (no error checking)
  const rvalue& operator[](std::size_t idx) const {
    static rvalue invalid;
    if (t_ != type::List || idx >= list_.size())
      return invalid;
    return list_[idx];
  }

  const rvalue& operator[](const std::string& key) const {
    static rvalue invalid;
    if (t_ != type::Object) return invalid;
    auto it = object_.find(key);
    if (it == object_.end()) return invalid;
    return it->second;
  }

  bool is_valid() const noexcept { return !error_; }

 private:
  friend class parser;

  void set_error() noexcept { error_ = true; }

  void set_null() noexcept {
    t_ = type::Null;
    nt_ = num_type::Null;
  }

  void set_bool(bool v) noexcept {
    t_ = v ? type::True : type::False;
    nt_ = num_type::Null;
  }

  void set_number_signed(int64_t v) noexcept {
    t_  = type::Number;
    nt_ = num_type::Signed_integer;
    i_  = v;
    u_  = static_cast<uint64_t>(v);
    d_  = static_cast<double>(v);
  }

  void set_number_unsigned(uint64_t v) noexcept {
    t_  = type::Number;
    nt_ = num_type::Unsigned_integer;
    u_  = v;
    i_  = static_cast<int64_t>(v);
    d_  = static_cast<double>(v);
  }

  void set_number_double(double v) noexcept {
    t_  = type::Number;
    nt_ = num_type::Floating_point;
    d_  = v;
    i_  = static_cast<int64_t>(v);
    u_  = static_cast<uint64_t>(v);
  }

  void set_string(std::string v) {
    t_  = type::String;
    nt_ = num_type::Null;
    str_ = std::move(v);
  }

  void set_list(list v) {
    t_  = type::List;
    nt_ = num_type::Null;
    list_ = std::move(v);
  }

  void set_object(object v) {
    t_  = type::Object;
    nt_ = num_type::Null;
    object_ = std::move(v);
  }

  type     t_;
  num_type nt_;
  bool     error_ = false;

          // basic storage
  int64_t   i_ = 0;
  uint64_t  u_ = 0;
  double    d_ = 0.0;
  std::string str_;
  list       list_;
  object     object_;
};

class parser {
 public:
  parser(const char* begin, std::size_t size)
      : cur_(begin), end_(begin + size) {}

  rvalue parse() {
    rvalue rv;
    skip_ws();
    parse_value(rv);
    if (!rv) return rv;

    skip_ws();
    if (cur_ != end_) {
      // trailing garbage
      rv.set_error();
    }
    return rv;
  }

 private:
  void skip_ws() {
    while (cur_ != end_ && std::isspace(static_cast<unsigned char>(*cur_)))
      ++cur_;
  }

  void parse_value(rvalue& out) {
    if (cur_ == end_) {
      out.set_error();
      return;
    }

    switch (*cur_) {
      case 'n': parse_null(out);  break;
      case 't': parse_true(out);  break;
      case 'f': parse_false(out); break;
      case '"': parse_string(out); break;
      case '[': parse_array(out);  break;
      case '{': parse_object(out); break;
      default:
        if (*cur_ == '-' || std::isdigit(static_cast<unsigned char>(*cur_))) {
          parse_number(out);
        } else {
          out.set_error();
        }
        break;
    }
  }

  void parse_null(rvalue& out) {
    if (consume_literal("null")) {
      out.set_null();
      out.error_ = false;
    } else {
      out.set_error();
    }
  }

  void parse_true(rvalue& out) {
    if (consume_literal("true")) {
      out.set_bool(true);
      out.error_ = false;
    } else {
      out.set_error();
    }
  }

  void parse_false(rvalue& out) {
    if (consume_literal("false")) {
      out.set_bool(false);
      out.error_ = false;
    } else {
      out.set_error();
    }
  }

  bool consume_literal(const char* lit) {
    const char* p = cur_;
    while (*lit && p != end_ && *p == *lit) {
      ++p; ++lit;
    }
    if (*lit == '\0') {
      cur_ = p;
      return true;
    }
    return false;
  }

  void parse_string(rvalue& out) {
    if (cur_ == end_ || *cur_ != '"') {
      out.set_error();
      return;
    }
    ++cur_; // skip opening quote

    std::string result;
    while (cur_ != end_) {
      char c = *cur_++;
      if (c == '"') {
        out.set_string(std::move(result));
        out.error_ = false;
        return;
      }
      if (c == '\\') {
        if (cur_ == end_) { out.set_error(); return; }
        char esc = *cur_++;
        switch (esc) {
          case '"': result.push_back('"'); break;
          case '\\': result.push_back('\\'); break;
          case '/': result.push_back('/'); break;
          case 'b': result.push_back('\b'); break;
          case 'f': result.push_back('\f'); break;
          case 'n': result.push_back('\n'); break;
          case 'r': result.push_back('\r'); break;
          case 't': result.push_back('\t'); break;
          case 'u':
            // minimal \uXXXX handling: skip 4 hex digits, no actual UTF-16 decode
            if (end_ - cur_ < 4) { out.set_error(); return; }
            for (int i = 0; i < 4; ++i) {
              if (!std::isxdigit(static_cast<unsigned char>(cur_[i]))) {
                out.set_error();
                return;
              }
            }
            // Just store as-is or replace with '?'
            result.push_back('?');
            cur_ += 4;
            break;
          default:
            out.set_error();
            return;
        }
      } else {
        result.push_back(c);
      }
    }
    // Unterminated string
    out.set_error();
  }

  void parse_number(rvalue& out) {
    const char* start = cur_;

    if (*cur_ == '-') ++cur_;
    if (cur_ == end_) { out.set_error(); return; }

    if (*cur_ == '0') {
      ++cur_;
    } else if (std::isdigit(static_cast<unsigned char>(*cur_))) {
      while (cur_ != end_ && std::isdigit(static_cast<unsigned char>(*cur_)))
        ++cur_;
    } else {
      out.set_error();
      return;
    }

    bool is_float = false;
    if (cur_ != end_ && *cur_ == '.') {
      is_float = true;
      ++cur_;
      if (cur_ == end_ || !std::isdigit(static_cast<unsigned char>(*cur_))) {
        out.set_error();
        return;
      }
      while (cur_ != end_ && std::isdigit(static_cast<unsigned char>(*cur_)))
        ++cur_;
    }

    if (cur_ != end_ && (*cur_ == 'e' || *cur_ == 'E')) {
      is_float = true;
      ++cur_;
      if (cur_ != end_ && (*cur_ == '+' || *cur_ == '-'))
        ++cur_;
      if (cur_ == end_ || !std::isdigit(static_cast<unsigned char>(*cur_))) {
        out.set_error();
        return;
      }
      while (cur_ != end_ && std::isdigit(static_cast<unsigned char>(*cur_)))
        ++cur_;
    }

    std::string num_str(start, cur_);
    char* endptr = nullptr;

    if (is_float) {
      double v = std::strtod(num_str.c_str(), &endptr);
      if (endptr != num_str.c_str() + num_str.size()) {
        out.set_error();
        return;
      }
      out.set_number_double(v);
    } else {
      bool negative = (num_str[0] == '-');
      if (negative) {
        long long v = std::strtoll(num_str.c_str(), &endptr, 10);
        if (endptr != num_str.c_str() + num_str.size()) {
          out.set_error();
          return;
        }
        out.set_number_signed(static_cast<int64_t>(v));
      } else {
        unsigned long long v = std::strtoull(num_str.c_str(), &endptr, 10);
        if (endptr != num_str.c_str() + num_str.size()) {
          out.set_error();
          return;
        }
        out.set_number_unsigned(static_cast<uint64_t>(v));
      }
    }
    out.error_ = false;
  }

  void parse_array(rvalue& out) {
    if (*cur_ != '[') { out.set_error(); return; }
    ++cur_;
    skip_ws();

    rvalue::list elems;
    if (cur_ != end_ && *cur_ == ']') {
      ++cur_;
      out.set_list(std::move(elems));
      out.error_ = false;
      return;
    }

    while (true) {
      rvalue elem;
      skip_ws();
      parse_value(elem);
      if (!elem) { out.set_error(); return; }
      elems.push_back(std::move(elem));

      skip_ws();
      if (cur_ == end_) { out.set_error(); return; }
      if (*cur_ == ',') {
        ++cur_;
        skip_ws();
        continue;
      }
      if (*cur_ == ']') {
        ++cur_;
        break;
      }
      out.set_error();
      return;
    }

    out.set_list(std::move(elems));
    out.error_ = false;
  }

  void parse_object(rvalue& out) {
    if (*cur_ != '{') { out.set_error(); return; }
    ++cur_;
    skip_ws();

    rvalue::object obj;
    if (cur_ != end_ && *cur_ == '}') {
      ++cur_;
      out.set_object(std::move(obj));
      out.error_ = false;
      return;
    }

    while (true) {
      skip_ws();
      rvalue key_rv;
      parse_string(key_rv);
      if (!key_rv || key_rv.t() != type::String) { out.set_error(); return; }
      std::string key = key_rv.as_string();

      skip_ws();
      if (cur_ == end_ || *cur_ != ':') { out.set_error(); return; }
      ++cur_;

      skip_ws();
      rvalue value_rv;
      parse_value(value_rv);
      if (!value_rv) { out.set_error(); return; }

      obj.emplace(std::move(key), std::move(value_rv));

      skip_ws();
      if (cur_ == end_) { out.set_error(); return; }
      if (*cur_ == ',') {
        ++cur_;
        skip_ws();
        continue;
      }
      if (*cur_ == '}') {
        ++cur_;
        break;
      }
      out.set_error();
      return;
    }

    out.set_object(std::move(obj));
    out.error_ = false;
  }

  const char* cur_;
  const char* end_;
};

inline rvalue load(const char* data, std::size_t size) {
  parser p(data, size);
  return p.parse();
}

inline rvalue load(const char* data) {
  std::size_t len = 0;
  while (data[len] != '\0') ++len;
  return load(data, len);
}

inline rvalue load(const std::string& s) {
  return load(s.data(), s.size());
}

} // namespace json
} // namespace crow
