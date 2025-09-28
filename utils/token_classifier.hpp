#pragma once
#include "../ast/ast.hpp"

constexpr TypeKind TokenTypeToTypeKind(TokenType t) {
  switch(t) {
    case TokenType::INT:
      return TypeKind::INT;
    case TokenType::LONG:
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
  switch(k) {
    case TypeKind::INT:   
      return TokenType::INT;
    case TypeKind::LONG:
      return TokenType::LONG; 
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

