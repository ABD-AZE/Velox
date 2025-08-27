#pragma once
#include "../token/token.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

std::string TokenTypeToString(TokenType type);

class Lexer {
public:
  Lexer(const std::string &file);
  void PrintSymbolTable() const;
  void PrintTokens() const;
  void PrintErrors() const;
  const std::vector<Token> &GenerateTokens();
  void ErrorRecovery(Token &token, std::streampos lastAcceptedTokenPos,
                     TokenType lastAcceptedTokenType,
                     int lastAcceptedColumnNumber, int lastAcceptedLineNumber);

  void initializeToken(Token &token, char c, TokenType tokenType,
                       int &lastAcceptedColumnNumber,
                       int &lastAcceptedLineNumber,
                       std::streampos &lastAcceptedTokenPos,
                       TokenType &lastAcceptedTokenType);

  bool success = 1;

private:
  std::string fileName;
  std::vector<Token> tokens;
  std::vector<ErrorInfo> errors;
  std::ifstream inputFileStream;
  int currentLineNumber;
  int currentColumnNumber;
  // SymbolTable symbolTable;
};