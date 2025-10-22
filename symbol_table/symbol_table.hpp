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

bool operator==(const StorageClass &a,const LinkageType& b){
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
  int value; // for initialized constants
  Type type;
  SymbolTableEntry() = default;
  SymbolTableEntry(std::string name,SymbolType symbolType, InitType initType, Type type): name(name),symbolType(symbolType), initType(initType),type(type){}
};

extern std::unordered_map<std::string, SymbolTableEntry> global_symbol_table;