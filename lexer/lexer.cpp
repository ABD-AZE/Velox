#include "lexer.hpp"
#include "../utils/char_classifier.hpp"
#include "../utils/termcolor.hpp"
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

static const std::unordered_map<std::string, TokenType> keywords = {
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
    {"goto", TokenType::GOTO},
    {"switch", TokenType::SWITCH},
    {"case", TokenType::CASE},
    {"default", TokenType::DEFAULT_CASE},
    {"printf", TokenType::PRINTF},
    {"scanf", TokenType::SCANF},
    {"typedef", TokenType::TYPEDEF},
    {"enum", TokenType::ENUM},
    {"union", TokenType::UNION}};

static std::string TokenTypeToString(TokenType type) {
  if (auto it = TokenTypeNames.find(type); it != TokenTypeNames.end()) {
    return it->second;
  }
  return "UNKNOWN_TOKEN";
}

Lexer::Lexer(const std::string &file) : inputFileStream(file) {
  try {
    if (!inputFileStream.is_open()) {
      throw std::runtime_error("Failed to open file: " + file);
    }
  } catch (const std::ios_base::failure &e) {
    throw std::runtime_error("Error opening file: " + file + " - " + e.what());
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

  // Helpers
  auto digits = [](int x) -> int {
    if (x <= 0)
      return 1;
    int d = 0;
    while (x) {
      x /= 10;
      ++d;
    }
    return d;
  };
  auto quote_and_escape = [](const std::string &s) -> std::string {
    std::string out;
    out.reserve(s.size() + 2);
    out.push_back('\'');
    for (unsigned char c : s) {
      switch (c) {
      case '\n':
        out += "\\n";
        break;
      case '\t':
        out += "\\t";
        break;
      case '\r':
        out += "\\r";
        break;
      case '\f':
        out += "\\f";
        break;
      case '\v':
        out += "\\v";
        break;
      case '\\':
        out += "\\\\";
        break;
      case '\'':
        out += "\\\'";
        break;
      default:
        if (c < 0x20 || c == 0x7F) {
          char buf[5];
          std::snprintf(buf, sizeof(buf), "\\x%02X", c);
          out += buf;
        } else {
          out.push_back(static_cast<char>(c));
        }
      }
    }
    out.push_back('\'');
    return out;
  };

  // Computing dynamic widths
  int maxLine = 1, maxCol = 1;
  size_t nameW = std::string("Name").size();
  size_t lexW = std::string("Lexeme").size();

  std::vector<std::string> names;
  names.reserve(tokens.size());
  std::vector<std::string> qlexs;
  qlexs.reserve(tokens.size());

  for (const auto &tok : tokens) {
    maxLine = std::max(maxLine, tok.GetLineNumber());
    maxCol = std::max(maxCol, tok.GetColumnNumber());
    names.emplace_back(TokenTypeToString(tok.GetType()));
    qlexs.emplace_back(quote_and_escape(tok.GetLexeme()));
    nameW = std::max(nameW, names.back().size());
    lexW = std::max(lexW, qlexs.back().size());
  }

  const size_t lineW = std::max<size_t>(4, digits(maxLine));
  const size_t colW = std::max<size_t>(3, digits(maxCol));
  const size_t nameMin = 16;
  const size_t lexMin = 9;
  const size_t nameColW = std::max(nameW, nameMin);
  const size_t lexColW = std::max(lexW, lexMin);

  // Header
  std::cout << bold
            << std::format("{:>{}}  {:>{}}  {:<{}}  {:<{}}\n", "Line", lineW,
                           "Col", colW, "Token", nameColW, "Lexeme", lexColW)
            << reset;

  // Separator
  std::cout << std::format("{:->{}}  {:->{}}  {:->{}}  {:->{}}\n", "", lineW,
                           "", colW, "", nameColW, "", lexColW);

  // Rows
  for (size_t i = 0; i < tokens.size(); ++i) {
    const auto &tok = tokens[i];
    std::cout
        // line, col in grey
        << grey
        << std::format("{:>{}}  {:>{}}  ", tok.GetLineNumber(), lineW,
                       tok.GetColumnNumber(), colW)
        << reset
        // token in blue
        << blue << std::format("{:<{}}  ", names[i], nameColW)
        << reset
        // lexeme in cyan
        << cyan << std::format("{:<{}}", qlexs[i], lexColW) << reset << '\n';
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
                          TokenType lastAcceptedTokenType,
                          int lastAcceptedColumnNumber,
                          int lastAcceptedLineNumber) {
  if (lastAcceptedTokenPos == std::streampos(0)) {
    errors.emplace_back(token.GetLineNumber(), token.GetColumnNumber(),
                        token.GetLexeme());
    this->success = 0;
  } else {
    while (inputFileStream.tellg() != lastAcceptedTokenPos) {
      token.pop();
      inputFileStream.unget();
    }
    currentColumnNumber = lastAcceptedColumnNumber;
    currentLineNumber = lastAcceptedLineNumber;
    token.SetType(lastAcceptedTokenType);
    tokens.push_back(token);
  }
}

/*
NOTE for self: peek() doesn't move fp, get() does
*/

const std::vector<Token> &Lexer::GenerateTokens() {
  // main logic goes here
  currentLineNumber = 1;   // 1 based indexing
  currentColumnNumber = 0; // 1 based indexing
  char c;
  while (true) {
    c = inputFileStream.get();
    if (c == EOF) {
      break;
    } else if (c == '\n') {
      currentLineNumber++;
      currentColumnNumber = 0;
      continue;
    } else if (c == ' ') {
      currentColumnNumber++;
      continue;
    }

    Token token;
    std::streampos lastAcceptedTokenPos =
        std::streampos(0); // default invalid position
    TokenType lastAcceptedTokenType;
    int lastAcceptedColumnNumber = 0;
    int lastAcceptedLineNumber = 0;
    //-----------------------------------COMMENTS_START------------------------------------

    // potential tokens: LINE_COMMENT, BLOCK_COMMENT, FORWARD_SLASH,
    // COMPOUND_DIVISION
    if (c == '/') {
      token.push(c);
      currentColumnNumber++;
      token.SetColumnNumber(currentColumnNumber);
      token.SetLineNumber(currentLineNumber);
      lastAcceptedColumnNumber = currentColumnNumber;
      lastAcceptedLineNumber = currentLineNumber;
      lastAcceptedTokenPos = inputFileStream.tellg();
      lastAcceptedTokenType = TokenType::FORWARD_SLASH;
      c = inputFileStream.get();
      if (c == '/') {
        // LINE_COMMENT
        token.push('/');
        currentColumnNumber++;
        token.SetColumnNumber(currentColumnNumber);
        token.SetLineNumber(currentLineNumber);
        while (c != '\n') {
          token.push(c);
          c = inputFileStream.get();
          if (c == EOF) {
            token.push(EOF);
            break;
          }
          if (c == '\n') {
            token.push(c);
            currentLineNumber++;
            currentColumnNumber = 0;
            break;
          }
          currentColumnNumber++;
        }
        if (token.GetLexeme().back() == EOF) {
          break;
        } else if (token.GetLexeme().back() == '\n') {
          continue;
        }
        // careful logical review required
        inputFileStream.unget();
        continue;
      } else if (c == '*') {
        // BLOCK_COMMENT
        token.push('*');
        currentColumnNumber++;
        token.SetColumnNumber(currentColumnNumber);
        token.SetLineNumber(currentLineNumber);
        while (c != EOF) {
          token.push(c);
          c = inputFileStream.get();
          if (c == '*' && inputFileStream.peek() == '/') {
            token.push(c);
            token.push('/');
            currentColumnNumber += 2;
            break;
          } else if (c == '\n') {
            currentLineNumber++;
            currentColumnNumber = 0;
          } else {
            currentColumnNumber++;
          }
        }
        if (c == EOF) {
          break;
        }
        continue;
      } else if (c == '=') {
        token.push('=');
        currentColumnNumber++;
        token.SetType(TokenType::COMPOUND_DIVISION);
        tokens.push_back(token);
      } else {
        if (c == std::char_traits<char>::eof()) {
          inputFileStream.clear();
          inputFileStream.seekg(0, std::ios::end);
        } else {
          inputFileStream.unget();
        }
        ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                      lastAcceptedColumnNumber, lastAcceptedLineNumber);
      }
      continue;
    }

    //-----------------------------------COMMENTS_END--------------------------------------

    //-----------------------------------OPERATORS_START-----------------------------------

    // potential tokens: FLOAT_CONSTANT, DOT, ELLIPSIS
    else if (c == '.') {
      token.push(c);
      currentColumnNumber++;
      token.SetColumnNumber(currentColumnNumber);
      token.SetLineNumber(currentLineNumber);
      lastAcceptedTokenPos = inputFileStream.tellg();
      lastAcceptedTokenType = TokenType::DOT;
      lastAcceptedColumnNumber = currentColumnNumber;
      lastAcceptedLineNumber = currentLineNumber;
      c = inputFileStream.get();
      if (c == '.') {
        token.push(c);
        currentColumnNumber++;
        c = inputFileStream.get();
        if (c == '.') {
          // ellipsis found
          token.push(c);
          currentColumnNumber++;
          token.SetType(TokenType::ELLIPSIS);
          tokens.push_back(token);
          continue;
        } else {
          // Careful review required
          if (c == std::char_traits<char>::eof()) {
            inputFileStream.clear();
            inputFileStream.seekg(0, std::ios::end);
          } else {
            inputFileStream.unget();
          }
          ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                        lastAcceptedColumnNumber, lastAcceptedLineNumber);
          continue;
        }
      } else if (isDigit(c)) {
        // potential FLOAT_CONSTANT
        token.SetType(TokenType::FLOAT_CONSTANT);
        int flag = 0;
        while (isDigit(c)) {
          token.push(c);
          currentColumnNumber++;
          c = inputFileStream.get();
          lastAcceptedTokenPos = inputFileStream.tellg();
          lastAcceptedTokenType = TokenType::FLOAT_CONSTANT;
          lastAcceptedColumnNumber = currentColumnNumber;
          lastAcceptedLineNumber = currentLineNumber;
        }
        if (c != std::char_traits<char>::eof()) {
          inputFileStream.unget();
        } else {
          inputFileStream.clear();
          inputFileStream.seekg(0, std::ios::end);
        }
        lastAcceptedTokenPos = inputFileStream.tellg();
        ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                      lastAcceptedColumnNumber, lastAcceptedLineNumber);
      } else {
        // dot
        if (c == std::char_traits<char>::eof()) {
          inputFileStream.clear();
          inputFileStream.seekg(0, std::ios::end);
        } else {
          inputFileStream.unget();
        }
        ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                      lastAcceptedColumnNumber, lastAcceptedLineNumber);
        continue;
      }
    }

    // potential tokens: PLUS, COMPOUND_SUM, INCREMENT_OPERATOR
    else if (c == '+') {
      token.push(c);
      currentColumnNumber++;
      token.SetColumnNumber(currentColumnNumber);
      token.SetLineNumber(currentLineNumber);
      lastAcceptedTokenPos = inputFileStream.tellg();
      lastAcceptedTokenType = TokenType::PLUS;
      lastAcceptedColumnNumber = currentColumnNumber;
      lastAcceptedLineNumber = currentLineNumber;
      c = inputFileStream.get();
      if (c == '+') {
        token.push(c);
        currentColumnNumber++;
        token.SetType(TokenType::INCREMENT_OPERATOR);
        tokens.push_back(token);
      } else if (c == '=') {
        token.push(c);
        currentColumnNumber++;
        token.SetType(TokenType::COMPOUND_SUM);
        tokens.push_back(token);
      } else {
        if (c == std::char_traits<char>::eof()) {
          inputFileStream.clear();
          inputFileStream.seekg(0, std::ios::end);
        } else {
          inputFileStream.unget();
        }
        ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                      lastAcceptedColumnNumber, lastAcceptedLineNumber);
      }
      continue;
    }
    // potential tokens: HYPHEN, COMPOUND_DIFFERENCE, DECREMENT_OPERATOR,
    // ARROW_OPERATOR
    if (c == '-') {
      token.push(c);
      currentColumnNumber++;
      token.SetColumnNumber(currentColumnNumber);
      token.SetLineNumber(currentLineNumber);
      lastAcceptedTokenPos = inputFileStream.tellg();
      lastAcceptedTokenType = TokenType::DECREMENT_OPERATOR;
      lastAcceptedColumnNumber = currentColumnNumber;
      lastAcceptedLineNumber = currentLineNumber;
      c = inputFileStream.get();
      if (c == '-') {
        token.push(c);
        currentColumnNumber++;
        token.SetType(TokenType::DECREMENT_OPERATOR);
        tokens.push_back(token);
      } else if (c == '=') {
        token.push(c);
        currentColumnNumber++;
        token.SetType(TokenType::COMPOUND_DIFFERENCE);
        tokens.push_back(token);
      } else if (c == '>') {
        token.push(c);
        currentColumnNumber++;
        token.SetType(TokenType::ARROW_OPERATOR);
        tokens.push_back(token);
      } else {
        if (c == std::char_traits<char>::eof()) {
          inputFileStream.clear();
          inputFileStream.seekg(0, std::ios::end);
        } else {
          inputFileStream.unget();
        }
        ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                      lastAcceptedColumnNumber, lastAcceptedLineNumber);
      }
      continue;
    }
    // potential tokens: ASTERISK, COMPOUND_PRODUCT, INCREMENT_OPERATOR
    else if (c == '*') {
      token.push(c);
      currentColumnNumber++;
      token.SetColumnNumber(currentColumnNumber);
      token.SetLineNumber(currentLineNumber);
      lastAcceptedTokenPos = inputFileStream.tellg();
      lastAcceptedTokenType = TokenType::ASTERISK;
      lastAcceptedColumnNumber = currentColumnNumber;
      lastAcceptedLineNumber = currentLineNumber;
      c = inputFileStream.get();
      if (c == '=') {
        token.push(c);
        currentColumnNumber++;
        token.SetType(TokenType::COMPOUND_PRODUCT);
        tokens.push_back(token);
      } else {
        if (c == std::char_traits<char>::eof()) {
          inputFileStream.clear();
          inputFileStream.seekg(0, std::ios::end);
        } else {
          inputFileStream.unget();
        }
        ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                      lastAcceptedColumnNumber, lastAcceptedLineNumber);
      }
      continue;
    }

    // potential tokens: COMPOUND_RIGHTSHIFT, RIGHT_SHIFT, GREATERTHAN,
    // GREATERTHANEQUAL
    else if (c == '>') {
      token.push(c);
      currentColumnNumber++;
      token.SetColumnNumber(currentColumnNumber);
      token.SetLineNumber(currentLineNumber);
      lastAcceptedTokenPos = inputFileStream.tellg();
      lastAcceptedTokenType = TokenType::GREATERTHAN;
      lastAcceptedColumnNumber = currentColumnNumber;
      lastAcceptedLineNumber = currentLineNumber;
      c = inputFileStream.get();
      // rightshift, compountrightshift
      if (c == '>') {
        token.push(c);
        currentColumnNumber++;
        lastAcceptedTokenPos = inputFileStream.tellg();
        lastAcceptedTokenType = TokenType::RIGHT_SHIFT;
        lastAcceptedColumnNumber = currentColumnNumber;
        lastAcceptedLineNumber = currentLineNumber;
        if (inputFileStream.peek() == EOF) {

          ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                        lastAcceptedColumnNumber, lastAcceptedLineNumber);
          continue;
        }
        c = inputFileStream.get();
        if (c == '=') {
          token.push(c);
          currentColumnNumber++;
          token.SetType(TokenType::COMPOUND_RIGHTSHIFT);
          tokens.push_back(token);
        } else {
          if (c == std::char_traits<char>::eof()) {
            inputFileStream.clear();
            inputFileStream.seekg(0, std::ios::end);
          } else {
            inputFileStream.unget();
          }
          ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                        lastAcceptedColumnNumber, lastAcceptedLineNumber);
        }
      } else if (c == '=') {
        token.push(c);
        currentColumnNumber++;
        token.SetType(TokenType::GREATERTHANEQUAL);
        tokens.push_back(token);
      } else {
        if (c == std::char_traits<char>::eof()) {
          inputFileStream.clear();
          inputFileStream.seekg(0, std::ios::end);
        } else {
          inputFileStream.unget();
        }
        ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                      lastAcceptedColumnNumber, lastAcceptedLineNumber);
      }
      continue;
    }

    // potential tokens: COMPOUND_LEFTSHIFT, LEFT_SHIFT, LESSTHAN, LESSTHANEQUAL
    else if (c == '<') {
      token.push(c);
      currentColumnNumber++;
      token.SetColumnNumber(currentColumnNumber);
      token.SetLineNumber(currentLineNumber);
      lastAcceptedTokenPos = inputFileStream.tellg();
      lastAcceptedTokenType = TokenType::LESSTHAN;
      lastAcceptedColumnNumber = currentColumnNumber;
      lastAcceptedLineNumber = currentLineNumber;
      c = inputFileStream.get();
      // leftshift, compoundleftshift
      if (c == '<') {
        token.push(c);
        currentColumnNumber++;
        lastAcceptedTokenPos = inputFileStream.tellg();
        lastAcceptedTokenType = TokenType::LEFT_SHIFT;
        lastAcceptedColumnNumber = currentColumnNumber;
        lastAcceptedLineNumber = currentLineNumber;
        c = inputFileStream.get();
        if (c == '=') {
          token.push(c);
          currentColumnNumber++;
          token.SetType(TokenType::COMPOUND_LEFTSHIFT);
          tokens.push_back(token);
          continue;
        } else {
          if (c == std::char_traits<char>::eof()) {
            inputFileStream.clear();
            inputFileStream.seekg(0, std::ios::end);
          } else {
            inputFileStream.unget();
          }
          ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                        lastAcceptedColumnNumber, lastAcceptedLineNumber);
        }
      } else if (c == '=') {
        token.push(c);
        currentColumnNumber++;
        token.SetType(TokenType::LESSTHANEQUAL);
        tokens.push_back(token);
      } else {
        if (c == std::char_traits<char>::eof()) {
          inputFileStream.clear();
          inputFileStream.seekg(0, std::ios::end);
        } else {
          inputFileStream.unget();
        }
        ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                      lastAcceptedColumnNumber, lastAcceptedLineNumber);
      }
      continue;
    }

    // % , %=
    else if (c == '%') {
      token.push(c);
      currentColumnNumber++;
      lastAcceptedColumnNumber = currentColumnNumber;
      lastAcceptedLineNumber = currentLineNumber;
      lastAcceptedTokenPos = inputFileStream.tellg();
      lastAcceptedTokenType = TokenType::PERCENT_SIGN;
      c = inputFileStream.get();
      if (c == '=') {
        token.push(c);
        currentColumnNumber++;
        token.SetType(TokenType::COMPOUND_REMAINDER);
        tokens.push_back(token);
      } else {
        if (c == std::char_traits<char>::eof()) {
          inputFileStream.clear();
          inputFileStream.seekg(0, std::ios::end);
        } else {
          inputFileStream.unget();
        }
        ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                      lastAcceptedColumnNumber, lastAcceptedLineNumber);
      }
      continue;
    }

    // &, &=, &&
    else if (c == '&') {
      token.push(c);
      currentColumnNumber++;
      lastAcceptedColumnNumber = currentColumnNumber;
      lastAcceptedLineNumber = currentLineNumber;
      lastAcceptedTokenPos = inputFileStream.tellg();
      lastAcceptedTokenType = TokenType::AAND;
      c = inputFileStream.get();
      if (c == '&') {
        token.push(c);
        currentColumnNumber++;
        token.SetType(TokenType::LAND);
        tokens.push_back(token);
      } else if (c == '=') {
        token.push(c);
        currentColumnNumber++;
        token.SetType(TokenType::COMPOUND_AND);
        tokens.push_back(token);
      } else {
        if (c == std::char_traits<char>::eof()) {
          inputFileStream.clear();
          inputFileStream.seekg(0, std::ios::end);
        } else {
          inputFileStream.unget();
        }
        ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                      lastAcceptedColumnNumber, lastAcceptedLineNumber);
      }
      continue;
    }

    // ^, ^=
    else if (c == '^') {
      token.push(c);
      currentColumnNumber++;
      lastAcceptedColumnNumber = currentColumnNumber;
      lastAcceptedLineNumber = currentLineNumber;
      lastAcceptedTokenPos = inputFileStream.tellg();
      lastAcceptedTokenType = TokenType::XOR;
      c = inputFileStream.get();
      if (c == '=') {
        token.push(c);
        currentColumnNumber++;
        token.SetType(TokenType::COMPOUND_XOR);
        tokens.push_back(token);
      } else {
        if (c == std::char_traits<char>::eof()) {
          inputFileStream.clear();
          inputFileStream.seekg(0, std::ios::end);
        } else {
          inputFileStream.unget();
        }
        ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                      lastAcceptedColumnNumber, lastAcceptedLineNumber);
      }
      continue;
    }
    // | , || , |=
    else if (c == '|') {
      token.push(c);
      currentColumnNumber++;
      lastAcceptedColumnNumber = currentColumnNumber;
      lastAcceptedLineNumber = currentLineNumber;
      lastAcceptedTokenPos = inputFileStream.tellg();
      lastAcceptedTokenType = TokenType::AOR;
      c = inputFileStream.get();
      if (c == '|') {
        token.push(c);
        currentColumnNumber++;
        token.SetType(TokenType::LOR);
        tokens.push_back(token);
      } else if (c == '=') {
        token.push(c);
        currentColumnNumber++;
        token.SetType(TokenType::COMPOUND_OR);
        tokens.push_back(token);
      } else {
        if (c == std::char_traits<char>::eof()) {
          inputFileStream.clear();
          inputFileStream.seekg(0, std::ios::end);
        } else {
          inputFileStream.unget();
        }
        ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                      lastAcceptedColumnNumber, lastAcceptedLineNumber);
      }
      continue;
    }
    // =, ==
    else if (c == '=') {
      token.push(c);
      currentColumnNumber++;
      lastAcceptedColumnNumber = currentColumnNumber;
      lastAcceptedLineNumber = currentLineNumber;
      lastAcceptedTokenPos = inputFileStream.tellg();
      lastAcceptedTokenType = TokenType::ASSIGNMENT;
      c = inputFileStream.get();
      if (c == '=') {
        token.push(c);
        currentColumnNumber++;
        token.SetType(TokenType::EQUAL);
        tokens.push_back(token);
      } else {
        if (c == std::char_traits<char>::eof()) {
          inputFileStream.clear();
          inputFileStream.seekg(0, std::ios::end);
        } else {
          inputFileStream.unget();
        }
        ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                      lastAcceptedColumnNumber, lastAcceptedLineNumber);
      }
      continue;
    }

    // ! , !=
    else if (c == '!') {
      token.push(c);
      currentColumnNumber++;
      lastAcceptedColumnNumber = currentColumnNumber;
      lastAcceptedLineNumber = currentLineNumber;
      lastAcceptedTokenPos = inputFileStream.tellg();
      lastAcceptedTokenType = TokenType::NOT;
      c = inputFileStream.get();
      if (c == '=') {
        token.push(c);
        currentColumnNumber++;
        token.SetType(TokenType::NOTEQUAL);
        tokens.push_back(token);
      } else {
        if (c == std::char_traits<char>::eof()) {
          inputFileStream.clear();
          inputFileStream.seekg(0, std::ios::end);
        } else {
          inputFileStream.unget();
        }
        ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                      lastAcceptedColumnNumber, lastAcceptedLineNumber);
      }
      continue;
    }

    // ~
    else if (c == '~') {
      token.push(c);
      token.SetColumnNumber(currentColumnNumber);
      token.SetLineNumber(currentLineNumber);
      token.SetType(TokenType::TILDE);
      tokens.push_back(token);
      continue;
    }

    // ?
    else if (c == '?') {
      token.push(c);
      currentColumnNumber++;
      token.SetColumnNumber(currentColumnNumber);
      token.SetLineNumber(currentLineNumber);
      token.SetType(TokenType::QUESTION_MARK);
      tokens.push_back(token);
      continue;
    }

    // (
    else if (c == '(') {
      token.push(c);
      token.SetColumnNumber(currentColumnNumber);
      token.SetLineNumber(currentLineNumber);
      token.SetType(TokenType::OPEN_PARENTHESES);
      tokens.push_back(token);
      continue;
    }
    // )
    else if (c == ')') {
      token.push(c);
      token.SetColumnNumber(currentColumnNumber);
      token.SetLineNumber(currentLineNumber);
      token.SetType(TokenType::CLOSE_PARENTHESES);
      tokens.push_back(token);
      continue;
    }
    // {
    else if (c == '{') {
      token.push(c);
      token.SetColumnNumber(currentColumnNumber);
      token.SetLineNumber(currentLineNumber);
      token.SetType(TokenType::OPEN_BRACE);
      tokens.push_back(token);
      continue;
    }
    // }
    else if (c == '}') {
      token.push(c);
      token.SetColumnNumber(currentColumnNumber);
      token.SetLineNumber(currentLineNumber);
      token.SetType(TokenType::CLOSE_BRACE);
      tokens.push_back(token);
      continue;
    }
    // [
    else if (c == '[') {
      token.push(c);
      token.SetColumnNumber(currentColumnNumber);
      token.SetLineNumber(currentLineNumber);
      token.SetType(TokenType::OPEN_BRACKET);
      tokens.push_back(token);
      continue;
    }
    // ]
    else if (c == ']') {
      token.push(c);
      token.SetColumnNumber(currentColumnNumber);
      token.SetLineNumber(currentLineNumber);
      token.SetType(TokenType::CLOSE_BRACKET);
      tokens.push_back(token);
      continue;
    }
    // ,
    else if (c == ',') {
      token.push(c);
      token.SetColumnNumber(currentColumnNumber);
      token.SetLineNumber(currentLineNumber);
      token.SetType(TokenType::COMMA);
      tokens.push_back(token);
      continue;
    }
    // ;
    else if (c == ';') {
      token.push(c);
      token.SetColumnNumber(currentColumnNumber);
      token.SetLineNumber(currentLineNumber);
      token.SetType(TokenType::SEMICOLON);
      tokens.push_back(token);
      continue;
    }
    //-----------------------------------OPERATORS_END-------------------------------------

    //-----------------------------------PRADY---------------------------------------------
    //--------------------------------IDENTIFIERS_AND_TOKENS-------------------------------
    else if (isAlpha(c)) {
      token.push(c);
      currentColumnNumber++;
      token.SetColumnNumber(currentColumnNumber);
      token.SetLineNumber(currentLineNumber);
      lastAcceptedTokenPos = inputFileStream.tellg();
      lastAcceptedTokenType = TokenType::IDENTIFIER;
      lastAcceptedColumnNumber = currentColumnNumber;
      lastAcceptedLineNumber = currentLineNumber;
      while (true) {
        c = inputFileStream.get();
        currentColumnNumber++;
        if (!isAlphanumeric(c)) {
          if (c == std::char_traits<char>::eof()) {
            inputFileStream.clear();
            inputFileStream.seekg(0, std::ios::end);
          } else {
            inputFileStream.unget();
          }
          currentColumnNumber--;
          break;
        }
        lastAcceptedColumnNumber = currentColumnNumber;
        lastAcceptedLineNumber = currentLineNumber;
        lastAcceptedTokenPos = inputFileStream.tellg();
        token.push(c);
      }

      if (keywords.find(token.GetLexeme()) != keywords.end()) {
        token.SetType(keywords.at(token.GetLexeme()));
      } else {
        token.SetType(TokenType::IDENTIFIER);
      }

      tokens.push_back(token);
      continue;
    }

    //--------------------------------IDENTIFIERS_AND_TOKENS_END---------------------------
    //-------------------------------STRINGS_AND_CHARACTERS_START--------------------------

    else if(c == '"') {
      // STRING
      // token.push(c);
      currentColumnNumber++;
      token.SetColumnNumber(currentColumnNumber);
      token.SetLineNumber(currentLineNumber);
      bool good = true;
      //we don't accept a single quote as a string
      while(true) {
        c = inputFileStream.get();
        if (c == EOF) {
          inputFileStream.clear();
          inputFileStream.seekg(0, std::ios::end);
          good = false;
          break;
        }
        else if( c == '"') {
          // token.push(c);
          token.pop_front();
          currentColumnNumber++;
          token.SetType(TokenType::STRING);
          tokens.push_back(token);
          break;
        }
        else if(c == '\n') {
          inputFileStream.unget();
          good = false;
          break;
        }
        else if(c == '\\') {
          currentColumnNumber++;
          c = inputFileStream.get();
          if (c == EOF) {
            inputFileStream.clear();
            inputFileStream.seekg(0, std::ios::end);
            good = false;
            break;
          }
          else if(c == '\n') {
            currentColumnNumber = 0;
            currentLineNumber++;
          }
          else if(c == '"') {
            token.push('"');
            currentColumnNumber++;
          }
          else if(c == '\'') {
            token.push('\'');
            currentColumnNumber++;
          }
          else if(c == '\\') {
            token.push('\\');
            currentColumnNumber++;
          }
          else if(c == 'n') {
            token.push('\n');
            currentColumnNumber++;
          }
          else if(c == 't') {
            token.push('\t');
            currentColumnNumber++;
          }
          else if(c == 'r') {
            token.push('\r');
            currentColumnNumber++;
          }
          else if(c == 'f') {
            token.push('\f');
            currentColumnNumber++;
          }
          else if(c == 'v') {
            token.push('\v');
            currentColumnNumber++;
          }
          else if(c == '0') {
            token.push('\0');
            currentColumnNumber++;
          }
          else {
            inputFileStream.unget();
            good = false;
            break;
          }
        }
        else{
          token.push(c);
          currentColumnNumber++;
        }
      }
      if (!good) {
        ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                      lastAcceptedColumnNumber, lastAcceptedLineNumber);
      }
    }
    else if(c == '\'') {
      // CHARACTER
      token.push(c);
      currentColumnNumber++;
      token.SetColumnNumber(currentColumnNumber);
      token.SetLineNumber(currentLineNumber);
    LABEL:
      c = inputFileStream.get();
      bool good = true;
      if (c == EOF) {
        inputFileStream.clear();
        inputFileStream.seekg(0, std::ios::end);
        ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                      lastAcceptedColumnNumber, lastAcceptedLineNumber);
        continue;
      }
      else if(c == '\n' || c == '\'') {
        inputFileStream.unget();
        ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                      lastAcceptedColumnNumber, lastAcceptedLineNumber);
        continue;
      }
      else if(c == '\\') {
        currentColumnNumber++;
        c = inputFileStream.get();
        if(c == '\n') {
          currentColumnNumber = 0;
          currentLineNumber++;
          goto LABEL;
        }
        else if (c == EOF) {
          inputFileStream.clear();
          inputFileStream.seekg(0, std::ios::end);
          ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                        lastAcceptedColumnNumber, lastAcceptedLineNumber);
          continue;
        }
        else if(c == '\'') {
          token.push('\'');
          currentColumnNumber++;
        }
        else if(c == '"') {
          token.push('"');
          currentColumnNumber++;
        }
        else if(c == '\\') {
          token.push('\\');
          currentColumnNumber++;
        }
        else if(c == 'n') {
          token.push('\n');
          currentColumnNumber++;
        }
        else if(c == 't') {
          token.push('\t');
          currentColumnNumber++;
        }
        else if(c == 'r') {
          token.push('\r');
          currentColumnNumber++;
        }
        else if(c == 'f') {
          token.push('\f');
          currentColumnNumber++;
        }
        else if(c == 'v') {
          token.push('\v');
          currentColumnNumber++;
        }
        else if(c == '0') {
          token.push('\0');
          currentColumnNumber++;
        }
        else {
          inputFileStream.unget();
          ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                        lastAcceptedColumnNumber, lastAcceptedLineNumber);
          continue;
        }
      }
      else {
        token.push(c);
        currentColumnNumber++;
      }
      c = inputFileStream.get();
      if(c == EOF) {
        inputFileStream.clear();
        inputFileStream.seekg(0, std::ios::end);
        ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                      lastAcceptedColumnNumber, lastAcceptedLineNumber);
        continue;
      }
      else if(c == '\'') {
        token.push(c);
        currentColumnNumber++;
        token.SetType(TokenType::CHARACTER);
        token.pop();
        token.pop_front();
        tokens.push_back(token);
      }
      else {
        inputFileStream.unget();
        ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                      lastAcceptedColumnNumber, lastAcceptedLineNumber);
      }
    }
    //-------------------------------STRINGS_AND_CHARACTERS_END----------------------------
    //-----------------------------------PRADY_END-----------------------------------------
    else {
      token.push(c);
      currentColumnNumber++;
      token.SetColumnNumber(currentColumnNumber);
      token.SetLineNumber(currentLineNumber);
      ErrorRecovery(token, lastAcceptedTokenPos, lastAcceptedTokenType,
                    lastAcceptedColumnNumber, lastAcceptedLineNumber);
    }
  }
  return tokens;
}

int main() {
  Lexer lexer = Lexer("input");
  std::vector<Token> tokens = lexer.GenerateTokens();
  lexer.PrintTokens();
  lexer.PrintErrors();
  return 0;
}
