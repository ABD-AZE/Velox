#include "ast.hpp"

extern bool operator==(const TokenType &a, const StorageClass &b) {
  if (b == StorageClass::STATIC && a == TokenType::STATIC) {
    return true;
  }
  if (b == StorageClass::EXTERN && a == TokenType::EXTERN) {
    return true;
  }
  return false;
}

int size(TypeKind &kind) {
  switch (kind) {
  case TypeKind::CHAR:
    return 1;
  case TypeKind::SCHAR:
    return 1;
  case TypeKind::UCHAR:
    return 1;
  case TypeKind::INT:
    return 4;
  case TypeKind::UINT:
    return 4;
  case TypeKind::LONG:
    return 8;
  case TypeKind::ULONG:
    return 8;
  case TypeKind::DOUBLE:
    return 8;
  default:
    return 0;
  }
};