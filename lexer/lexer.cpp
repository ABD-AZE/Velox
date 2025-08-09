#include "lexer.hpp"
#include "../utils/char_classifier.hpp"
#include "termcolor.hpp"
#include <algorithm>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

static const std::unordered_map<TokenType, std::string> TokenTypeNames = {
    {TokenType::WS, "WS"},
    {TokenType::LINE_COMMENT, "LINE_COMMENT"},
    {TokenType::BLOCK_COMMENT, "BLOCK_COMMENT"},
    {TokenType::ELLIPSIS, "ELLIPSIS"},
    {TokenType::COMPOUND_RIGHTSHIFT, "COMPOUND_RIGHTSHIFT"},
    {TokenType::COMPOUND_LEFTSHIFT, "COMPOUND_LEFTSHIFT"},
    {TokenType::COMPOUND_SUM, "COMPOUND_SUM"},
    {TokenType::COMPOUND_DIFFERENCE, "COMPOUND_DIFFERENCE"},
    {TokenType::COMPOUND_PRODUCT, "COMPOUND_PRODUCT"},
    {TokenType::COMPOUND_DIVISION, "COMPOUND_DIVISION"},
    {TokenType::COMPOUND_REMAINDER, "COMPOUND_REMAINDER"},
    {TokenType::COMPOUND_AND, "COMPOUND_AND"},
    {TokenType::COMPOUND_XOR, "COMPOUND_XOR"},
    {TokenType::COMPOUND_OR, "COMPOUND_OR"},
    {TokenType::EQUAL, "EQUAL"},
    {TokenType::NOTEQUAL, "NOTEQUAL"},
    {TokenType::LESSTHANEQUAL, "LESSTHANEQUAL"},
    {TokenType::GREATERTHANEQUAL, "GREATERTHANEQUAL"},
    {TokenType::INCREMENT_OPERATOR, "INCREMENT_OPERATOR"},
    {TokenType::DECREMENT_OPERATOR, "DECREMENT_OPERATOR"},
    {TokenType::ARROW_OPERATOR, "ARROW_OPERATOR"},
    {TokenType::LEFT_SHIFT, "LEFT_SHIFT"},
    {TokenType::RIGHT_SHIFT, "RIGHT_SHIFT"},
    {TokenType::LAND, "LAND"},
    {TokenType::LOR, "LOR"},
    {TokenType::VOID, "VOID"},
    {TokenType::RETURN, "RETURN"},
    {TokenType::IF, "IF"},
    {TokenType::ELSE, "ELSE"},
    {TokenType::DO, "DO"},
    {TokenType::WHILE, "WHILE"},
    {TokenType::FOR, "FOR"},
    {TokenType::BREAK, "BREAK"},
    {TokenType::CONTINUE, "CONTINUE"},
    {TokenType::STATIC, "STATIC"},
    {TokenType::EXTERN, "EXTERN"},
    {TokenType::INT, "INT"},
    {TokenType::LONG, "LONG"},
    {TokenType::SIGNED, "SIGNED"},
    {TokenType::UNSIGNED, "UNSIGNED"},
    {TokenType::DOUBLE, "DOUBLE"},
    {TokenType::CHAR, "CHAR"},
    {TokenType::SIZEOF, "SIZEOF"},
    {TokenType::STRUCT, "STRUCT"},
    {TokenType::GOTO, "GOTO"},
    {TokenType::SWITCH, "SWITCH"},
    {TokenType::CASE, "CASE"},
    {TokenType::DEFAULT_CASE, "DEFAULT_CASE"},
    {TokenType::PRINTF, "PRINTF"},
    {TokenType::SCANF, "SCANF"},
    {TokenType::TYPEDEF, "TYPEDEF"},
    {TokenType::ENUM, "ENUM"},
    {TokenType::UNION, "UNION"},
    {TokenType::FLOAT_CONSTANT, "FLOAT_CONSTANT"},
    {TokenType::ULONG_CONSTANT, "ULONG_CONSTANT"},
    {TokenType::LONG_CONSTANT, "LONG_CONSTANT"},
    {TokenType::UINT_CONSTANT, "UINT_CONSTANT"},
    {TokenType::INT_CONSTANT, "INT_CONSTANT"},
    {TokenType::CHARACTER, "CHARACTER"},
    {TokenType::STRING, "STRING"},
    {TokenType::IDENTIFIER, "IDENTIFIER"},
    {TokenType::OPEN_PARENTHESES, "OPEN_PARENTHESES"},
    {TokenType::CLOSE_PARENTHESES, "CLOSE_PARENTHESES"},
    {TokenType::OPEN_BRACE, "OPEN_BRACE"},
    {TokenType::CLOSE_BRACE, "CLOSE_BRACE"},
    {TokenType::OPEN_BRACKET, "OPEN_BRACKET"},
    {TokenType::CLOSE_BRACKET, "CLOSE_BRACKET"},
    {TokenType::SEMICOLON, "SEMICOLON"},
    {TokenType::COLON, "COLON"},
    {TokenType::COMMA, "COMMA"},
    {TokenType::DOT, "DOT"},
    {TokenType::QUESTION_MARK, "QUESTION_MARK"},
    {TokenType::ASSIGNMENT, "ASSIGNMENT"},
    {TokenType::PLUS, "PLUS"},
    {TokenType::HYPHEN, "HYPHEN"},
    {TokenType::ASTERISK, "ASTERISK"},
    {TokenType::FORWARD_SLASH, "FORWARD_SLASH"},
    {TokenType::PERCENT_SIGN, "PERCENT_SIGN"},
    {TokenType::TILDE, "TILDE"},
    {TokenType::NOT, "NOT"},
    {TokenType::AMP, "AMP"},
    {TokenType::LESSTHAN, "LESSTHAN"},
    {TokenType::GREATERTHAN, "GREATERTHAN"},
    {TokenType::AAND, "AAND"},
    {TokenType::AOR, "AOR"},
    {TokenType::XOR, "XOR"},
};

static std::string TokenTypeToString(TokenType type) {
  if (auto it = TokenTypeNames.find(type); it != TokenTypeNames.end()) {
    return it->second;
  }
  return "UNKNOWN_TOKEN";
}

Lexer::Lexer(const std::string &file) : inputFileStream(file) {
  inputFileStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  if (!inputFileStream.is_open()) {
    throw std::runtime_error("Failed to open file: " + file);
  }
}

void Lexer::PrintSymbolTable() const {
  // unimplemented
}

void Lexer::PrintTokens() const {
  using namespace termcolor;

  if (tokens.empty()) {
    std::cout << yellow << "[info] No tokens to print.\n" << reset;
    return;
  }

  std::cout << termcolor::bold
            << std::format("{:<8} {:<7} {:<24} {}\n", "Line:Col", "Type",
                           "Name", "Lexeme")
            << termcolor::reset;

  for (const auto &tok : tokens) {
    const int ln = tok.GetLineNumber();
    const int col = tok.GetColumnNumber();
    const auto kind = tok.GetType();
    const std::string name = TokenTypeToString(kind);
    std::string lex = tok.GetLexeme();

    std::cout << termcolor::grey
              << std::format("{:<8} ", std::format("{}:{}", ln, col))
              << std::format("{:<7} ", static_cast<int>(kind))
              << termcolor::reset;

    std::cout << termcolor::blue << std::format("{:<24} ", name)
              << termcolor::reset;
    std::cout << termcolor::cyan << std::format("'{}'\n", lex)
              << termcolor::reset;
  }
}

void Lexer::PrintErrors() const {
  using namespace termcolor;

  if (errors.empty()) {
    std::cout << green << "[ok] No lexical errors.\n" << reset;
    return;
  }

  std::cout << bold << red << "[errors] Lexical errors found: " << errors.size()
            << "\n"
            << reset;

  for (const auto &e : errors) {
    std::cout << red
              << std::format("  {}:{}: unexpected token '{}'\n", e.lineNumber,
                             e.columnNumber, e.unidentifiedToken)
              << reset;
  }
}

void Lexer::ErrorRecovery(Token &token, std::streampos lastAcceptedTokenPos,
                          TokenType lastAcceptedTokenType) {
  if (lastAcceptedTokenPos == std::streampos(-1)) {
    errors.emplace_back(token.GetLineNumber(), token.GetColumnNumber(),
                        token.GetLexeme());
  } else {
    while (inputFileStream.tellg() > lastAcceptedTokenPos) {
      token.pop();
      inputFileStream.unget();
    }
    token.SetType(lastAcceptedTokenType);
    tokens.push_back(token);
  }
}

/*
NOTE for self: peek() doesn't move fp, get() does
*/

const std::vector<Token> &Lexer::GenerateTokens() {
  // main logic goes here
  currentLineNumber = 1;
  currentColumnNumber = 1;
  char c;
  Token token;
  std::streampos lastAcceptedTokenPos =
      std::streampos(-1); // default invalid position
  TokenType lastAcceptedTokenType;
  while (c = inputFileStream.get()) {
    //-----------------------------------COMMENTS_START------------------------------------

    if (c == '\\') {

    }

    //-----------------------------------COMMENTS_END--------------------------------------

    //-----------------------------------OPERATORS_START-----------------------------------

    // potential tokens: FLOAT_CONSTANT, DOT, ELLIPSIS
    else if (c == '.') {
      token.push(c);
      token.SetColumnNumber(currentColumnNumber);
      token.SetLineNumber(currentLineNumber);
      lastAcceptedTokenPos = inputFileStream.tellg();
      lastAcceptedTokenType = TokenType::DOT;
      c = inputFileStream.get();
      if (c == '.') {
        token.push(c);
        currentColumnNumber++;
        c = inputFileStream.get();
        if (c == '.') {
          // ellipsis found
          token.SetType(TokenType::ELLIPSIS);
          lastAcceptedTokenPos = inputFileStream.tellg();
          lastAcceptedTokenType = TokenType::ELLIPSIS;
          tokens.push_back(token);
          continue;
        } else {
          // Careful review required
          ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType);
          continue;
        }
      } else if (isDigit(c)) {
        // potential FLOAT_CONSTANT
        token.SetType(TokenType::FLOAT_CONSTANT);
        while (isDigit(c)) {
          token.push(c);
          currentColumnNumber++;
          c = inputFileStream.get();
          lastAcceptedTokenPos = inputFileStream.tellg();
          lastAcceptedTokenType = TokenType::FLOAT_CONSTANT;
        }
        ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType);
      } else {
        // dot found
        token.SetType(TokenType::DOT);
        tokens.push_back(token);
        inputFileStream.unget(); 
        continue;
      }
    }

    // potential tokens: PLUS, COMPOUND_SUM, INCREMENT_OPERATOR
    if (c == '+') {
      token.push(c);
      token.SetColumnNumber(currentColumnNumber);
      token.SetLineNumber(currentLineNumber);
      lastAcceptedTokenPos = inputFileStream.tellg();
      lastAcceptedTokenType = TokenType::PLUS;
      c = inputFileStream.get();
      if (c == '+') {
        token.push(c);
        currentColumnNumber++;
        token.SetType(TokenType::INCREMENT_OPERATOR);
        lastAcceptedTokenPos = inputFileStream.tellg();
        lastAcceptedTokenType = TokenType::INCREMENT_OPERATOR;
        tokens.push_back(token);
      } else if (c == '=') {
        token.push(c);
        currentColumnNumber++;
        token.SetType(TokenType::COMPOUND_SUM);
        lastAcceptedTokenPos = inputFileStream.tellg();
        lastAcceptedTokenType = TokenType::COMPOUND_SUM;
        tokens.push_back(token);
      } else {
        ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType);
      }
      continue;
    }
    //-----------------------------------OPERATORS_END-------------------------------------

    //-----------------------------------PRADY---------------------------------------------
    
    //-----------------------------------PRADY_END-----------------------------------------
  }
  return tokens;
}
