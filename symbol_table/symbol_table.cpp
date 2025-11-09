#include "symbol_table.hpp"

extern bool operator==(const StorageClass &a, const LinkageType &b) {
  if (b == LinkageType::INTERNAL && a == StorageClass::STATIC) {
    return true;
  }
  if (b == LinkageType::EXTERNAL && a == StorageClass::EXTERN) {
    return true;
  }
  return false;
}