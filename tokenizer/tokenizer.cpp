#include "tokenizer.hpp"

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "bee/file_path.hpp"
#include "bee/file_reader.hpp"
#include "bee/format.hpp"
#include "bee/or_error.hpp"

namespace tokenizer {
namespace {

struct TrieNode {
 public:
  TrieNode() {}

  void set_kind(const int kind) { _kind = kind; }

  const std::optional<int>& kind() const { return _kind; }

  const TrieNode* get_child(const char c) const
  {
    auto it = _children.find(c);
    if (it != _children.end()) { return &it->second; }
    return nullptr;
  }

  TrieNode* get_or_create_child(const char c) { return &_children[c]; }

 private:
  std::optional<int> _kind;
  std::map<char, TrieNode> _children;
};

TrieNode build_operators_trie(
  const std::vector<std::pair<std::string, int>>& operators)
{
  TrieNode root;
  for (const auto& [str, kind] : operators) {
    auto node = &root;
    for (const char c : str) { node = node->get_or_create_child(c); }
    node->set_kind(kind);
  }
  return root;
}

// const TrieNode OperatorsTrie = build_operators_trie();

struct Reader {
 public:
  Reader(const std::string_view content) : _content(content) {}

  char peek() const
  {
    if (_pos >= size()) {
      return 0;
    } else {
      return _content.at(_pos);
    }
  }

  char read()
  {
    const char out = peek();
    if (out == '\n') {
      _col = 1;
      _line++;
    } else {
      _col++;
    }
    _pos++;
    return out;
  }

  size_t size() const { return _content.size(); }
  size_t pos() const { return _pos; }
  bool eof() const { return pos() >= size(); }
  int col() const { return _col; }
  int line() const { return _line; }

  std::string_view substr(size_t begin, size_t end) const
  {
    return _content.substr(begin, end - begin);
  }

 private:
  const std::string_view _content;
  size_t _line = 1;
  size_t _col = 1;
  size_t _pos = 0;
};

bool is_alpha(char c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_digit(char c) { return c >= '0' && c <= '9'; }

bool is_first_identifier_char(char c) { return is_alpha(c) || (c == '_'); }

bool is_identifier_char(char c)
{
  return is_first_identifier_char(c) || is_digit(c);
}

bool is_space(char c) { return c == ' ' || c == '\n' || c == '\t'; }

bool is_comment_end(char c) { return c == '\n' || c == 0; }

struct TokenizerImpl final : public Tokenizer {
 public:
  TokenizerImpl(
    const std::vector<std::pair<std::string, int>>& operators,
    const std::map<std::string, int>& keywords)
      : _operators_trie(build_operators_trie(operators)), _keywords(keywords)
  {}

  virtual ~TokenizerImpl() {}

  bee::OrError<std::vector<Token>> tokenize(
    const std::string_view content, const bee::FilePath& src_path) override
  {
    Reader reader(content);

    std::vector<Token> tokens;
    while (!reader.eof()) {
      const char c = reader.peek();
      if (is_space(c)) {
        reader.read();
        continue;
      }
      auto token_line = reader.line();
      auto token_col = reader.col();
      auto token_pos = reader.pos();
      auto token_err = read_token(reader);
      if (token_err.is_error()) {
        return EF(
          "$:$:$ Invalid token: $",
          src_path,
          token_line,
          token_col,
          token_err.error());
      }
      auto token_type = *token_err;
      tokens.push_back({
        .type = token_type,
        .line = token_line,
        .col = token_col,
        .content = std::string(reader.substr(token_pos, reader.pos())),
      });
    }
    return tokens;
  }

 private:
  TokenType maybe_keyword(const std::string_view content) const
  {
    auto it = _keywords.find(std::string(content));
    if (it == _keywords.end()) { return TokenType(TokenKind::Identifier); }
    return TokenType::make_user_defined(it->second);
  }

  bee::OrError<TokenType> read_token(Reader& reader)
  {
    const auto begin = reader.pos();
    const char c = reader.read();

    if (c == '/' && reader.peek() == '/') {
      reader.read();
      while (!is_comment_end(reader.peek())) { reader.read(); }
      return TokenType(TokenKind::Comment);
    }

    if (is_first_identifier_char(c)) {
      while (is_identifier_char(reader.peek())) { reader.read(); }
      return maybe_keyword(reader.substr(begin, reader.pos()));
    }

    if (c == '"') {
      bool escaped = false;
      while (!reader.eof()) {
        auto c = reader.read();
        if (c == '\\') {
          escaped = true;
        } else if (!escaped && c == '"') {
          return TokenType(TokenKind::String);
        } else {
          escaped = false;
        }
      }
      return EF("Reached EOF before end of string");
    }

    if (is_digit(c) || c == '.') {
      bool has_period = c == '.';
      bool has_digit = is_digit(c);
      while (true) {
        const char c = reader.peek();
        if (c == '.') {
          if (has_period) { return EF("Numbers must contain at most one '.'"); }
          has_period = true;
        } else if (!is_digit(c)) {
          break;
        } else {
          has_digit = true;
        }
        reader.read();
      }
      if (has_digit) {
        if (reader.peek() == 'e') {
          reader.read();
          while (is_digit(reader.peek())) { reader.read(); }
        }
        return TokenType(TokenKind::Number);
      }
    }

    auto node = _operators_trie.get_child(c);
    if (node) {
      while (true) {
        auto next = node->get_child(reader.peek());
        if (next == nullptr) {
          const auto& k = node->kind();
          if (k.has_value()) { return TokenType::make_user_defined(*k); }
          break;
        } else {
          node = next;
          reader.read();
        }
      }
    }

    return EF("Unexpected character: '$'", c);
  }

  const TrieNode _operators_trie;
  const std::map<std::string, int> _keywords;
};

} // namespace

////////////////////////////////////////////////////////////////////////////////
// Tokenizer
//

Tokenizer::~Tokenizer() {}

bee::OrError<std::vector<Token>> Tokenizer::tokenize(
  const bee::FilePath& src_path)
{
  bail(content, bee::FileReader::read_file(src_path));
  return tokenize(content, src_path);
}

Tokenizer::ptr Tokenizer::create(
  const std::vector<std::pair<std::string, int>>& operators,
  const std::map<std::string, int>& keywords)
{
  return std::make_shared<TokenizerImpl>(operators, keywords);
}

} // namespace tokenizer
