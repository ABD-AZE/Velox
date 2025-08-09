#include "lexer.hpp"
#include "termcolor.hpp"
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>

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
    if (auto it = TokenTypeNames.find(type); it != TokenTypeNames.end())
        return it->second;
    return "UNKNOWN_TOKEN";
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
