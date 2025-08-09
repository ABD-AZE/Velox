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
      c = inputFileStream.get();
      if (c == '.') {
        token.push(c);
        currentColumnNumber++;
        c = inputFileStream.get();
        if (c == '.') {
          // ellipsis found
          token.SetType(TokenType::ELLIPSIS);
          lastAcceptedTokenPos = inputFileStream.tellg();
          tokens.push_back(token);
          continue;
        }
        // this else block will be repeated a lot so better to export it as a
        // function
        else {
          // Careful review required
          if (lastAcceptedTokenPos == std::streampos(-1)) {
            errors.emplace_back(currentLineNumber, currentColumnNumber,
                                token.GetLexeme());
            continue;
          } else {
            // reset the fp to the last accepted state position and push the
            // token after popping relevant characters
          }
        }
      }
    }

    //-----------------------------------OPERATORS_END-------------------------------------
  }
  return tokens;
}