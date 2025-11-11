#include "valor.hpp"
#include "../utils/token_classifier.hpp"
#include <functional>
#include <iostream>
#include <sstream>

// IRInstructionNode implementation
std::string IRInstructionNode::toString() const {
  std::stringstream ss;

  switch (opType) {
  case IROpType::RETURN:
    ss << "return " << (src1 ? src1->toString() : "");
    break;
  case IROpType::COPY:
    ss << dst->toString() << " = " << src1->toString();
    break;
  case IROpType::ADD:
    ss << dst->toString() << " = " << src1->toString() << " + "
       << src2->toString();
    break;
  case IROpType::SUBTRACT:
    ss << dst->toString() << " = " << src1->toString() << " - "
       << src2->toString();
    break;
  case IROpType::MULTIPLY:
    ss << dst->toString() << " = " << src1->toString() << " * "
       << src2->toString();
    break;
  case IROpType::DIVIDE:
    ss << dst->toString() << " = " << src1->toString() << " / "
       << src2->toString();
    break;
  case IROpType::REMAINDER:
    ss << dst->toString() << " = " << src1->toString() << " % "
       << src2->toString();
    break;
  case IROpType::AND:
    ss << dst->toString() << " = " << src1->toString() << " & "
       << src2->toString();
    break;
  case IROpType::OR:
    ss << dst->toString() << " = " << src1->toString() << " | "
       << src2->toString();
    break;
  case IROpType::XOR:
    ss << dst->toString() << " = " << src1->toString() << " ^ "
       << src2->toString();
    break;
  case IROpType::LEFT_SHIFT:
    ss << dst->toString() << " = " << src1->toString() << " << "
       << src2->toString();
    break;
  case IROpType::RIGHT_SHIFT:
    ss << dst->toString() << " = " << src1->toString() << " >> "
       << src2->toString();
    break;
  case IROpType::EQUAL:
    ss << dst->toString() << " = " << src1->toString()
       << " == " << src2->toString();
    break;
  case IROpType::NOT_EQUAL:
    ss << dst->toString() << " = " << src1->toString()
       << " != " << src2->toString();
    break;
  case IROpType::LESS_THAN:
    ss << dst->toString() << " = " << src1->toString() << " < "
       << src2->toString();
    break;
  case IROpType::LESS_EQUAL:
    ss << dst->toString() << " = " << src1->toString()
       << " <= " << src2->toString();
    break;
  case IROpType::GREATER_THAN:
    ss << dst->toString() << " = " << src1->toString() << " > "
       << src2->toString();
    break;
  case IROpType::GREATER_EQUAL:
    ss << dst->toString() << " = " << src1->toString() << " >= "
       << src2->toString();
    break;
  case IROpType::LOGICAL_AND:
    ss << dst->toString() << " = " << src1->toString() << " && "
       << src2->toString();
    break;
  case IROpType::LOGICAL_OR:
    ss << dst->toString() << " = " << src1->toString() << " || "
       << src2->toString();
    break;
  case IROpType::NOT:
    ss << dst->toString() << " = !" << src1->toString();
    break;
  case IROpType::NEGATE:
    ss << dst->toString() << " = -" << src1->toString();
    break;
  case IROpType::COMPLEMENT:
    ss << dst->toString() << " = ~" << src1->toString();
    break;
  case IROpType::SIGN_EXTEND:
    ss << dst->toString() << " = sign_extend(" << src1->toString() << ")";
    break;
  case IROpType::TRUNCATE:
    ss << dst->toString() << " = truncate(" << src1->toString() << ")";
    break;
  case IROpType::ZERO_EXTEND:
    ss << dst->toString() << " = zero_extend(" << src1->toString() << ")";
    break;
  case IROpType::DOUBLE_TO_LONG:
    ss << dst->toString() << " = double_to_long(" << src1->toString() << ")";
    break;
  case IROpType::DOUBLE_TO_ULONG:
    ss << dst->toString() << " = double_to_ulong(" << src1->toString() << ")";
    break;
  case IROpType::LONG_TO_DOUBLE:
    ss << dst->toString() << " = long_to_double(" << src1->toString() << ")";
    break;
  case IROpType::ULONG_TO_DOUBLE:
    ss << dst->toString() << " = ulong_to_double(" << src1->toString() << ")";
    break;
  case IROpType::GET_ADDRESS:
    ss << dst->toString() << " = get_address(" << src1->toString() << ")";
    break;
  case IROpType::LOAD:
    ss << dst->toString() << " = load(" << src1->toString() << ")";
    break;
  case IROpType::STORE:
    ss << "store(" << src1->toString() << ", " << dst->toString() << ")";
    break;
  case IROpType::ADD_PTR:
    ss << dst->toString() << " = add_ptr(" << src1->toString() << ", "
       << src2->toString() << ", " << scale << ")";
    break;
  case IROpType::COPY_TO_OFFSET:
    ss << "copy_to_offset(" << src1->toString() << ", " << label << ", "
       << offset << ")";
    break;
  case IROpType::JUMP:
    ss << "jump " << label;
    break;
  case IROpType::JUMP_IF_ZERO:
    ss << "jump_if_zero " << src1->toString() << " " << label;
    break;
  case IROpType::JUMP_IF_NOT_ZERO:
    ss << "jump_if_not_zero " << src1->toString() << " " << label;
    break;
  case IROpType::LABEL:
    ss << label << ":";
    break;
  case IROpType::CALL:
    if (dst) {
      ss << dst->toString() << " = call " << src1->toString() << ", args: ";
    } else {
      ss << "call " << src1->toString() << ", args: ";
    }
    if (src2 && src2->type == IRValueType::ARGS) {
      for (size_t i = 0; i < src2->args.size(); ++i) {
        ss << src2->args[i]->toString();
        if (i < src2->args.size() - 1) {
          ss << ", ";
        }
      }
    }
    break;
  default:
    ss << "unknown_op";
    break;
  }

  return ss.str();
}

// IRFunctionNode implementation
std::string IRFunctionNode::toString() const {
  std::stringstream ss;
  ss << "Function(name=" << identifier
     << ", global=" << (global ? "true" : "false");

  // Print parameters
  if (!parameters.empty()) {
    ss << ", params=[";
    for (size_t i = 0; i < parameters.size(); ++i) {
      ss << parameters[i];
      if (i < parameters.size() - 1) {
        ss << ", ";
      }
    }
    ss << "]";
  }

  ss << ") {\n";

  // Print instructions
  for (const auto &instruction : instructions) {
    if (instruction->opType == IROpType::LABEL) {
      ss << instruction->toString() << "\n";
    } else {
      ss << "    " << instruction->toString() << "\n";
    }
  }
  ss << "}\n";
  return ss.str();
}

// IRStaticVariableNode implementation
std::string IRStaticVariableNode::toString() const {
  std::stringstream ss;
  ss << "StaticVariable(name=" << identifier
     << ", global=" << (global ? "true" : "false")
     << ", type=" << TypeKindToString(type.kind) << ", init=[";

  // Helper lambda to recursively print StaticInit
  std::function<void(const StaticInit &)> printInit =
      [&](const StaticInit &init) {
        switch (init.kind) {
        case StaticInitKind::INT_INIT:
          ss << "Int(" << std::get<int>(init.data) << ")";
          break;
        case StaticInitKind::LONG_INIT:
          ss << "Long(" << std::get<long>(init.data) << ")";
          break;
        case StaticInitKind::UINT_INIT:
          ss << "UInt(" << std::get<unsigned int>(init.data) << ")";
          break;
        case StaticInitKind::ULONG_INIT:
          ss << "ULong(" << std::get<unsigned long>(init.data) << ")";
          break;
        case StaticInitKind::CHAR_INIT:
          ss << "Char(" << (int)std::get<char>(init.data) << ")";
          break;
        case StaticInitKind::UCHAR_INIT:
          ss << "UChar(" << (int)std::get<unsigned char>(init.data) << ")";
          break;
        case StaticInitKind::DOUBLE_INIT:
          ss << "Double(" << std::get<double>(init.data) << ")";
          break;
        case StaticInitKind::ZERO_INIT:
          ss << "ZeroInit(" << std::get<int>(init.data) << ")";
          break;
        case StaticInitKind::STRING_INIT: {
          const auto &stringInit = std::get<StringStaticInit>(init.data);
          ss << "String(\"" << stringInit.value << "\", null_terminated=" 
             << (stringInit.null_terminated ? "true" : "false") << ")";
          break;
        }
        case StaticInitKind::POINTER_INIT: {
          const auto &ptrInit = std::get<PointerStaticInit>(init.data);
          ss << "Pointer(" << ptrInit.name << ")";
          break;
        }
        case StaticInitKind::COMPOUND: {
          ss << "{";
          const auto &compound = std::get<CompoundStaticInit>(init.data);
          for (size_t i = 0; i < compound.initializers.size(); ++i) {
            printInit(compound.initializers[i]);
            if (i < compound.initializers.size() - 1) {
              ss << ", ";
            }
          }
          ss << "}";
          break;
        }
        }
      };

  // Print all initializers
  for (size_t i = 0; i < init_list.size(); ++i) {
    printInit(init_list[i]);
    if (i < init_list.size() - 1) {
      ss << ", ";
    }
  }

  ss << "])";
  return ss.str();
}

// IRStaticConstantNode implementation
std::string IRStaticConstantNode::toString() const {
  std::stringstream ss;
  ss << "StaticConstant(name=" << identifier
     << ", type=" << TypeKindToString(type.kind) << ", init=[";

  // Helper lambda to recursively print StaticInit
  std::function<void(const StaticInit &)> printInit =
      [&](const StaticInit &init) {
        switch (init.kind) {
        case StaticInitKind::INT_INIT:
          ss << "Int(" << std::get<int>(init.data) << ")";
          break;
        case StaticInitKind::LONG_INIT:
          ss << "Long(" << std::get<int>(init.data) << ")";
          break;
        case StaticInitKind::UINT_INIT:
          ss << "UInt(" << std::get<int>(init.data) << ")";
          break;
        case StaticInitKind::ULONG_INIT:
          ss << "ULong(" << std::get<int>(init.data) << ")";
          break;
        case StaticInitKind::CHAR_INIT:
          ss << "Char(" << std::get<int>(init.data) << ")";
          break;
        case StaticInitKind::UCHAR_INIT:
          ss << "UChar(" << std::get<int>(init.data) << ")";
          break;
        case StaticInitKind::DOUBLE_INIT:
          ss << "Double(" << std::get<double>(init.data) << ")";
          break;
        case StaticInitKind::ZERO_INIT:
          ss << "ZeroInit(" << std::get<int>(init.data) << ")";
          break;
        case StaticInitKind::STRING_INIT: {
          const auto &stringInit = std::get<StringStaticInit>(init.data);
          ss << "String(\"" << stringInit.value << "\", null_terminated=" 
             << (stringInit.null_terminated ? "true" : "false") << ")";
          break;
        }
        case StaticInitKind::POINTER_INIT: {
          const auto &ptrInit = std::get<PointerStaticInit>(init.data);
          ss << "Pointer(" << ptrInit.name << ")";
          break;
        }
        case StaticInitKind::COMPOUND: {
          ss << "{";
          const auto &compound = std::get<CompoundStaticInit>(init.data);
          for (size_t i = 0; i < compound.initializers.size(); ++i) {
            printInit(compound.initializers[i]);
            if (i < compound.initializers.size() - 1) {
              ss << ", ";
            }
          }
          ss << "}";
          break;
        }
        }
      };

  // Print all initializers
  for (size_t i = 0; i < init_list.size(); ++i) {
    printInit(init_list[i]);
    if (i < init_list.size() - 1) {
      ss << ", ";
    }
  }

  ss << "])";
  return ss.str();
}

// IRProgramNode implementation
std::string IRProgramNode::toString() const {
  std::stringstream ss;
  ss << "Program(\n";
  for (const auto &item : topLevelItems) {
    ss << item->toString();
    // Add newline if it's a static variable (functions already have newlines)
    if (dynamic_cast<IRStaticVariableNode *>(item.get()) || 
        dynamic_cast<IRStaticConstantNode *>(item.get())) {
      ss << "\n";
    }
  }
  ss << ")\n";
  return ss.str();
}

// IRGenerator implementation
IRProgramPtr IRGenerator::generateIR(const ASTNodePtr &ast) {
  program = std::make_unique<IRProgramNode>();
  tempCounter = 0;
  labelCounter = 0;

  // First, process the AST to generate function definitions
  ast->accept(*this);

  // Second, convert symbol table entries to static variables
  convertSymbolTableToIR();
  // move the staticvariable to the front
  std::stable_partition(
      program->topLevelItems.begin(), program->topLevelItems.end(),
      [](const IRTopLevelPtr &item) {
        return dynamic_cast<IRStaticVariableNode *>(item.get()) != nullptr;
      });

  return std::move(program);
}

// Helper function to calculate type size in bytes
int IRGenerator::getTypeSize(const Type &type) {
  switch (type.kind) {
  case TypeKind::INT:
  case TypeKind::UINT:
    return 4;
  case TypeKind::LONG:
  case TypeKind::ULONG:
  case TypeKind::DOUBLE:
  case TypeKind::POINTER:
    return 8;
  case TypeKind::CHAR:
  case TypeKind::UCHAR:
    return 1;
  case TypeKind::ARRAY: {
    const auto &arrayType = std::get<ArrayType>(type.data);
    return arrayType.size * getTypeSize(*arrayType.element);
  }
  default:
    return 0;
  }
}

void IRGenerator::convertSymbolTableToIR() {
  // Iterate through the global symbol table
  for (const auto &[name, entry] : global_symbol_table) {
    // Skip functions
    if (entry.symbolType == SymbolType::FUNCTION) {
      continue;
    }

    // Skip if it doesn't have static storage (we only want static variables)
    if (entry.storageClass != StorageClass::STATIC) {
      continue;
    }

    // Skip if it has no initializer (not defined in this translation unit)
    if (entry.initType == InitType::UNINITIALIZED) {
      continue;
    }

    // Create initializer list
    std::vector<StaticInit> init_list;

    if (entry.type.kind == TypeKind::ARRAY) {
      // Check if this is a string constant (has stringValue set)
      if (!entry.stringValue.empty()) {
        // This is a string constant - create StringInit
        int arraySize = entry.type.kind == TypeKind::ARRAY 
                       ? std::get<ArrayType>(entry.type.data).size 
                       : entry.stringValue.length() + 1;
        
        // Determine if null-terminated (string length + 1 <= array size)
        bool hasNullTerminator = (entry.stringValue.length() + 1) <= static_cast<size_t>(arraySize);
        
        // Add StringInit
        init_list.push_back(StaticInit::makeStringInit(entry.stringValue, hasNullTerminator));
        
        // Add padding if needed
        int bytesUsed = entry.stringValue.length() + (hasNullTerminator ? 1 : 0);
        int paddingBytes = arraySize - bytesUsed;
        if (paddingBytes > 0) {
          init_list.push_back(StaticInit::makeZeroInit(paddingBytes));
        }
      }
      // For arrays with stored initializer
      else if (entry.initType == InitType::INITIALIZED && entry.initializer) {
        // Convert the stored InitializerNode to StaticInit, passing array type
        // for padding
        init_list.push_back(
            convertToStaticInit(entry.initializer, &entry.type));
      } else if (entry.initType == InitType::TENTATIVE ||
                 entry.initType == InitType::ZERO_INITIALIZED) {
        // Use ZeroInit for the entire array
        int arraySize = getTypeSize(entry.type);
        init_list.push_back(StaticInit::makeZeroInit(arraySize));
      }
    } else if (entry.type.kind == TypeKind::POINTER) {
      // For pointer types
      if (entry.initType == InitType::INITIALIZED) {
        // Check if this pointer is initialized with a string literal
        if (!entry.stringConstantName.empty()) {
          // Create PointerInit pointing to the string constant
          init_list.push_back(StaticInit::makePointerInit(entry.stringConstantName));
        } else {
          // Regular scalar initialization (shouldn't happen for pointers typically)
          init_list.push_back(StaticInit::makeInitial(entry.value));
        }
      } else if (entry.initType == InitType::TENTATIVE ||
                 entry.initType == InitType::ZERO_INITIALIZED) {
        // Use ZeroInit for pointers
        int pointerSize = getTypeSize(entry.type);
        init_list.push_back(StaticInit::makeZeroInit(pointerSize));
      }
    } else {
      // For scalar types
      if (entry.initType == InitType::INITIALIZED) {
        init_list.push_back(StaticInit::makeInitial(entry.value));
      } else if (entry.initType == InitType::TENTATIVE ||
                 entry.initType == InitType::ZERO_INITIALIZED) {
        // Use ZeroInit for scalars too
        int scalarSize = getTypeSize(entry.type);
        init_list.push_back(StaticInit::makeZeroInit(scalarSize));
      }
    }

    // Check if this is a constant (string literals in expressions)
    if (entry.symbolType == SymbolType::CONSTANT) {
      // Create static constant node (read-only)
      auto staticConst = std::make_shared<IRStaticConstantNode>(
          name, entry.type, std::move(init_list));
      program->addTopLevel(std::static_pointer_cast<IRTopLevelNode>(staticConst));
    } else {
      // Determine if it's global (external linkage) or file-scope (internal linkage)
      bool isGlobal = (entry.linkage == LinkageType::EXTERNAL);
      
      // Create static variable node
      auto staticVar = std::make_shared<IRStaticVariableNode>(
          name, isGlobal, entry.type, std::move(init_list));
      program->addStaticVariable(std::move(staticVar));
    }
  }
}

// Helper function to recursively pad an array (or nested arrays) with zeros
void IRGenerator::padArrayWithZeros(const std::string &varName,
                                    const Type &arrayType, int baseOffset) {
  if (arrayType.kind != TypeKind::ARRAY) {
    // Not an array - shouldn't happen, but handle gracefully
    IRValuePtr zeroValue = IRValueNode::makeConstant(0);
    auto copyInst = IRInstructionNode::makeCopyToOffset(std::move(zeroValue),
                                                        varName, baseOffset);
    currentFunction->addInstruction(std::move(copyInst));
    return;
  }

  const auto &arrayData = std::get<ArrayType>(arrayType.data);
  Type elemType = *arrayData.element;
  int elemSize = getTypeSize(elemType);
  int arraySize = arrayData.size;

  int currentOffset = baseOffset;
  for (int i = 0; i < arraySize; i++) {
    if (elemType.kind == TypeKind::ARRAY) {
      // Recursively pad nested array
      padArrayWithZeros(varName, elemType, currentOffset);
    } else {
      // Scalar element - write zero
      IRValuePtr zeroValue = IRValueNode::makeConstant(0);
      auto copyInst = IRInstructionNode::makeCopyToOffset(
          std::move(zeroValue), varName, currentOffset);
      currentFunction->addInstruction(std::move(copyInst));
    }
    currentOffset += elemSize;
  }
}

// Helper function to process compound initializers recursively
void IRGenerator::processCompoundInitializer(InitializerNode *init,
                                             const std::string &varName,
                                             const Type &varType,
                                             int baseOffset) {
  if (!init)
    return;

  if (init->kind == InitializerKind::SINGLE_INIT) {
    // Single initializer - evaluate expression and copy to offset
    auto &singleInit = std::get<SingleInit>(init->data);
    if (singleInit.expression) {
      // Check if this is a string literal initializing a char array
      auto stringLiteral =
          dynamic_cast<StringLiteralExpression *>(singleInit.expression.get());
      if (stringLiteral && varType.kind == TypeKind::ARRAY) {
        // Initialize array with string literal byte-by-byte
        const auto &arrayType = std::get<ArrayType>(varType.data);
        int arraySize = arrayType.size;
        const std::string &str = stringLiteral->value;

        // Copy each character from the string
        int currentOffset = baseOffset;
        for (size_t i = 0;
             i < str.length() && i < static_cast<size_t>(arraySize); ++i) {
          IRValuePtr charValue = IRValueNode::makeConstant(
              static_cast<int>(static_cast<unsigned char>(str[i])));
          auto copyInst = IRInstructionNode::makeCopyToOffset(
              std::move(charValue), varName, currentOffset);
          currentFunction->addInstruction(std::move(copyInst));
          currentOffset += 1;
        }

        // Add null terminator if there's room
        if (str.length() < static_cast<size_t>(arraySize)) {
          IRValuePtr nullValue = IRValueNode::makeConstant(0);
          auto copyInst = IRInstructionNode::makeCopyToOffset(
              std::move(nullValue), varName, currentOffset);
          currentFunction->addInstruction(std::move(copyInst));
          currentOffset += 1;

          // Pad remaining bytes with zeros
          while (currentOffset < baseOffset + arraySize) {
            IRValuePtr zeroValue = IRValueNode::makeConstant(0);
            auto padInst = IRInstructionNode::makeCopyToOffset(
                std::move(zeroValue), varName, currentOffset);
            currentFunction->addInstruction(std::move(padInst));
            currentOffset += 1;
          }
        }
        return;
      }

      // Not a string literal - process normally
      singleInit.expression->accept(*this);

      // Get the expression node to access its type
      auto exprNode =
          dynamic_cast<ExpressionNode *>(singleInit.expression.get());
      if (exprNode && exprNode->type) {
        IRValuePtr exprValue =
            convertExpResult(currentExpResult, *exprNode->type);

        // Generate CopyToOffset instruction
        auto copyInst = IRInstructionNode::makeCopyToOffset(
            std::move(exprValue), varName, baseOffset);
        currentFunction->addInstruction(std::move(copyInst));
      }
    }
  } else if (init->kind == InitializerKind::COMPOUND_INIT) {
    // Compound initializer - process each element recursively
    auto &compoundInit = std::get<CompoundInit>(init->data);

    if (varType.kind == TypeKind::ARRAY) {
      const auto &arrayType = std::get<ArrayType>(varType.data);
      Type elemType = *arrayType.element;
      int elemSize = getTypeSize(elemType);
      int arraySize = arrayType.size;

      // Process each initializer element
      int currentOffset = baseOffset;
      int elementsProvided = compoundInit.initializers.size();

      for (auto &elemInit : compoundInit.initializers) {
        processCompoundInitializer(&elemInit, varName, elemType, currentOffset);
        currentOffset += elemSize;
      }

      // Pad remaining elements with zeros (C standard requires this)
      for (int i = elementsProvided; i < arraySize; i++) {
        // For multidimensional arrays or structs, we need to zero-pad the
        // entire element For scalar types, just write a single zero
        if (elemType.kind == TypeKind::ARRAY) {
          // Recursively zero-pad nested array
          padArrayWithZeros(varName, elemType, currentOffset);
        } else {
          // Generate CopyToOffset with zero constant for scalar
          IRValuePtr zeroValue = IRValueNode::makeConstant(0);
          auto copyInst = IRInstructionNode::makeCopyToOffset(
              std::move(zeroValue), varName, currentOffset);
          currentFunction->addInstruction(std::move(copyInst));
        }
        currentOffset += elemSize;
      }
    }
  }
}

// Helper function to create zero-initialized StaticInit for an array type
StaticInit IRGenerator::createZeroStaticInit(const Type &type) {
  if (type.kind == TypeKind::ARRAY) {
    const auto &arrayData = std::get<ArrayType>(type.data);
    Type elemType = *arrayData.element;
    int arraySize = arrayData.size;

    std::vector<StaticInit> zeroInits;
    for (int i = 0; i < arraySize; i++) {
      zeroInits.push_back(createZeroStaticInit(elemType));
    }
    return StaticInit::makeCompound(std::move(zeroInits));
  } else {
    // Scalar type - return a zero constant
    return StaticInit::makeInitial(0);
  }
}

// Helper function to convert InitializerNode to StaticInit
StaticInit IRGenerator::convertToStaticInit(InitializerNode *init,
                                            const Type *arrayType) {
  if (!init) {
    // Return a zero init for null initializer
    return StaticInit::makeZeroInit(0);
  }

  if (init->kind == InitializerKind::SINGLE_INIT) {
    // Single initializer - extract constant value
    auto &singleInit = std::get<SingleInit>(init->data);
    if (singleInit.expression) {
      // Check if this is a string literal (possibly wrapped in AddressOf)
      StringLiteralExpression *stringLiteral = nullptr;

      // Try direct string literal
      stringLiteral =
          dynamic_cast<StringLiteralExpression *>(singleInit.expression.get());

      // If not direct, check if it's wrapped in AddressOf
      if (!stringLiteral) {
        auto addressOf =
            dynamic_cast<AddressOfExpression *>(singleInit.expression.get());
        if (addressOf && addressOf->variableExpr) {
          stringLiteral = dynamic_cast<StringLiteralExpression *>(
              addressOf->variableExpr.get());
        }
      }

      // Handle string literal initialization for char arrays
      if (stringLiteral && arrayType && arrayType->kind == TypeKind::ARRAY) {
        const auto &arrayData = std::get<ArrayType>(arrayType->data);
        Type elemType = *arrayData.element;

        // Only handle char arrays
        if (elemType.kind == TypeKind::CHAR ||
            elemType.kind == TypeKind::UCHAR || 
            elemType.kind == TypeKind::SCHAR) {
          int arraySize = arrayData.size;
          const std::string &str = stringLiteral->value;
          
          // Determine if we have room for null terminator
          bool hasNullTerminator = str.length() < static_cast<size_t>(arraySize);
          
          // Calculate bytes needed for padding after the string (and null terminator if present)
          int bytesUsed = str.length() + (hasNullTerminator ? 1 : 0);
          int paddingBytes = arraySize - bytesUsed;
          
          // Create the initializer list
          std::vector<StaticInit> initList;
          
          // Add StringInit with the string value and null-termination flag
          initList.push_back(StaticInit::makeStringInit(str, hasNullTerminator));
          
          // Add ZeroInit for remaining padding if needed
          if (paddingBytes > 0) {
            initList.push_back(StaticInit::makeZeroInit(paddingBytes));
          }
          
          // Return compound initializer containing StringInit and optional ZeroInit
          return StaticInit::makeCompound(std::move(initList));
        }
      }

      // Try constant expression
      auto constExpr =
          dynamic_cast<ConstantExpression *>(singleInit.expression.get());
      if (constExpr) {
        // Extract the constant value and convert to the supported variant type
        return std::visit(
            [](auto &&val) -> StaticInit {
              using T = std::decay_t<decltype(val)>;
              if constexpr (std::is_same_v<T, char> ||
                            std::is_same_v<T, unsigned char>) {
                // Convert char types to int
                return StaticInit::makeInitial(static_cast<int>(val));
              } else {
                // int, long, unsigned long, unsigned int, double are directly
                // supported
                return StaticInit::makeInitial(val);
              }
            },
            constExpr->value);
      }
    }
    // If not a constant, return zero init (shouldn't happen for static
    // variables)
    return StaticInit::makeZeroInit(4);
  } else if (init->kind == InitializerKind::COMPOUND_INIT) {
    // Compound initializer - recursively convert each element
    auto &compoundInit = std::get<CompoundInit>(init->data);
    std::vector<StaticInit> staticInits;

    // Get element type if we have an array type
    const Type *elementType = nullptr;
    int expectedCount = 0;
    if (arrayType && arrayType->kind == TypeKind::ARRAY) {
      auto &arrayData = std::get<ArrayType>(arrayType->data);
      elementType = arrayData.element.get();
      expectedCount = arrayData.size;
    }

    // Convert provided initializers
    for (auto &elemInit : compoundInit.initializers) {
      staticInits.push_back(convertToStaticInit(&elemInit, elementType));
    }

    // Pad with zero initializers if we have fewer elements than expected
    if (arrayType && arrayType->kind == TypeKind::ARRAY) {
      int providedCount = staticInits.size();
      if (providedCount < expectedCount) {
        // Add zero initializers for remaining elements
        // For multidimensional arrays, we need to recursively create zero-inits
        for (int i = providedCount; i < expectedCount; i++) {
          if (elementType) {
            staticInits.push_back(createZeroStaticInit(*elementType));
          } else {
            staticInits.push_back(StaticInit::makeInitial(0));
          }
        }
      }
    }

    return StaticInit::makeCompound(std::move(staticInits));
  }

  return StaticInit::makeZeroInit(0);
}

void IRGenerator::visit(IfStatement &node) {
  if (node.condition) {
    // Generate IR for condition with lvalue-to-rvalue conversion
    node.condition->accept(*this);
    auto condExpr = dynamic_cast<ExpressionNode *>(node.condition.get());
    IRValuePtr conditionValue =
        currentExpResult.type == ExpResultType::PLAIN_OPERAND
            ? currentValue
            : (condExpr && condExpr->type
                   ? convertExpResult(currentExpResult, *condExpr->type)
                   : currentValue);

    // Generate labels for branching
    std::string elseLabel = generateLabelName();
    std::string endLabel = generateLabelName();

    // Create jump instruction based on condition
    auto jumpInst =
        IRInstructionNode::makeJumpIfZero(std::move(conditionValue), elseLabel);
    currentFunction->addInstruction(std::move(jumpInst));

    // Generate IR for 'then' block
    if (node.thenBranch) {
      node.thenBranch->accept(*this);
    }

    // Jump to end after 'then' block
    auto jumpToEndInst = IRInstructionNode::makeJump(endLabel);
    currentFunction->addInstruction(std::move(jumpToEndInst));

    // Else label
    auto elseLabelInst = IRInstructionNode::makeLabel(elseLabel);
    currentFunction->addInstruction(std::move(elseLabelInst));

    // Generate IR for 'else' block if it exists
    if (node.elseBranch) {
      (*node.elseBranch)->accept(*this);
    }

    // End label
    auto endLabelInst = IRInstructionNode::makeLabel(endLabel);
    currentFunction->addInstruction(std::move(endLabelInst));
  }
}

void IRGenerator::visit(PostfixExpression &node) {
  if (node.operand) {
    // Generate IR for operand with lvalue-to-rvalue conversion
    node.operand->accept(*this);
    auto operandExpr = dynamic_cast<ExpressionNode *>(node.operand.get());
    IRValuePtr operand =
        currentExpResult.type == ExpResultType::PLAIN_OPERAND
            ? currentValue
            : (operandExpr && operandExpr->type
                   ? convertExpResult(currentExpResult, *operandExpr->type)
                   : currentValue);

    // Create temporary for result with proper type tracking
    IRValuePtr result = makeTackyVariable(*node.type);

    // Convert token type to IR operation
    IROpType irOp;
    if (node.op == TokenType::INCREMENT_OPERATOR) {
      irOp = IROpType::ADD;
    } else if (node.op == TokenType::DECREMENT_OPERATOR) {
      irOp = IROpType::SUBTRACT;
    } else {
      // Unsupported postfix operation
      return;
    }

    // Create constant value of 1
    IRValuePtr one = IRValueNode::makeConstant(1);
    IRValuePtr operandcopytemp = makeTackyVariable(*node.type);
    auto copyinst = IRInstructionNode::makeCopy(
        std::make_shared<IRValueNode>(*operand), operandcopytemp);
    currentFunction->addInstruction(std::move(copyinst));
    // Create postfix instruction
    auto inst =
        IRInstructionNode::makeBinary(irOp, result, operand, std::move(one));
    currentFunction->addInstruction(std::move(inst));

    inst = IRInstructionNode::makeCopy(std::make_shared<IRValueNode>(*result),
                                       (operand));
    currentFunction->addInstruction(std::move(inst));
    // Update current value to the operand value before assignment
    currentValue = operandcopytemp;
    currentExpResult = ExpResult::makePlainOperand(currentValue);
  }
}
void IRGenerator::visit(ConditionalExpression &node) {
  if (node.condition) {
    // Generate IR for condition with lvalue-to-rvalue conversion
    node.condition->accept(*this);
    auto condExpr = dynamic_cast<ExpressionNode *>(node.condition.get());
    IRValuePtr conditionValue =
        currentExpResult.type == ExpResultType::PLAIN_OPERAND
            ? currentValue
            : (condExpr && condExpr->type
                   ? convertExpResult(currentExpResult, *condExpr->type)
                   : currentValue);

    // Generate labels for branching
    std::string falseLabel = generateLabelName();
    std::string endLabel = generateLabelName();

    // Create jump instruction based on condition
    auto jumpInst = IRInstructionNode::makeJumpIfZero(std::move(conditionValue),
                                                      falseLabel);
    currentFunction->addInstruction(std::move(jumpInst));

    // Check if result type is void
    bool isVoid = (node.type && node.type->kind == TypeKind::VOID);

    // Generate IR for 'then' block with lvalue-to-rvalue conversion
    if (node.trueExpr) {
      node.trueExpr->accept(*this);
    }
    
    IRValuePtr result = nullptr;
    
    if (!isVoid) {
      auto trueExprNode = dynamic_cast<ExpressionNode *>(node.trueExpr.get());
      IRValuePtr trueExprValue =
          currentExpResult.type == ExpResultType::PLAIN_OPERAND
              ? currentValue
              : (trueExprNode && trueExprNode->type
                     ? convertExpResult(currentExpResult, *trueExprNode->type)
                     : currentValue);
      // Create a temporary variable to hold the result with proper type tracking
      result = makeTackyVariable(*node.type);
      // Assign true expression value to result
      auto copyTrueInst = IRInstructionNode::makeCopy(
          std::move(trueExprValue), std::make_shared<IRValueNode>(result));
      currentFunction->addInstruction(std::move(copyTrueInst));
    }

    // Jump to end after 'then' block
    auto jumpToEndInst = IRInstructionNode::makeJump(endLabel);
    currentFunction->addInstruction(std::move(jumpToEndInst));

    // False label
    auto falseLabelInst = IRInstructionNode::makeLabel(falseLabel);
    currentFunction->addInstruction(std::move(falseLabelInst));

    // Generate IR for 'false' block if it exists with lvalue-to-rvalue conversion
    if (node.falseExpr) {
      (node.falseExpr)->accept(*this);
    }

    if (!isVoid) {
      auto falseExprNode = dynamic_cast<ExpressionNode *>(node.falseExpr.get());
      IRValuePtr falseExprValue =
          currentExpResult.type == ExpResultType::PLAIN_OPERAND
              ? currentValue
              : (falseExprNode && falseExprNode->type
                     ? convertExpResult(currentExpResult, *falseExprNode->type)
                     : currentValue);
      // Assign false expression value to result
      auto copyFalseInst = IRInstructionNode::makeCopy(
          std::move(falseExprValue), std::make_shared<IRValueNode>(result));
      currentFunction->addInstruction(std::move(copyFalseInst));
    }
    
    // End label
    auto endLabelInst = IRInstructionNode::makeLabel(endLabel);
    currentFunction->addInstruction(std::move(endLabelInst));
    
    if (!isVoid) {
      currentValue = result;
      currentExpResult = ExpResult::makePlainOperand(currentValue);
    } else {
      // For void conditional, set dummy value
      currentValue = IRValueNode::makeConstant(0);
      currentExpResult = ExpResult::makePlainOperand(currentValue);
    }
  }
}

void IRGenerator::visit(ProgramNode &node) {
  // Process all declarations in the program
  for (const auto &declaration : node.Declarations) {
    declaration->accept(*this);
  }
}

void IRGenerator::visit(FunctionDefinitionNode &node) {
  (void)node;
  // not used
}

void IRGenerator::visit(VarDeclNode &node) {
  // Skip file-scope variables - they'll be generated from the symbol table
  if (!currentFunction) {
    return;
  }

  // Skip local static and extern variables - they're in the symbol table
  if (node.storage_class.has_value()) {
    if (node.storage_class.value() == TokenType::STATIC ||
        node.storage_class.value() == TokenType::EXTERN) {
      return;
    }
  }

  // Local automatic variable
  IRValuePtr var = IRValueNode::makeVariable(node.name);

  // If there's an initializer, generate IR for it
  if (node.init) {
    InitializerNode *initNode =
        dynamic_cast<InitializerNode *>(node.init->get());
    // Check if this is an array with initializer
    if (initNode && node.type.kind == TypeKind::ARRAY) {
      if (initNode->kind == InitializerKind::COMPOUND_INIT) {
        // Process compound initializer with CopyToOffset instructions
        processCompoundInitializer(initNode, node.name, node.type, 0);
      } else if (initNode->kind == InitializerKind::SINGLE_INIT) {
        // Check if this is a string literal initializing an array
        auto &singleInit = std::get<SingleInit>(initNode->data);
        auto stringLiteral = dynamic_cast<StringLiteralExpression *>(
            singleInit.expression.get());
        if (stringLiteral) {
          // Process string literal as array initializer
          processCompoundInitializer(initNode, node.name, node.type, 0);
        } else {
          // Single non-string initializer for array - shouldn't happen normally
          (*node.init)->accept(*this);
          IRValuePtr initValue =
              currentExpResult.type == ExpResultType::PLAIN_OPERAND
                  ? currentValue
                  : convertExpResult(currentExpResult, node.type);
          auto copyInst =
              IRInstructionNode::makeCopy(std::move(initValue), std::move(var));
          currentFunction->addInstruction(std::move(copyInst));
        }
      }
    } else {
      // Scalar or single initializer
      (*node.init)->accept(*this);
      // After visiting the initializer, currentValue and currentExpResult are
      // set by the inner expression Perform lvalue conversion if needed (check
      // if we have a dereferenced pointer)
      IRValuePtr initValue =
          currentExpResult.type == ExpResultType::PLAIN_OPERAND
              ? currentValue
              : convertExpResult(currentExpResult, node.type);
      // Create copy instruction to assign initializer value to variable
      auto copyInst =
          IRInstructionNode::makeCopy(std::move(initValue), std::move(var));
      currentFunction->addInstruction(std::move(copyInst));
    }
  }
}

void IRGenerator::visit(FunDeclNode &node) {
  if (!node.body.has_value()) {
    return;
  }

  // Determine if function is global based on linkage
  bool isGlobal = global_symbol_table[node.name].linkage == LinkageType::EXTERNAL;

  std::shared_ptr<IRFunctionNode> func =
      std::make_shared<IRFunctionNode>(node.name, isGlobal);

  // Add parameters
  for (const auto &param : node.param_names) {
    func->parameters.push_back(param);
  }

  currentFunction = func;
  // Generate IR for function body
  if (node.body) {
    (*node.body)->accept(*this);
  }
  func->addInstruction(IRInstructionNode::makeReturn(
      IRValueNode::makeConstant(0))); // Ensure function ends with return
  program->addFunction(std::move(func));
  currentFunction = nullptr;
}

void IRGenerator::visit(BlockNode &node) {
  // Process all block items
  for (const auto &item : node.block_items) {
    item->accept(*this);
  }
}

void IRGenerator::visit(BlockItemNode &node) {
  // Delegate to the contained statement or declaration
  if (node.block_item) {
    node.block_item->accept(*this);
  }
}

void IRGenerator::visit(ReturnStatement &node) {
  IRValuePtr returnValue = nullptr;

  if (node.expression) {
    // Generate IR for the return expression
    node.expression->accept(*this);

    // Perform lvalue conversion if needed
    auto exprNode = dynamic_cast<ExpressionNode *>(node.expression.get());
    if (exprNode && exprNode->type) {
      returnValue = convertExpResult(currentExpResult, *exprNode->type);
    } else if (currentValue) {
      returnValue = std::make_shared<IRValueNode>(*currentValue);
    }
  }

  // Create return instruction
  auto returnInst = IRInstructionNode::makeReturn(std::move(returnValue));
  currentFunction->addInstruction(std::move(returnInst));
}

void IRGenerator::visit(ExpressionStatement &node) {
  if (node.expression) {
    node.expression->accept(*this);
    // Result is in currentValue but we don't need to do anything with it
  }
}

void IRGenerator::visit(ConstantExpression &node) {
  // Create a constant value
  // Extract the value from the variant
  currentValue = IRValueNode::makeConstant(node.value);
  currentExpResult = ExpResult::makePlainOperand(currentValue);
}

void IRGenerator::visit(VariableExpression &node) {
  // Create a variable reference
  currentValue = IRValueNode::makeVariable(node.identifier);
  currentExpResult = ExpResult::makePlainOperand(currentValue);
}

void IRGenerator::visit(UnaryExpression &node) {
  // Generate IR for operand with lvalue-to-rvalue conversion
  node.operand->accept(*this);
  auto operandExpr = dynamic_cast<ExpressionNode *>(node.operand.get());
  IRValuePtr operand =
      currentExpResult.type == ExpResultType::PLAIN_OPERAND
          ? currentValue
          : (operandExpr && operandExpr->type
                 ? convertExpResult(currentExpResult, *operandExpr->type)
                 : currentValue);

  // Create temporary for result with proper type tracking
  IRValuePtr result = makeTackyVariable(*node.type);

  // Convert token type to IR operation
  IROpType irOp = tokenTypeToUnaryIR(node.op);

  if (node.op == INCREMENT_OPERATOR || node.op == DECREMENT_OPERATOR) {
    // Create unary instruction
    // Create constant value of 1
    IRValuePtr one = IRValueNode::makeConstant(1);

    // Create postfix instruction
    auto inst = IRInstructionNode::makeBinary(
        node.op == INCREMENT_OPERATOR ? IROpType::ADD : IROpType::SUBTRACT,
        result, operand, std::move(one));
    currentFunction->addInstruction(std::move(inst));
    inst = IRInstructionNode::makeCopy(std::make_shared<IRValueNode>(*result),
                                       std::move(operand));
    currentFunction->addInstruction(std::move(inst));
  } else {
    // Create unary instruction
    auto inst = IRInstructionNode::makeUnary(irOp, result, operand);
    currentFunction->addInstruction(std::move(inst));
  }
  currentValue = std::move(result);
  currentExpResult = ExpResult::makePlainOperand(currentValue);
}

void IRGenerator::visit(BinaryExpression &node) {
  // Handle short-circuiting operators specially
  if (node.op == TokenType::LAND) {
    // Implement && operator with short-circuiting
    // e1 && e2 pattern:
    // <instructions for e1>
    // v1 = <result of e1>
    // JumpIfZero(v1, false_label)
    // <instructions for e2>
    // v2 = <result of e2>
    // JumpIfZero(v2, false_label)
    // result = 1
    // Jump(end)
    // Label(false_label)
    // result = 0
    // Label(end)

    std::string falseLabel = generateLabelName();
    std::string endLabel = generateLabelName();
    // Logical operators always produce int type (0 or 1)
    IRValuePtr result = makeTackyVariable(Type::Int());

    // Generate IR for left operand with lvalue-to-rvalue conversion
    node.left->accept(*this);
    auto leftExpr = dynamic_cast<ExpressionNode *>(node.left.get());
    IRValuePtr leftValue =
        currentExpResult.type == ExpResultType::PLAIN_OPERAND
            ? currentValue
            : (leftExpr && leftExpr->type
                   ? convertExpResult(currentExpResult, *leftExpr->type)
                   : currentValue);

    // Jump to false_label if left operand is zero
    auto jumpIfZero1 = IRInstructionNode::makeJumpIfZero(leftValue, falseLabel);
    currentFunction->addInstruction(std::move(jumpIfZero1));

    // Generate IR for right operand with lvalue-to-rvalue conversion
    node.right->accept(*this);
    auto rightExpr = dynamic_cast<ExpressionNode *>(node.right.get());
    IRValuePtr rightValue =
        currentExpResult.type == ExpResultType::PLAIN_OPERAND
            ? currentValue
            : (rightExpr && rightExpr->type
                   ? convertExpResult(currentExpResult, *rightExpr->type)
                   : currentValue);

    // Jump to false_label if right operand is zero
    auto jumpIfZero2 =
        IRInstructionNode::makeJumpIfZero(rightValue, falseLabel);
    currentFunction->addInstruction(std::move(jumpIfZero2));

    // Both operands are true, set result to 1
    auto setTrue = IRInstructionNode::makeCopy(
        IRValueNode::makeConstant(1), std::make_shared<IRValueNode>(*result));
    currentFunction->addInstruction(std::move(setTrue));

    // Jump over the false case
    auto jumpEnd = IRInstructionNode::makeJump(endLabel);
    currentFunction->addInstruction(std::move(jumpEnd));

    // False label: set result to 0
    auto falseLabelInst = IRInstructionNode::makeLabel(falseLabel);
    currentFunction->addInstruction(std::move(falseLabelInst));

    auto setFalse = IRInstructionNode::makeCopy(
        IRValueNode::makeConstant(0), std::make_shared<IRValueNode>(*result));
    currentFunction->addInstruction(std::move(setFalse));

    // End label
    auto endLabelInst = IRInstructionNode::makeLabel(endLabel);
    currentFunction->addInstruction(std::move(endLabelInst));

    currentValue = std::move(result);
    currentExpResult = ExpResult::makePlainOperand(currentValue);
    return;
  }

  if (node.op == TokenType::LOR) {
    // Implement || operator with short-circuiting
    // e1 || e2 pattern:
    // <instructions for e1>
    // v1 = <result of e1>
    // JumpIfNotZero(v1, true_label)
    // <instructions for e2>
    // v2 = <result of e2>
    // JumpIfNotZero(v2, true_label)
    // result = 0
    // Jump(end)
    // Label(true_label)
    // result = 1
    // Label(end)

    std::string trueLabel = generateLabelName();
    std::string endLabel = generateLabelName();
    // Logical operators always produce int type (0 or 1)
    IRValuePtr result = makeTackyVariable(Type::Int());

    // Generate IR for left operand with lvalue-to-rvalue conversion
    node.left->accept(*this);
    auto leftExpr = dynamic_cast<ExpressionNode *>(node.left.get());
    IRValuePtr leftValue =
        currentExpResult.type == ExpResultType::PLAIN_OPERAND
            ? currentValue
            : (leftExpr && leftExpr->type
                   ? convertExpResult(currentExpResult, *leftExpr->type)
                   : currentValue);

    // Jump to true_label if left operand is non-zero
    auto jumpIfNotZero1 =
        IRInstructionNode::makeJumpIfNotZero(leftValue, trueLabel);
    currentFunction->addInstruction(std::move(jumpIfNotZero1));

    // Generate IR for right operand with lvalue-to-rvalue conversion
    node.right->accept(*this);
    auto rightExpr = dynamic_cast<ExpressionNode *>(node.right.get());
    IRValuePtr rightValue =
        currentExpResult.type == ExpResultType::PLAIN_OPERAND
            ? currentValue
            : (rightExpr && rightExpr->type
                   ? convertExpResult(currentExpResult, *rightExpr->type)
                   : currentValue);

    // Jump to true_label if right operand is non-zero
    auto jumpIfNotZero2 =
        IRInstructionNode::makeJumpIfNotZero(rightValue, trueLabel);
    currentFunction->addInstruction(std::move(jumpIfNotZero2));

    // Both operands are false, set result to 0
    auto setFalse = IRInstructionNode::makeCopy(
        IRValueNode::makeConstant(0), std::make_shared<IRValueNode>(*result));
    currentFunction->addInstruction(std::move(setFalse));

    // Jump over the true case
    auto jumpEnd = IRInstructionNode::makeJump(endLabel);
    currentFunction->addInstruction(std::move(jumpEnd));

    // True label: set result to 1
    auto trueLabelInst = IRInstructionNode::makeLabel(trueLabel);
    currentFunction->addInstruction(std::move(trueLabelInst));

    auto setTrue = IRInstructionNode::makeCopy(
        IRValueNode::makeConstant(1), std::make_shared<IRValueNode>(*result));
    currentFunction->addInstruction(std::move(setTrue));

    // End label
    auto endLabelInst = IRInstructionNode::makeLabel(endLabel);
    currentFunction->addInstruction(std::move(endLabelInst));

    currentValue = std::move(result);
    currentExpResult = ExpResult::makePlainOperand(currentValue);
    return;
  }

  // Handle regular binary operations (non-short-circuiting)
  // Generate IR for left operand with lvalue-to-rvalue conversion
  node.left->accept(*this);
  auto leftExpr = dynamic_cast<ExpressionNode *>(node.left.get());
  IRValuePtr leftValue =
      currentExpResult.type == ExpResultType::PLAIN_OPERAND
          ? currentValue
          : (leftExpr && leftExpr->type
                 ? convertExpResult(currentExpResult, *leftExpr->type)
                 : currentValue);

  // Generate IR for right operand with lvalue-to-rvalue conversion
  node.right->accept(*this);
  auto rightExpr = dynamic_cast<ExpressionNode *>(node.right.get());
  IRValuePtr rightValue =
      currentExpResult.type == ExpResultType::PLAIN_OPERAND
          ? currentValue
          : (rightExpr && rightExpr->type
                 ? convertExpResult(currentExpResult, *rightExpr->type)
                 : currentValue);

  // Create temporary for result with proper type tracking
  IRValuePtr result = makeTackyVariable(*node.type);

  // Convert token type to IR operation
  IROpType irOp = tokenTypeToBinaryIR(node.op);

  // Create binary instruction
  auto inst = IRInstructionNode::makeBinary(irOp, result, std::move(leftValue),
                                            std::move(rightValue));
  currentFunction->addInstruction(std::move(inst));

  currentValue = std::move(result);
  currentExpResult = ExpResult::makePlainOperand(currentValue);
}

void IRGenerator::visit(AssignmentExpression &node) {
  // Generate IR for right side (value being assigned) - WITH lvalue conversion
  node.right->accept(*this);
  auto rightExpr = dynamic_cast<ExpressionNode *>(node.right.get());
  IRValuePtr rightValue =
      currentExpResult.type == ExpResultType::PLAIN_OPERAND
          ? currentValue
          : (rightExpr && rightExpr->type
                 ? convertExpResult(currentExpResult, *rightExpr->type)
                 : currentValue);

  // Generate IR for left side WITHOUT lvalue conversion
  // Save and restore state to avoid lvalue conversion
  node.left->accept(*this);
  ExpResult leftResult = currentExpResult;

  // Check if left side is a dereferenced pointer or plain variable
  if (leftResult.type == ExpResultType::DEREFERENCED_POINTER) {
    // *ptr = value => Store(value, ptr)
    auto storeInst = IRInstructionNode::makeStore(rightValue, leftResult.value);
    currentFunction->addInstruction(storeInst);
    // Assignment result is the assigned value
    currentValue = rightValue;
    currentExpResult = ExpResult::makePlainOperand(rightValue);
  } else {
    // Normal variable assignment => Copy(value, var)
    IRValuePtr leftValue = leftResult.value;
    auto copyInst = IRInstructionNode::makeCopy(rightValue, leftValue);
    currentFunction->addInstruction(copyInst);
    // Assignment result is the assigned value
    currentValue = rightValue;
    currentExpResult = ExpResult::makePlainOperand(rightValue);
  }
}

void IRGenerator::visit(CompoundStatement &node) { node.block->accept(*this); }

void IRGenerator::visit(BreakNode &node) {
  IRInstructionPtr breakInst =
      IRInstructionNode::makeJump("break_" + node.label);
  currentFunction->addInstruction(std::move(breakInst));
}

void IRGenerator::visit(ContinueNode &node) {
  IRInstructionPtr continueInst =
      IRInstructionNode::makeJump("continue_" + node.label);
  currentFunction->addInstruction(std::move(continueInst));
}

void IRGenerator::visit(DoWhileNode &node) {
  std::string startLabel = generateLabelName();
  // Start label
  auto startLabelInst = IRInstructionNode::makeLabel(startLabel);
  currentFunction->addInstruction(std::move(startLabelInst));
  // Generate IR for body
  if (node.body) {
    node.body->accept(*this);
  }
  // continue label
  auto continueLabel = IRInstructionNode::makeLabel("continue_" + node.label);
  currentFunction->addInstruction(std::move(continueLabel));

  // Generate IR for condition
  IRValuePtr conditionValue;
  if (node.condition) {
    node.condition->accept(*this);
    auto condExpr = dynamic_cast<ExpressionNode *>(node.condition.get());
    conditionValue =
        currentExpResult.type == ExpResultType::PLAIN_OPERAND
            ? currentValue
            : (condExpr && condExpr->type
                   ? convertExpResult(currentExpResult, *condExpr->type)
                   : currentValue);
    // Create jump instruction based on condition
  }
  auto jumpInst = IRInstructionNode::makeJumpIfNotZero(
      std::move(conditionValue), startLabel);
  currentFunction->addInstruction(std::move(jumpInst));
  // break label
  auto breakLabel = IRInstructionNode::makeLabel("break_" + node.label);
  currentFunction->addInstruction(std::move(breakLabel));
}

void IRGenerator::visit(WhileNode &node) {
  auto continueLabel = "continue_" + node.label;
  auto breakLabel = "break_" + node.label;
  auto startInstr = IRInstructionNode::makeLabel(continueLabel);
  currentFunction->addInstruction(std::move(startInstr));

  // condition instructions
  auto conditionValue = std::make_shared<IRValueNode>();
  // always true for while and do while
  if (node.condition) {
    node.condition->accept(*this);
    auto condExpr = dynamic_cast<ExpressionNode *>(node.condition.get());
    conditionValue =
        currentExpResult.type == ExpResultType::PLAIN_OPERAND
            ? currentValue
            : (condExpr && condExpr->type
                   ? convertExpResult(currentExpResult, *condExpr->type)
                   : currentValue);
  }
  IRInstructionPtr jumpInstr;
  jumpInstr = IRInstructionNode::makeJumpIfZero(conditionValue, breakLabel);
  currentFunction->addInstruction(std::move(jumpInstr));
  // body instructions
  if (node.body) {
    node.body->accept(*this);
  }
  // jump back to continue
  auto jumpBackInstr = IRInstructionNode::makeJump(continueLabel);
  currentFunction->addInstruction(std::move(jumpBackInstr));
  // break label
  auto breakInstr = IRInstructionNode::makeLabel(breakLabel);
  currentFunction->addInstruction(std::move(breakInstr));
}

void IRGenerator::visit(ForNode &node) {
  // instructions for init
  if (node.init) {
    node.init->accept(*this);
  }
  // start label
  auto startLabel = generateLabelName();
  auto continueLabel = "continue_" + node.label;
  auto breakLabel = "break_" + node.label;
  auto startLabelInstr = IRInstructionNode::makeLabel(startLabel);
  currentFunction->addInstruction(std::move(startLabelInstr));
  // condition instructions
  IRInstructionPtr jumpInstr;
  IRValuePtr conditionValue;
  // if condition is not present then no need for an additional jump instruction
  if (node.condition) {
    (*node.condition)->accept(*this);
    auto condExpr = dynamic_cast<ExpressionNode *>((*node.condition).get());
    conditionValue =
        currentExpResult.type == ExpResultType::PLAIN_OPERAND
            ? currentValue
            : (condExpr && condExpr->type
                   ? convertExpResult(currentExpResult, *condExpr->type)
                   : currentValue);
    jumpInstr = IRInstructionNode::makeJumpIfZero(conditionValue, breakLabel);
    currentFunction->addInstruction(std::move(jumpInstr));
  }
  // body instructions
  if (node.body) {
    node.body->accept(*this);
  }
  // continue label
  auto continueLabelInstr = IRInstructionNode::makeLabel(continueLabel);
  currentFunction->addInstruction(std::move(continueLabelInstr));
  // post instructions
  if (node.post) {
    (*node.post)->accept(*this);
  }
  // jump back to start
  auto jumpBackInstr = IRInstructionNode::makeJump(startLabel);
  currentFunction->addInstruction(std::move(jumpBackInstr));
  // break label
  auto breakInstr = IRInstructionNode::makeLabel(breakLabel);
  currentFunction->addInstruction(std::move(breakInstr));
}

void IRGenerator::visit(FunctionCallNode &node) {
  // Generate IR for arguments
  std::vector<IRValuePtr> argValues;
  for (const auto &arg : node.args) {
    arg->accept(*this);
    argValues.push_back(std::make_shared<IRValueNode>(*currentValue));
  }

  // Create IRValue for function name
  IRValuePtr funcValue = IRValueNode::makeVariable(node.name);

  // Check if function returns void
  IRValuePtr result = nullptr;
  if (node.type->kind != TypeKind::VOID) {
    // Create temporary for result with proper type tracking (only for non-void functions)
    result = makeTackyVariable(*node.type);
  }

  // Create call instruction (result will be nullptr for void functions)
  auto callInst = IRInstructionNode::makeCall(
      funcValue, IRValueNode::makeArgs(argValues), result);
  currentFunction->addInstruction(std::move(callInst));

  if (result) {
    currentValue = std::move(result);
    currentExpResult = ExpResult::makePlainOperand(currentValue);
  } else {
    // For void functions, we don't have a meaningful result
    // Set currentValue to null or a dummy value
    currentValue = nullptr;
    currentExpResult = ExpResult::makePlainOperand(nullptr);
  }
}

IROpType IRGenerator::tokenTypeToBinaryIR(TokenType tokenType) {
  switch (tokenType) {
  case TokenType::PLUS:
    return IROpType::ADD;
  case TokenType::HYPHEN:
    return IROpType::SUBTRACT;
  case TokenType::ASTERISK:
    return IROpType::MULTIPLY;
  case TokenType::FORWARD_SLASH:
    return IROpType::DIVIDE;
  case TokenType::PERCENT_SIGN:
    return IROpType::REMAINDER;
  case TokenType::NOT:
    return IROpType::NOT;
  case TokenType::AAND:
    return IROpType::AND;
  case TokenType::AOR:
    return IROpType::OR;
  case TokenType::XOR:
    return IROpType::XOR;
  case TokenType::EQUAL:
    return IROpType::EQUAL;
  case TokenType::NOTEQUAL:
    return IROpType::NOT_EQUAL;
  case TokenType::LESSTHAN:
    return IROpType::LESS_THAN;
  case TokenType::LESSTHANEQUAL:
    return IROpType::LESS_EQUAL;
  case TokenType::GREATERTHAN:
    return IROpType::GREATER_THAN;
  case TokenType::GREATERTHANEQUAL:
    return IROpType::GREATER_EQUAL;
  case TokenType::LEFT_SHIFT:
    return IROpType::LEFT_SHIFT;
  case TokenType::RIGHT_SHIFT:
    return IROpType::RIGHT_SHIFT;
  case TokenType::LAND:
    return IROpType::LOGICAL_AND;
  case TokenType::LOR:
    return IROpType::LOGICAL_OR;
  default:
    return IROpType::ADD; // Default fallback
  }
}

IROpType IRGenerator::tokenTypeToUnaryIR(TokenType tokenType) {
  switch (tokenType) {
  case TokenType::HYPHEN:
    return IROpType::NEGATE;
  case TokenType::TILDE:
    return IROpType::COMPLEMENT;
  case TokenType::NOT:
    return IROpType::NOT;
  default:
    return IROpType::NEGATE; // Default fallback
  }
}

IRValuePtr IRGenerator::createTemporary() {
  return IRValueNode::makeTemporary(generateTempName());
}

// Helper function for generating TACKY variables with type tracking
IRValuePtr IRGenerator::makeTackyVariable(Type varType) {
  std::string varName = generateTempName();

  // Add to symbol table with type and LocalAttr
  SymbolTableEntry entry(varName, SymbolType::VARIABLE, InitType::UNINITIALIZED,
                         varType);
  entry.linkage = LinkageType::NONE;
  entry.storageClass = StorageClass::AUTO;
  global_symbol_table[varName] = entry;

  return IRValueNode::makeTemporary(varName);
}

// Helper function to convert ExpResult to IRValuePtr with lvalue conversion
IRValuePtr IRGenerator::convertExpResult(const ExpResult &result,
                                         const Type &exprType) {
  if (result.type == ExpResultType::PLAIN_OPERAND) {
    // Plain operand - just return as is
    return result.value;
  } else {
    // Dereferenced pointer - emit Load instruction
    IRValuePtr dst = makeTackyVariable(exprType);
    auto loadInst = IRInstructionNode::makeLoad(result.value, dst);
    currentFunction->addInstruction(loadInst);
    return dst;
  }
}

// Valor class implementation
IRProgramPtr Valor::convertToIR(const ASTNodePtr &ast) {
  return generator.generateIR(ast);
}

// <------------------------------------------------------------------------------------->
void IRGenerator::visit(GotoStatement &node) { (void)node;}
void IRGenerator::visit(LabelStatement &node) { (void)node;}

void IRGenerator::visit(CastExpression &node) {
  // Handle cast to void - just process the inner expression for side effects
  if (node.targetType.kind == TypeKind::VOID) {
    // Process the inner expression to execute any side effects
    node.expression->accept(*this);
    // Return a dummy value (the caller won't use it)
    currentValue = IRValueNode::makeConstant(0);
    currentExpResult = ExpResult::makePlainOperand(currentValue);
    return;
  }

  // Generate IR for the inner expression
  node.expression->accept(*this);
  IRValuePtr result = std::make_shared<IRValueNode>(*currentValue);

  // Get the type we're casting from (the inner expression's type)
  auto exp = dynamic_cast<ExpressionNode *>(node.expression.get());
  Type innerType = *exp->type;

  // If already the correct type, no cast needed
  if (node.targetType == innerType) {
    currentValue = result;
    currentExpResult = ExpResult::makePlainOperand(currentValue);
    return;
  }

  // Create destination variable with the target type
  IRValuePtr dst = makeTackyVariable(node.targetType);

  // Handle pointer casts - treat pointers like ULong
  Type effectiveInnerType = innerType;
  Type effectiveTargetType = node.targetType;

  if (innerType.kind == TypeKind::POINTER) {
    effectiveInnerType = Type::ULong();
  }
  if (node.targetType.kind == TypeKind::POINTER) {
    effectiveTargetType = Type::ULong();
  }

  if (effectiveTargetType.kind == TypeKind::DOUBLE &&
      (effectiveInnerType.kind == TypeKind::INT ||
       effectiveInnerType.kind == TypeKind::LONG)) {
    // Int/Long to Double
    if (effectiveInnerType.kind == TypeKind::INT) {
      // First convert Long to Int
      IRValuePtr longTemp = makeTackyVariable(Type::Long());
      auto intToLongInst =
          IRInstructionNode::makeSignExtend(std::move(result), longTemp);
      currentFunction->addInstruction(std::move(intToLongInst));
      result = longTemp;
    }
    auto longToDoubleInst =
        IRInstructionNode::makeLongToDouble(std::move(result), dst);
    currentFunction->addInstruction(std::move(longToDoubleInst));
    currentValue = dst;
    currentExpResult = ExpResult::makePlainOperand(currentValue);
    return;
  } else if ((effectiveTargetType.kind == TypeKind::INT ||
              effectiveTargetType.kind == TypeKind::LONG) &&
             effectiveInnerType.kind == TypeKind::DOUBLE) {
    // Double to Int/Long
    auto doubleToLongInst =
        IRInstructionNode::makeDoubleToLong(std::move(result), dst);
    currentFunction->addInstruction(std::move(doubleToLongInst));
    if (effectiveTargetType.kind == TypeKind::INT) {
      // Then convert Int to Long
      IRValuePtr intTemp = makeTackyVariable(Type::Int());
      auto longToIntInst = IRInstructionNode::makeSignExtend(
          std::make_shared<IRValueNode>(*dst), intTemp);
      currentFunction->addInstruction(std::move(longToIntInst));
      dst = intTemp;
    }
    currentValue = dst;
    currentExpResult = ExpResult::makePlainOperand(currentValue);
    return;
  } else if (effectiveInnerType.kind == TypeKind::DOUBLE &&
             (effectiveTargetType.kind == TypeKind::UINT ||
              effectiveTargetType.kind == TypeKind::ULONG)) {
    // Double to Unsigned Int/Long
    auto doubleToLongInst =
        IRInstructionNode::makeDoubleToLong(std::move(result), dst);
    currentFunction->addInstruction(std::move(doubleToLongInst));
    if (effectiveTargetType.kind == TypeKind::UINT) {
      // Then convert Int to Long
      IRValuePtr intTemp = makeTackyVariable(Type::UInt());
      auto longToIntInst = IRInstructionNode::makeTruncate(
          std::make_shared<IRValueNode>(*dst), intTemp);
      currentFunction->addInstruction(std::move(longToIntInst));
      dst = intTemp;
    }
    currentValue = dst;
    currentExpResult = ExpResult::makePlainOperand(currentValue);
    return;
  } else if ((effectiveInnerType.kind == TypeKind::UINT ||
              effectiveInnerType.kind == TypeKind::ULONG) &&
             effectiveTargetType.kind == TypeKind::DOUBLE) {
    // Unsigned Int/Long to Double
    if (effectiveInnerType.kind == TypeKind::UINT) {
      // First convert Long to Int
      IRValuePtr longTemp = makeTackyVariable(Type::ULong());
      auto intToLongInst =
          IRInstructionNode::makeSignExtend(std::move(result), longTemp);
      currentFunction->addInstruction(std::move(intToLongInst));
      result = longTemp;
    }
    auto longToDoubleInst =
        IRInstructionNode::makeLongToDouble(std::move(result), dst);
    currentFunction->addInstruction(std::move(longToDoubleInst));
    currentValue = dst;
    currentExpResult = ExpResult::makePlainOperand(currentValue);
    return;
  }

  if (size(effectiveTargetType.kind) == size(effectiveInnerType.kind)) {
    // Same size cast - use copy
    auto copyInst = IRInstructionNode::makeCopy(
        std::move(result), std::make_shared<IRValueNode>(*dst));
    currentFunction->addInstruction(std::move(copyInst));
  } else if (size(effectiveTargetType.kind) < size(effectiveInnerType.kind)) {
    // Truncation
    auto truncInst = IRInstructionNode::makeTruncate(std::move(result), dst);
    currentFunction->addInstruction(std::move(truncInst));
  } else if (effectiveInnerType.kind == TypeKind::INT ||
             effectiveInnerType.kind == TypeKind::LONG ||
             effectiveInnerType.kind == TypeKind::DOUBLE) {
    // Sign extension
    auto signExtInst =
        IRInstructionNode::makeSignExtend(std::move(result), dst);
    currentFunction->addInstruction(std::move(signExtInst));
  } else {
    // Zero extension
    auto zeroExtInst =
        IRInstructionNode::makeZeroExtend(std::move(result), dst);
    currentFunction->addInstruction(std::move(zeroExtInst));
  }
  currentValue = dst;
  currentExpResult = ExpResult::makePlainOperand(currentValue);
}

void IRGenerator::visit(DereferenceExpression &node) {
  // Process the pointer expression and perform lvalue conversion
  if (node.pointerExpr) {
    node.pointerExpr->accept(*this);
    IRValuePtr ptr = currentValue;

    // Store as dereferenced pointer in currentExpResult
    // The actual Load will happen during lvalue conversion if needed
    currentExpResult = ExpResult::makeDereferencedPointer(ptr);
    // For compatibility, also store in currentValue (will be loaded if needed)
    currentValue = ptr;
  }
}

void IRGenerator::visit(AddressOfExpression &node) {
  if (node.variableExpr) {
    // Process the inner expression WITHOUT lvalue conversion
    // We need to check if it's a dereference or a regular variable
    auto *derefExpr =
        dynamic_cast<DereferenceExpression *>(node.variableExpr.get());

    if (derefExpr) {
      // &*ptr => just ptr (cancel out dereference)
      if (derefExpr->pointerExpr) {
        derefExpr->pointerExpr->accept(*this);
        // Result is already the pointer, no GetAddress needed
        currentExpResult = ExpResult::makePlainOperand(currentValue);
      }
    } else {
      // Regular variable or other expression - use GetAddress
      node.variableExpr->accept(*this);

      // Check if result is a dereferenced pointer or plain variable
      if (currentExpResult.type == ExpResultType::DEREFERENCED_POINTER) {
        // Taking address of something already dereferenced - just return the
        // pointer
        currentValue = currentExpResult.value;
        currentExpResult = ExpResult::makePlainOperand(currentValue);
      } else {
        // Plain variable - emit GetAddress instruction
        IRValuePtr src = currentValue;
        IRValuePtr dst = makeTackyVariable(*node.type);
        auto getAddrInst = IRInstructionNode::makeGetAddress(src, dst);
        currentFunction->addInstruction(getAddrInst);
        currentValue = dst;
        currentExpResult = ExpResult::makePlainOperand(dst);
      }
    }
  }
}
void IRGenerator::visit(StringLiteralExpression &node) {
  // Check if we're in a function (expression context) or not (static initializer)
  if (!currentFunction) {
    // String literal in static context - already handled by semantic analyzer
    return;
  }
  
  // Generate a unique identifier for this string literal
  std::string stringName = generateStringName();

  // Calculate the size including null terminator
  int arraySize = node.value.length() + 1;

  // Create the type for the string (array of char)
  Type stringType =
      Type::Array(std::make_shared<Type>(Type::Char()), arraySize);

  // Add the string to the global symbol table as a constant
  SymbolTableEntry entry;
  entry.name = stringName;
  entry.type = stringType;
  entry.symbolType = SymbolType::CONSTANT;  // Mark as CONSTANT not VARIABLE
  entry.storageClass = StorageClass::STATIC;
  entry.linkage = LinkageType::INTERNAL;
  entry.initType = InitType::INITIALIZED;
  entry.stringValue = node.value;  // Store the actual string value

  global_symbol_table[stringName] = entry;

  // Generate GetAddress instruction to load the string's address
  IRValuePtr stringVar = IRValueNode::makeVariable(stringName);
  IRValuePtr dst = createTemporary();

  auto getAddrInst =
      IRInstructionNode::makeGetAddress(std::move(stringVar), dst);
  currentFunction->addInstruction(std::move(getAddrInst));

  // Set the current value to the address we just loaded
  currentValue = dst;
  currentExpResult = ExpResult::makePlainOperand(dst);
}
void IRGenerator::visit(SizeofExpression &node) {
  // Get the type of the inner expression WITHOUT evaluating it
  // We just need the type information, not the runtime value
  auto exprNode = dynamic_cast<ExpressionNode *>(node.expr.get());
  if (!exprNode || !exprNode->type) {
    return;
  }
  
  // Calculate the size in bytes at compile time
  int sizeInBytes = getTypeSize(*exprNode->type);
  
  // Return as an unsigned long constant (size_t)
  currentValue = IRValueNode::makeConstant(static_cast<unsigned long>(sizeInBytes));
  currentExpResult = ExpResult::makePlainOperand(currentValue);
}

void IRGenerator::visit(SizeofTypeExpression &node) {
  // Calculate the size in bytes at compile time
  int sizeInBytes = getTypeSize(*node.typeOperand);
  
  // Return as an unsigned long constant (size_t)
  currentValue = IRValueNode::makeConstant(static_cast<unsigned long>(sizeInBytes));
  currentExpResult = ExpResult::makePlainOperand(currentValue);
}
void IRGenerator::visit(ForInit &node) { (void)node; /* TODO: Implement for loops */ }
void IRGenerator::visit(InitDecl &node) { (void)node; /* TODO: Implement declarations */ }
void IRGenerator::visit(InitExp &node) { (void)node; /* TODO: Implement expressions */ }
void IRGenerator::visit(Ident &node) { (void)node; /* Not needed for basic IR generation */ }
void IRGenerator::visit(
    DeclaratorNode &node) { (void)node; /* Not needed for basic IR generation */ }
void IRGenerator::visit(
    PointerDeclarator &node) { (void)node; /* Not needed for basic IR generation */ }
void IRGenerator::visit(
    FunDeclarator &node) { (void)node; /* Not needed for basic IR generation */ }
void IRGenerator::visit(
    paraminfo &node) { (void)node; /* Not needed for basic IR generation */ }
void IRGenerator::visit(AbstractPointer &node) { (void)node; }
void IRGenerator::visit(AbstractBase &node) { (void)node; }
void IRGenerator::visit(Type &node) { (void)node; /* Not needed for basic IR generation */ }
void IRGenerator::visit(
    DeclarationNode &node) { (void)node; /* Handle declarations if needed */ }
void IRGenerator::visit(NullStatement &node) { (void)node; /* Nothing to do */ }
void IRGenerator::visit(ArrayDeclarator &node) { (void)node; }
void IRGenerator::visit(AbstractArray &node) { (void)node; }
void IRGenerator::visit(InitializerNode &node) {
  if (node.kind == InitializerKind::SINGLE_INIT) {
    auto &singleInit = std::get<SingleInit>(node.data);
    if (singleInit.expression) {
      singleInit.expression->accept(*this);
    }
  } else if (node.kind == InitializerKind::COMPOUND_INIT) {
    auto &initList = std::get<CompoundInit>(node.data);
    for (auto &init : initList.initializers) {
      init.accept(*this);
    }
  }
}
void IRGenerator::visit(SubscriptExpression &node) {
  // Process array expression
  if (node.arrayExpr) {
    node.arrayExpr->accept(*this);
  }
  auto arrayExprNode = dynamic_cast<ExpressionNode *>(node.arrayExpr.get());
  if (!arrayExprNode || !arrayExprNode->type) {
    return;
  }
  IRValuePtr arrayValue =
      convertExpResult(currentExpResult, *arrayExprNode->type);

  // Process index expression
  if (node.indexExpr) {
    node.indexExpr->accept(*this);
  }
  auto indexExprNode = dynamic_cast<ExpressionNode *>(node.indexExpr.get());
  if (!indexExprNode || !indexExprNode->type) {
    return;
  }
  IRValuePtr indexValue =
      convertExpResult(currentExpResult, *indexExprNode->type);

  // Determine which operand is the pointer and which is the index
  IRValuePtr ptrValue;
  IRValuePtr offsetValue;
  
  if (arrayExprNode->type->kind == TypeKind::POINTER) {
    // arrayExpr is the pointer, indexExpr is the offset
    ptrValue = std::move(arrayValue);
    offsetValue = std::move(indexValue);
  } else if (indexExprNode->type->kind == TypeKind::POINTER) {
    // indexExpr is the pointer, arrayExpr is the offset (e.g., idx[arr])
    ptrValue = std::move(indexValue);
    offsetValue = std::move(arrayValue);
  } else {
    // Neither is a pointer - error case
    return;
  }

  // Calculate the scale (size of element type)
  // node.type is the type of the result (the element type)
  int scale = getTypeSize(*node.type);

  // Generate AddPtr instruction: result = ptr + (offset * scale)
  IRValuePtr result = createTemporary();
  auto addPtrInst = IRInstructionNode::makeAddPtr(
      std::move(ptrValue), std::move(offsetValue), scale, result);
  currentFunction->addInstruction(std::move(addPtrInst));

  // Return DereferencedPointer since subscripting is equivalent to *(ptr +
  // index)
  currentExpResult = ExpResult::makeDereferencedPointer(result);
  currentValue = result;
}

void IRGenerator::visit(StructDeclarationNode &node) { (void)node; }

void IRGenerator::visit(MemberDeclarationNode &node) { (void)node; }

void IRGenerator::visit(DotExpression &node) { (void)node; }

void IRGenerator::visit(ArrowExpression &node) { (void)node; }