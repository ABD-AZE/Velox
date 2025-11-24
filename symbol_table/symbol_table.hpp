#pragma once
#include "../ast/ast.hpp"
#include <string>
#include <unordered_map>
#include <variant>

enum class InitType { TENTATIVE, ZERO_INITIALIZED, INITIALIZED, UNINITIALIZED };

enum class LinkageType { INTERNAL, EXTERNAL, NONE };

enum class SymbolType { VARIABLE, FUNCTION, CONSTANT };

enum class AssemblyType {
  LONG_WORD,
  QUAD_WORD,
  DOUBLE_WORD,
  BYTE_ARRAY,
  BYTE,
};

extern bool operator==(const StorageClass &a, const LinkageType &b);

class SymbolTableEntry {
public:
  std::string name;
  LinkageType linkage;
  SymbolType symbolType;
  StorageClass storageClass = StorageClass::STATIC; // no use for functions
  InitType initType;
  bool isVariadic = false;
  std::vector<Type> param_types; // for functions
  std::variant<int, long int, long unsigned int, unsigned int, char,
               unsigned char, double>
      value; // for initialized constants (scalars)
  InitializerNode
      *initializer; // non-owning pointer to array initializer in AST
  std::string
      stringConstantName;  // Name of string constant for pointer initializers
  std::string stringValue; // Actual string value for string constants
  Type type;
  AssemblyType assemblyType;

  SymbolTableEntry() = default;
  SymbolTableEntry(std::string name, SymbolType symbolType, InitType initType,
                   Type type, std::vector<Type> param_types = {})
      : name(name), symbolType(symbolType), initType(initType),
        param_types(param_types), type(type) {
    // Determine assembly type based on TypeKind
    switch (type.kind) {
    case TypeKind::CHAR:
    case TypeKind::SCHAR:
    case TypeKind::UCHAR:
      assemblyType = AssemblyType::BYTE;
      break;
    case TypeKind::INT:
    case TypeKind::UINT:
      assemblyType = AssemblyType::LONG_WORD;
      break;
    case TypeKind::LONG:
    case TypeKind::ULONG:
    case TypeKind::POINTER:
      assemblyType = AssemblyType::QUAD_WORD;
      break;
    case TypeKind::DOUBLE:
      assemblyType = AssemblyType::DOUBLE_WORD;
      break;
    case TypeKind::ARRAY:
      assemblyType = AssemblyType::BYTE_ARRAY;
      break;
    default:
      assemblyType = AssemblyType::LONG_WORD; // default
      break;
    }
  }
  void
  setValue(const std::variant<int, long int, long unsigned int, unsigned int,
                              double, char, unsigned char> &val) {
    std::visit(
        [this](auto &&extracted_val) {
          switch (type.kind) {
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
          case TypeKind::CHAR:
            value = static_cast<char>(extracted_val);
            break;
          case TypeKind::UCHAR:
            value = static_cast<unsigned char>(extracted_val);
            break;
          case TypeKind::SCHAR:
            value = static_cast<char>(extracted_val);
            break;
          default:
            // handle error
            break;
          }
        },
        val);
  }
};

extern std::unordered_map<std::string, SymbolTableEntry> global_symbol_table;