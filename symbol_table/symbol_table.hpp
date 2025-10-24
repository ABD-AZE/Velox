#pragma once
#include "../ast/ast.hpp"
#include <string>
#include <unordered_map>

enum class InitType {
  TENTATIVE,
  ZERO_INITIALIZED,
  INITIALIZED,
  UNINITIALIZED
};

enum class LinkageType {
  INTERNAL,
  EXTERNAL,
  NONE
};

enum class SymbolType {
  VARIABLE,
  FUNCTION
};

constexpr bool operator==(const StorageClass &a,const LinkageType& b){
  if(b == LinkageType::INTERNAL && a == StorageClass::STATIC){
    return true;
  }
  if(b == LinkageType::EXTERNAL && a == StorageClass::EXTERN){
    return true;
  }
  return false;
}


class SymbolTableEntry{
public:
  std::string name;
  LinkageType linkage;
  SymbolType symbolType;
  StorageClass storageClass;
  InitType initType;
  std::vector<Type> param_types; // for functions
  std::variant<int, long int, long unsigned int, unsigned int, double> value; // for initialized constants
  Type type;
  SymbolTableEntry() = default;
  SymbolTableEntry(std::string name,SymbolType symbolType, InitType initType, Type type, std::vector<Type> param_types = {}): name(name),symbolType(symbolType), initType(initType),type(type), param_types(param_types) {}
  void setValue(const std::variant<int, long int, long unsigned int, unsigned int, double>& val){
    std::visit([this](auto&& extracted_val) {
      switch (type.kind){
        case TypeKind::INT:
          value = static_cast<int>(extracted_val);
          break;
        case TypeKind::LONG:
          value = static_cast<long int>(extracted_val);
          break;
        case TypeKind::UINT:
          value = static_cast<unsigned int>(extracted_val);
          break;
        case TypeKind::ULONG:
          value = static_cast<long unsigned int>(extracted_val);
          break;
        case TypeKind::DOUBLE:
          value = static_cast<double>(extracted_val);
          break;
        default:
          // handle error
          break;
      }
    }, val);
  }
};

extern std::unordered_map<std::string, SymbolTableEntry> global_symbol_table;