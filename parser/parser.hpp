#pragma once

#include "../ast/ast.hpp"
#include "../token/token.hpp"
#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

extern std::map<TokenType, int> Precedence;

struct ParserErrorInfo {
  int lineNumber;
  int columnNumber;
  std::string expected;
  TokenType actualToken;

  ParserErrorInfo(int line, int column, TokenType actual, std::string expected)
      : lineNumber(line), columnNumber(column), expected(expected),
        actualToken(actual) {}
};

class Parser {
public:
  Parser(const std::vector<Token> &tokens);
  // Error handling and status
  bool isSuccessful() const { return success; }
  const ASTNodePtr &parseProgram();
  const std::vector<ParserErrorInfo> &getErrors() const { return errors; }
  void printErrors() const;
  const ASTNodePtr &getAST() const { return ast; }

private:
  std::vector<Token> tokens;
  size_t tokenSize = 0;
  size_t currentIndex = tokenSize - 1; // Add this for parsing position
  ASTNodePtr ast;
  bool success = 1;
  std::vector<ParserErrorInfo> errors;

  /// gives the token at currentIndex and advances currentIndex
  Token get();
  // token at current index
  Token peek();
  /// pops from the vector
  Token consume();
  void reset();
  bool expect(TokenType actual, TokenType expected);
  /// gives the last token which got looked up using peek(), get() or consume()
  Token currentToken;

private:
  ASTNodePtr parseFunctionDefinition();
  ASTNodePtr parseStatement();
  ASTNodePtr parseExpression(int minPrecedence);
  ASTNodePtr parseFactor();
  ASTNodePtr parseDeclaration();
  ASTNodePtr parseBlockItem();
  ASTNodePtr parseBlock();
};