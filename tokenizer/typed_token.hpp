#pragma once

#include <string>

#include "token.hpp"

namespace tokenizer {

template <class TokenKindT> struct TypedToken {
 public:
  TokenKindT kind;
  int line;
  int col;
  std::string content;

  std::string to_string() const
  {
    auto token_desc = [](const TypedToken& t) {
      switch (t.kind) {
      case TokenKindT::Identifier:
      case TokenKindT::String:
      case TokenKindT::Number:
      case TokenKindT::Comment:
      case TokenKindT::Eof:
        return F("$($)", t.kind, t.content);
      default:
        return F(t.kind);
      }
    };
    return F("line:$ col:$ $", line, col, token_desc(*this));
  }

  static TypedToken of_int_token(const Token& token)
  {
    auto cast_kind = [&]() -> TokenKindT {
      switch (token.type.kind) {
      case TokenKind::Identifier:
        return TokenKindT::Identifier;
      case TokenKind::String:
        return TokenKindT::String;
      case TokenKind::Number:
        return TokenKindT::Number;
      case TokenKind::Comment:
        return TokenKindT::Comment;
      case TokenKind::Eof:
        return TokenKindT::Eof;
      case TokenKind::UserDefined:
        return static_cast<TokenKindT>(token.type.user_defined_kind.value());
      }
    };
    return {
      .kind = cast_kind(),
      .line = token.line,
      .col = token.col,
      .content = token.content,
    };
  }
};

} // namespace tokenizer
