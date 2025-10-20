#pragma once
#include "../ast/ast.hpp"
#include <string>
#include <unordered_map>


class SymbolTableEntry{
  public:
  std::string name;
  bool isFunction;
  bool isExternal;
  bool isDefined;
  Type type;
  SymbolTableEntry() = default;
  SymbolTableEntry(std::string name, bool isFunction = false, bool isExternal = false, bool isDefined = false, Type type = Type::Error())
      : name(name), isFunction(isFunction), isExternal(isExternal), isDefined(isDefined), type(std::move(type)) {}
};

extern std::unordered_map<std::string, SymbolTableEntry> global_symbol_table;