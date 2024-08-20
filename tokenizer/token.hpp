#pragma once

#include <array>
#include <string>

#include "bee/format.hpp"

namespace tokenizer {

struct TokenKind {
 public:
  enum Enum {
    Comment,
    String,
    Number,
    Identifier,
    UserDefined,
    Eof,
  };
  constexpr TokenKind(Enum v) : _e(v) {}

  constexpr operator Enum() const { return _e; }

  static constexpr size_t count = 7;

  const char* to_string() const;

 private:
  Enum _e;
};

struct TokenType {
 public:
  TokenKind kind;
  std::optional<int> user_defined_kind;

  static TokenType make_user_defined(int user_defined_kind)
  {
    return {
      .kind = TokenKind::UserDefined, .user_defined_kind = user_defined_kind};
  }
};

struct Token {
 public:
  TokenType type;
  int line;
  int col;
  std::string content;

  std::string to_string() const;
};

} // namespace tokenizer
