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
  VA_START,
  VA_END,
  VA_ARG,
  VA_LIST,
  FOPEN,
  FCLOSE,
  FREAD,
  FWRITE,
  FPRINTF,
  FSCANF,
  MALLOC,
  FREE,
  UNTIL,
  CONST,
  END_OF_FILE,
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

std::string TokenTypeToString(TokenType type);

/*
 extern to follow ODR otherwise multiple definitions are created
 (not a problem for blueprints like classes or enum definitions but can be a
 problem for declarations which occupy memory)
*/
extern const std::unordered_map<TokenType, std::string> TokenTypeNames;

extern const std::unordered_map<std::string, TokenType> keywords;