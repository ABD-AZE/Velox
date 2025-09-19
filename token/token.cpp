#include "token.hpp"
#include <stdio.h>
std::string TokenTypeToString(TokenType type) {

  if (auto it = TokenTypeNames.find(type); it != TokenTypeNames.end()) {

    return it->second;
  }

  return "UNKNOWN_TOKEN";
}

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
    {TokenType::VA_START, "VA_START"},                   // va_start
    {TokenType::VA_END, "VA_END"},                       // va_end
    {TokenType::VA_ARG, "VA_ARG"},                       // va_arg
    {TokenType::VA_LIST, "VA_LIST"},                     // va_list
    {TokenType::FOPEN, "FOPEN"},                         // fopen
    {TokenType::FCLOSE, "FCLOSE"},                       // fclose
    {TokenType::FREAD, "FREAD"},                         // fread
    {TokenType::FWRITE, "FWRITE"},                       // fwrite
    {TokenType::FPRINTF, "FPRINTF"},                     // fprintf
    {TokenType::FSCANF, "FSCANF"},                       // fscanf
    {TokenType::MALLOC, "MALLOC"},                       // malloc
    {TokenType::FREE, "FREE"},                           // free
    {TokenType::UNTIL, "UNTIL"},                         // until
    {TokenType::CONST, "CONST"},                         // const
    {TokenType::END_OF_FILE, "END_OF_FILE"}              // end of file
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
    {"union", TokenType::UNION},
    {"va_start", TokenType::VA_START},
    {"va_end", TokenType::VA_END},
    {"va_arg", TokenType::VA_ARG},
    {"va_list", TokenType::VA_LIST},
    {"fopen", TokenType::FOPEN},
    {"fclose", TokenType::FCLOSE},
    {"fread", TokenType::FREAD},
    {"fwrite", TokenType::FWRITE},
    {"fprintf", TokenType::FPRINTF},
    {"fscanf", TokenType::FSCANF},
    {"malloc", TokenType::MALLOC},
    {"free", TokenType::FREE},
    {"until", TokenType::UNTIL},
    {"const", TokenType::CONST},
    {"end_of_file", TokenType::END_OF_FILE}};
