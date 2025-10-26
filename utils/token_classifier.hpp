#pragma once
#include "../ast/ast.hpp"

constexpr TypeKind TokenTypeToTypeKind(TokenType t) {
  switch (t) {
  case TokenType::INT_CONSTANT:
    return TypeKind::INT;
  case TokenType::LONG_CONSTANT:
    return TypeKind::LONG;
  case TokenType::UINT_CONSTANT:
    return TypeKind::UINT;
  case TokenType::ULONG_CONSTANT:
    return TypeKind::ULONG;
  case TokenType::FLOAT_CONSTANT:
    return TypeKind::DOUBLE;
  default:
    return TypeKind::ERROR; // default to FUNC for unsupported types
  }
}

constexpr TokenType TypeKindToTokenType(TypeKind k) {
  switch (k) {
  case TypeKind::INT:
    return TokenType::INT_CONSTANT;
  case TypeKind::LONG:
    return TokenType::LONG_CONSTANT;
  case TypeKind::UINT:
    return TokenType::UINT_CONSTANT;
  case TypeKind::ULONG:
    return TokenType::ULONG_CONSTANT;
  case TypeKind::DOUBLE:
    return TokenType::FLOAT_CONSTANT;
  default:
    return TokenType::WS; // using ws as error token
  }
}

constexpr std::string TypeKindToString(TypeKind k) {
  switch (k) {
  case TypeKind::CHAR:
    return "char";
  case TypeKind::SCHAR:
    return "signed char";
  case TypeKind::UCHAR:
    return "unsigned char";
  case TypeKind::INT:
    return "int";
  case TypeKind::LONG:
    return "long";
  case TypeKind::UINT:
    return "unsigned int";
  case TypeKind::ULONG:
    return "unsigned long";
  case TypeKind::DOUBLE:
    return "double";
  case TypeKind::FUNC:
    return "function";
  case TypeKind::POINTER:
    return "pointer";
  case TypeKind::ARRAY:
    return "array";
  case TypeKind::STRUCT:
    return "struct";
  case TypeKind::VOID:
    return "void";
  default:
    return "error"; // using ws as error token
  }
}
