#pragma once

#include <map>
#include <memory>
#include <vector>

#include "token.hpp"
#include "typed_token.hpp"

#include "bee/file_path.hpp"
#include "bee/or_error.hpp"

namespace tokenizer {

struct Tokenizer {
 public:
  using ptr = std::shared_ptr<Tokenizer>;

  static ptr create(
    const std::vector<std::pair<std::string, int>>& operators,
    const std::map<std::string, int>& keywords);

  virtual ~Tokenizer();

  bee::OrError<std::vector<Token>> tokenize(const bee::FilePath& src_path);

  virtual bee::OrError<std::vector<Token>> tokenize(
    const std::string_view content, const bee::FilePath& src_path) = 0;
};

template <class UserTokenKindT> struct TypedTokenizer {
 public:
  using TypedToken = TypedToken<UserTokenKindT>;

  TypedTokenizer(
    const std::vector<std::pair<std::string, UserTokenKindT>>& operators,
    const std::map<std::string, UserTokenKindT>& keywords)
  {
    std::vector<std::pair<std::string, int>> int_operators;
    std::map<std::string, int> int_keywords;

    for (const auto& op : operators) {
      int_operators.emplace_back(op.first, static_cast<int>(op.second));
    }
    for (const auto& op : keywords) {
      int_keywords.emplace(op.first, static_cast<int>(op.second));
    }

    _tokenizer = Tokenizer::create(int_operators, int_keywords);
  }

  bee::OrError<std::vector<TypedToken>> tokenize(const bee::FilePath& src_path)
  {
    bail(tokens, _tokenizer->tokenize(src_path));
    return cast_tokens(tokens);
  }

  bee::OrError<std::vector<TypedToken>> tokenize(
    const std::string_view content, const bee::FilePath& src_path)
  {
    bail(tokens, _tokenizer->tokenize(content, src_path));
    return cast_tokens(tokens);
  }

 private:
  std::vector<TypedToken> cast_tokens(const std::vector<Token>& tokens)
  {
    std::vector<TypedToken> out;
    for (const auto& t : tokens) { out.push_back(TypedToken::of_int_token(t)); }
    return out;
  }

  Tokenizer::ptr _tokenizer;
};

} // namespace tokenizer
