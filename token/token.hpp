#pragma once
#include <string>
#include <unordered_map>

// Refer token.md for the list of tokens and their description
enum TokenType {
  WS,
  LINE_COMMENT,
  BLOCK_COMMENT,
  ELLIPSIS,
  COMPOUND_RIGHTSHIFT,
  COMPOUND_LEFTSHIFT,
  COMPOUND_SUM,
  COMPOUND_DIFFERENCE,
  COMPOUND_PRODUCT,
  COMPOUND_DIVISION,
  COMPOUND_REMAINDER,
  COMPOUND_AND,
  COMPOUND_XOR,
  COMPOUND_OR,
  EQUAL,
  NOTEQUAL,
  LESSTHANEQUAL,
  GREATERTHANEQUAL,
  INCREMENT_OPERATOR,
  DECREMENT_OPERATOR,
  ARROW_OPERATOR,
  LEFT_SHIFT,
  RIGHT_SHIFT,
  LAND,
  LOR,
  VOID,
  RETURN,
  IF,
  ELSE,
  DO,
  WHILE,
  FOR,
  BREAK,
  CONTINUE,
  STATIC,
  EXTERN,
  INT,
  LONG,
  SIGNED,
  UNSIGNED,
  DOUBLE,
  CHAR,
  SIZEOF,
  STRUCT,
  CLASS,
  PUBLIC,
  PRIVATE,
  GOTO,
  SWITCH,
  CASE,
  DEFAULT_CASE,
  PRINTF,
  SCANF,
  TYPEDEF,
  ENUM,
  UNION,
  FLOAT_CONSTANT,
  ULONG_CONSTANT,
  LONG_CONSTANT,
  UINT_CONSTANT,
  INT_CONSTANT,
  CHARACTER,
  STRING,
  IDENTIFIER,
  OPEN_PARENTHESES,
  CLOSE_PARENTHESES,
  OPEN_BRACE,
  CLOSE_BRACE,
  OPEN_BRACKET,
  CLOSE_BRACKET,
  SEMICOLON,
  COLON,
  COMMA,
  DOT,
  QUESTION_MARK,
  ASSIGNMENT,
  PLUS,
  HYPHEN,
  ASTERISK,
  FORWARD_SLASH,
  PERCENT_SIGN,
  TILDE,
  NOT,
  LESSTHAN,
  GREATERTHAN,
  AAND,
  AOR,
  XOR,
};

struct ErrorInfo {
  int lineNumber;
  int columnNumber;
  const std::string unidentifiedToken;

  ErrorInfo(int line, int column, const std::string &lexeme)
      : lineNumber(line), columnNumber(column), unidentifiedToken(lexeme) {}
};

class Token {
public:
  TokenType GetType() const { return type; }
  const std::string &GetLexeme() const { return lexeme; }
  int GetLineNumber() const { return lineNumber; }
  int GetColumnNumber() const { return columnNumber; }
  void SetType(TokenType t) { type = t; }
  void SetLineNumber(int line) { lineNumber = line; } // starting line number
  void SetColumnNumber(int column) {
    columnNumber = column;
  } // starting column number
  void push(char c) { lexeme.push_back(c); }
  void reset() { lexeme.clear(); }
  void pop() { lexeme.pop_back(); }
  void pop_front() {
    if (!lexeme.empty()) {
      lexeme.erase(lexeme.begin());
    }
  }

private:
  TokenType type;
  std::string lexeme;
  int lineNumber = 0;
  int columnNumber = 0;
};

const std::unordered_map<TokenType, std::string> TokenTypeNames = {
    {TokenType::WS, "WS"},
    {TokenType::LINE_COMMENT, "LINE_COMMENT"},               // //
    {TokenType::BLOCK_COMMENT, "BLOCK_COMMENT"},             // /* */
    {TokenType::ELLIPSIS, "ELLIPSIS"},                       // ...
    {TokenType::COMPOUND_RIGHTSHIFT, "COMPOUND_RIGHTSHIFT"}, // >>=
    {TokenType::COMPOUND_LEFTSHIFT, "COMPOUND_LEFTSHIFT"},   // <<=
    {TokenType::COMPOUND_SUM, "COMPOUND_SUM"},               // +=
    {TokenType::COMPOUND_DIFFERENCE, "COMPOUND_DIFFERENCE"}, // -=
    {TokenType::COMPOUND_PRODUCT, "COMPOUND_PRODUCT"},       // *=
    {TokenType::COMPOUND_DIVISION, "COMPOUND_DIVISION"},     // /=
    {TokenType::COMPOUND_REMAINDER, "COMPOUND_REMAINDER"},   // %=
    {TokenType::COMPOUND_AND, "COMPOUND_AND"},               // &=
    {TokenType::COMPOUND_XOR, "COMPOUND_XOR"},               // ^=
    {TokenType::COMPOUND_OR, "COMPOUND_OR"},                 // |=
    {TokenType::EQUAL, "EQUAL"},                             // ==
    {TokenType::NOTEQUAL, "NOTEQUAL"},                       // !=
    {TokenType::LESSTHANEQUAL, "LESSTHANEQUAL"},             // <=
    {TokenType::GREATERTHANEQUAL, "GREATERTHANEQUAL"},       // >=
    {TokenType::INCREMENT_OPERATOR, "INCREMENT_OPERATOR"},   // ++
    {TokenType::DECREMENT_OPERATOR, "DECREMENT_OPERATOR"},   // --
    {TokenType::ARROW_OPERATOR, "ARROW_OPERATOR"},           // ->
    {TokenType::LEFT_SHIFT, "LEFT_SHIFT"},                   // <<
    {TokenType::RIGHT_SHIFT, "RIGHT_SHIFT"},                 // >>
    {TokenType::LAND, "LAND"},                               // &&
    {TokenType::LOR, "LOR"},                                 // ||
    {TokenType::VOID, "VOID"},                               // void
    {TokenType::RETURN, "RETURN"},                           // return
    {TokenType::IF, "IF"},                                   // if
    {TokenType::ELSE, "ELSE"},                               // else
    {TokenType::DO, "DO"},                                   // do
    {TokenType::WHILE, "WHILE"},                             // while
    {TokenType::FOR, "FOR"},                                 // for
    {TokenType::BREAK, "BREAK"},                             // break
    {TokenType::CONTINUE, "CONTINUE"},                       // continue
    {TokenType::STATIC, "STATIC"},                           // static
    {TokenType::EXTERN, "EXTERN"},                           // extern
    {TokenType::INT, "INT"},                                 // int
    {TokenType::LONG, "LONG"},                               // long
    {TokenType::SIGNED, "SIGNED"},                           // signed
    {TokenType::UNSIGNED, "UNSIGNED"},                       // unsigned
    {TokenType::DOUBLE, "DOUBLE"},                           // double
    {TokenType::CHAR, "CHAR"},                               // char
    {TokenType::SIZEOF, "SIZEOF"},                           // sizeof
    {TokenType::STRUCT, "STRUCT"},                           // struct
    {TokenType::CLASS, "CLASS"},                             // class
    {TokenType::PUBLIC, "PUBLIC"},                           // public
    {TokenType::PRIVATE, "PRIVATE"},                         // private
    {TokenType::GOTO, "GOTO"},                               // goto
    {TokenType::SWITCH, "SWITCH"},                           // switch
    {TokenType::CASE, "CASE"},                               // case
    {TokenType::DEFAULT_CASE, "DEFAULT_CASE"},               // default
    {TokenType::PRINTF, "PRINTF"},                           // printf
    {TokenType::SCANF, "SCANF"},                             // scanf
    {TokenType::TYPEDEF, "TYPEDEF"},                         // typedef
    {TokenType::ENUM, "ENUM"},                               // enum
    {TokenType::UNION, "UNION"},                             // union
    {TokenType::FLOAT_CONSTANT, "FLOAT_CONSTANT"},           // float constant
    {TokenType::ULONG_CONSTANT, "ULONG_CONSTANT"},     // unsigned long constant
    {TokenType::LONG_CONSTANT, "LONG_CONSTANT"},       // long constant
    {TokenType::UINT_CONSTANT, "UINT_CONSTANT"},       // unsigned int constant
    {TokenType::INT_CONSTANT, "INT_CONSTANT"},         // int constant
    {TokenType::CHARACTER, "CHARACTER"},               // character constant
    {TokenType::STRING, "STRING"},                     // string constant
    {TokenType::IDENTIFIER, "IDENTIFIER"},             // identifier
    {TokenType::OPEN_PARENTHESES, "OPEN_PARENTHESES"}, // (
    {TokenType::CLOSE_PARENTHESES, "CLOSE_PARENTHESES"}, // )
    {TokenType::OPEN_BRACE, "OPEN_BRACE"},               // {
    {TokenType::CLOSE_BRACE, "CLOSE_BRACE"},             // }
    {TokenType::OPEN_BRACKET, "OPEN_BRACKET"},           // [
    {TokenType::CLOSE_BRACKET, "CLOSE_BRACKET"},         // ]
    {TokenType::SEMICOLON, "SEMICOLON"},                 // ;
    {TokenType::COLON, "COLON"},                         // :
    {TokenType::COMMA, "COMMA"},                         // ,
    {TokenType::DOT, "DOT"},                             // .
    {TokenType::QUESTION_MARK, "QUESTION_MARK"},         // ?
    {TokenType::ASSIGNMENT, "ASSIGNMENT"},               // =
    {TokenType::PLUS, "PLUS"},                           // +
    {TokenType::HYPHEN, "HYPHEN"},                       // -
    {TokenType::ASTERISK, "ASTERISK"},                   // *
    {TokenType::FORWARD_SLASH, "FORWARD_SLASH"},         // /
    {TokenType::PERCENT_SIGN, "PERCENT_SIGN"},           // %
    {TokenType::TILDE, "TILDE"},                         // ~
    {TokenType::NOT, "NOT"},                             // !
    {TokenType::LESSTHAN, "LESSTHAN"},                   // <
    {TokenType::GREATERTHAN, "GREATERTHAN"},             // >
    {TokenType::AAND, "AAND"},                           // &&
    {TokenType::AOR, "AOR"},                             // ||
    {TokenType::XOR, "XOR"},                             // ^
};

const std::unordered_map<std::string, TokenType> keywords = {
    {"void", TokenType::VOID},
    {"return", TokenType::RETURN},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"do", TokenType::DO},
    {"while", TokenType::WHILE},
    {"for", TokenType::FOR},
    {"break", TokenType::BREAK},
    {"continue", TokenType::CONTINUE},
    {"static", TokenType::STATIC},
    {"extern", TokenType::EXTERN},
    {"int", TokenType::INT},
    {"long", TokenType::LONG},
    {"signed", TokenType::SIGNED},
    {"unsigned", TokenType::UNSIGNED},
    {"double", TokenType::DOUBLE},
    {"char", TokenType::CHAR},
    {"sizeof", TokenType::SIZEOF},
    {"struct", TokenType::STRUCT},
    {"class", TokenType::CLASS},
    {"public", TokenType::PUBLIC},
    {"private", TokenType::PRIVATE},
    {"goto", TokenType::GOTO},
    {"switch", TokenType::SWITCH},
    {"case", TokenType::CASE},
    {"default", TokenType::DEFAULT_CASE},
    {"printf", TokenType::PRINTF},
    {"scanf", TokenType::SCANF},
    {"typedef", TokenType::TYPEDEF},
    {"enum", TokenType::ENUM},
    {"union", TokenType::UNION}};