#include "token.hpp"

#include "bee/format.hpp"
#include "bee/mp.hpp"

namespace tokenizer {

const char* TokenKind::to_string() const
{
#define _(v)                                                                   \
  case v:                                                                      \
    return #v

  switch (_e) {
    _(Comment);
    _(String);
    _(Number);
    _(Identifier);
    _(UserDefined);
    _(Eof);
  }

#undef _
}

std::string Token::to_string() const
{
  auto token_desc = [&]() {
    switch (type.kind) {
    case TokenKind::Identifier:
    case TokenKind::String:
    case TokenKind::Number:
    case TokenKind::Comment:
    case TokenKind::Eof:
      return F("$($)", type.kind, content);
    case TokenKind::UserDefined:
      return F("$($)", type.user_defined_kind.value(), content);
    }
  };
  return F("line:$ col:$ $", line, col, token_desc());
}

} // namespace tokenizer
