#pragma once

#include "typed_token.hpp"

namespace tokenizer {

template <class TokenKindT> struct TokenReader {
 public:
  using Token = TypedToken<TokenKindT>;
  TokenReader(const std::vector<Token>& tokens, bool ignore_comments = false)
      : _tokens(tokens),
        _eof_token({
          .kind = TokenKindT::Eof,
          .line = tokens.back().line + 1,
          .col = 1,
          .content = "",
        }),
        _ignore_comments(ignore_comments)
  {
    _maybe_skip_comments();
  }
  bool eof() const { return _idx >= _tokens.size(); }
  const Token& peek() const
  {
    if (eof()) { return _eof_token; }
    return _tokens.at(_idx);
  }
  const Token& read()
  {
    if (eof()) { return _eof_token; }
    const auto& out = _tokens.at(_idx);
    _advance();
    return out;
  }
  void skip() { _advance(); }

 private:
  void _advance()
  {
    _idx++;
    _maybe_skip_comments();
  }
  void _maybe_skip_comments()
  {
    if (!_ignore_comments) return;
    while (_idx < _tokens.size()) {
      if (_tokens[_idx].kind != TokenKindT::Comment) { break; }
      ++_idx;
    }
  }
  size_t _idx = 0;
  const std::vector<Token> _tokens;
  const Token _eof_token;
  const bool _ignore_comments;
};

} // namespace tokenizer
