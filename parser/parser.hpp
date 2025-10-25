#pragma once

#include "../ast/ast.hpp"
#include "../semantic_analysis/semantic_analysis.hpp"
#include "../token/token.hpp"
#include "../utils/termcolor.hpp"
#include "../utils/token_classifier.hpp"
#include "../utils/type_utils.hpp"
#include <algorithm>
#include <format>
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
  ASTNodePtr &parseProgram();
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
  bool expect(TokenType actual, TokenType expected);
  /// gives the last token which got looked up using peek(), get() or consume()
  Token currentToken;

private:
  ASTNodePtr parseFunctionDeclaration();
  ASTNodePtr parseStatement();
  ASTNodePtr parseExpression(int minPrecedence);
  ASTNodePtr parseUnaryExp();
  ASTNodePtr parsePostfixExp();
  ASTNodePtr parsePrimaryExp();
  ASTNodePtr parseDeclaration();
  ASTNodePtr parseDeclarator();
  ASTNodePtr parseAbstractDeclarator();
  ASTNodePtr parseDirectDeclarator();
  ASTNodePtr parseDirectAbstractDeclarator();
  ASTNodePtr parseBlockItem();
  ASTNodePtr parseBlock();
  ASTNodePtr parseDoWhile();
  ASTNodePtr parseWhile();
  ASTNodePtr parseFor();
  ASTNodePtr parseVariableDeclaration();
  std::unique_ptr<InitializerNode> parseInitializer();
  /* returns {name, type, param names}
  name: Identifier
  Type: type of the declarator
  param names: for function declarators, list of parameter names. For
  non-function declarators, empty vector
  param types: for function declarators, list of parameter types. For
  non-function declarators, empty vector
  */
  std::tuple<std::string, Type, std::vector<std::string>, std::vector<Type>>
  processDeclarator(ASTNodePtr &declaratorNode, Type &baseType);
  Type processAbstractDeclarator(ASTNodePtr abstractDeclaratorNode,
                                 Type baseType);
  std::vector<paraminfo> parseParams();
  std::pair<std::vector<TokenType>, std::optional<TokenType>>
  parseSpecifierList(
      std::vector<TokenType>
          specifier_list); // returns {vector<type>, storage_class}
  Type parseTypeSpecifierList(std::vector<TokenType> list);
};