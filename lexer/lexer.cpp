#include "lexer.hpp"

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
  // add pretty printing of tokens here
}

void Lexer::PrintErrors() const {
  // add pretty printing of errors here
}

const std::vector<Token> &Lexer::GenerateTokens() {
  // main logic goes here
  return tokens;
}