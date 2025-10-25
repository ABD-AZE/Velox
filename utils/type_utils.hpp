#pragma once
#include "../ast/ast.hpp"

// only type
constexpr bool isTypeSpecifier(TokenType type) {
  if (type == TokenType::INT || type == TokenType::LONG ||
      type == TokenType::UNSIGNED || type == TokenType::SIGNED ||
      type == TokenType::DOUBLE || type == TokenType::CHAR) {
    return true;
  }
  return false;
}

constexpr bool isStorageSpecifier(TokenType type) {
  if (type == TokenType::STATIC || type == TokenType::EXTERN) {
    return true;
  }
  return false;
}

// type + storage
constexpr bool isSpecifier(TokenType type) {
  if (isStorageSpecifier(type) || isTypeSpecifier(type)) {
    return true;
  }
  return false;
}
