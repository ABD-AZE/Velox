#pragma once
#include "../ast/ast.hpp"

constexpr TypeKind TokenTypeToTypeKind(TokenType t) {
  switch(t) {
    case TokenType::INT:
      return TypeKind::INT;
    case TokenType::LONG:
      return TypeKind::LONG;
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
    default:
      return TokenType::WS; // using ws as error token
  }
}

