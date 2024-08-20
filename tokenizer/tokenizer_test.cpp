#include "tokenizer.hpp"

#include "bee/testing.hpp"

namespace tokenizer {
namespace {

struct UserTokenKind {
 public:
  enum Enum {
    Empty,
    Comment,
    Identifier,
    Fn,
    If,
    Else,
    For,
    Break,
    Return,
    Div,
    Mul,
    Add,
    Sub,
    Assign,
    Equal,
    Less,
    LessEq,
    Greater,
    GreaterEq,
    Colon,
    Semicolon,
    String,
    Number,
    OpenParens,
    CloseParens,
    OpenBraces,
    CloseBraces,
    OpenBrackets,
    CloseBrackets,
    Eof,
  };
  constexpr UserTokenKind(Enum e) : _e(e) {}
  constexpr explicit UserTokenKind(int e) : _e(Enum(e)) {}
  operator Enum() const { return _e; }

  const char* to_string() const
  {
#define _(v)                                                                   \
  case v:                                                                      \
    return #v;

    switch (_e) {
      _(Empty);
      _(Comment);
      _(Identifier);
      _(Fn);
      _(If);
      _(Else);
      _(For);
      _(Break);
      _(Return);
      _(Div);
      _(Mul);
      _(Add);
      _(Sub);
      _(Assign);
      _(Equal);
      _(Less);
      _(LessEq);
      _(Greater);
      _(GreaterEq);
      _(Colon);
      _(Semicolon);
      _(String);
      _(Number);
      _(OpenParens);
      _(CloseParens);
      _(OpenBraces);
      _(CloseBraces);
      _(OpenBrackets);
      _(CloseBrackets);
      _(Eof);
    }

#undef _
  }

 private:
  Enum _e;
};

const std::vector<std::pair<std::string, UserTokenKind>> operators = {
  {"*", UserTokenKind::Mul},
  {"/", UserTokenKind::Div},
  {"+", UserTokenKind::Add},
  {"-", UserTokenKind::Sub},
  {"=", UserTokenKind::Assign},
  {"==", UserTokenKind::Equal},
  {"<", UserTokenKind::Less},
  {"<=", UserTokenKind::LessEq},
  {">", UserTokenKind::Greater},
  {">=", UserTokenKind::GreaterEq},
  {":", UserTokenKind::Colon},
  {";", UserTokenKind::Semicolon},
  {"(", UserTokenKind::OpenParens},
  {")", UserTokenKind::CloseParens},
  {"{", UserTokenKind::OpenBraces},
  {"}", UserTokenKind::CloseBraces},
  {"[", UserTokenKind::OpenBrackets},
  {"]", UserTokenKind::CloseBrackets},
};

const std::map<std::string, UserTokenKind> keywords{
  {"fn", UserTokenKind::Fn},
  {"if", UserTokenKind::If},
  {"else", UserTokenKind::Else},
  {"for", UserTokenKind::For},
  {"return", UserTokenKind::Return},
  {"break", UserTokenKind::Break},
};

TEST(basic)
{
  TypedTokenizer<UserTokenKind> tok(operators, keywords);
  must(
    tokens,
    tok.tokenize("foo bar ( ) { } ( ) == <= <<", bee::FilePath("test")));
  for (const auto& t : tokens) P(t);
}

} // namespace
} // namespace tokenizer
