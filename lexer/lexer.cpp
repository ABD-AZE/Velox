#include "lexer.hpp"
#include "termcolor.hpp"
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

static std::string TokenTypeToString(TokenType) {
  // TODO: replace with our real mapping
  return "TOKEN";
}

static std::vector<std::string> loadFileLines(const std::string& path) {
  std::ifstream in(path);
  std::vector<std::string> lines;
  std::string line;
  while(std::getline(in, line)){
    lines.push_back(line);
  } 
  return lines;
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

  if(tokens.empty()){
    std::cout<<yellow<<"[info] No tokens to print.\n"<<reset;
    return;
  }

  std::cout<<termcolor::bold
          <<std::format("{:<8} {:<7} {:<24} {}\n", "Line:Col", "Type", "Name", "Lexeme")
          <<termcolor::reset;

  for(const auto& tok:tokens){
    const int ln=tok.GetLineNumber();
    const int col=tok.GetColumnNumber();
    const auto  kind=tok.GetType();
    const std::string name=TokenTypeToString(kind);
    std::string lex=tok.GetLexeme();

    std::cout<<termcolor::grey
            <<std::format("{:<8} ",std::format("{}:{}",ln,col))
            <<std::format("{:<7} ",static_cast<int>(kind))
            <<termcolor::reset;

    std::cout<<termcolor::blue<<std::format("{:<24} ",name)<<termcolor::reset;
    std::cout<<termcolor::cyan<<std::format("'{}'\n",lex)<<termcolor::reset;
  }
}

void Lexer::PrintErrors() const {
  using namespace termcolor;

  if(errors.empty()){
    std::cout<<green<<"[ok] No lexical errors.\n"<<reset;
    return;
  }

  auto lines=loadFileLines(fileName);
  std::cout<<bold<<red<<"[errors] Lexical errors found: "<<errors.size()<<"\n"<<reset;

  for(const auto& e:errors){
    const int ln=e.lineNumber;
    const int col=e.columnNumber;

    std::cout<<red
            <<std::format("  {}:{}: unexpected character '{}'\n",ln,col,e.unexpectedChar)
            <<reset;

    if(ln>=1&&ln<=static_cast<int>(lines.size())){
      const std::string& src=lines[ln-1];
      std::cout<<"    "<<src<<"\n";
      std::cout<<"    ";
      int caretPos=std::max(1,col)-1;
      for(int i=0;i<caretPos;++i){
        std::cout<<' ';
      }
      std::cout<<red<<"^\n"<<reset;
    } 
    else {
      std::cout<<termcolor::grey<<"    (no source context available)\n"<<reset;
    }
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
