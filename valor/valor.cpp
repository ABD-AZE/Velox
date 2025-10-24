#include "valor.hpp"
#include "../utils/token_classifier.hpp"
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
    ss << dst->toString() << " = " << src1->toString()
       << " >= " << src2->toString();
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
    ss << dst->toString() << " = call " << src1->toString() << ", args: ";
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
     << ", type=" << TypeKindToString(type.kind) << ", init=";

  // Handle the variant type by visiting it
  std::visit([&ss](auto &&arg) { ss << arg; }, initialValue);

  ss << ")";
  return ss.str();
}

// IRProgramNode implementation
std::string IRProgramNode::toString() const {
  std::stringstream ss;
  ss << "Program(\n";
  for (const auto &item : topLevelItems) {
    ss << item->toString();
    // Add newline if it's a static variable (functions already have newlines)
    if (dynamic_cast<IRStaticVariableNode *>(item.get())) {
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

void IRGenerator::convertSymbolTableToIR() {
  // Iterate through the global symbol table
  for (const auto &[name, entry] : global_symbol_table) {
    // Skip if it's not a variable
    if (entry.symbolType != SymbolType::VARIABLE) {
      continue;
    }

    // Skip if it doesn't have static linkage (we only want static variables)
    if (entry.linkage != LinkageType::INTERNAL &&
        entry.linkage != LinkageType::EXTERNAL) {
      continue;
    }

    // Skip if it has no initializer (not defined in this translation unit)
    if (entry.initType == InitType::UNINITIALIZED) {
      continue;
    }

    // Determine if it's global (external linkage) or file-scope (internal
    // linkage)
    bool isGlobal = (entry.linkage == LinkageType::EXTERNAL);

    // Get initial value
    std::variant<int, long int, long unsigned int, unsigned int, double>
        initValue = 0;
    if (entry.initType == InitType::INITIALIZED) {
      initValue = entry.value;
    } else if (entry.initType == InitType::TENTATIVE ||
               entry.initType == InitType::ZERO_INITIALIZED) {
      initValue = 0;
    }

    // Create static variable node
    auto staticVar = std::make_shared<IRStaticVariableNode>(
        name, isGlobal, entry.type, initValue);
    program->addStaticVariable(std::move(staticVar));
  }
}

void IRGenerator::visit(IfStatement &node) {
  if (node.condition) {
    // Generate IR for condition
    node.condition->accept(*this);
    IRValuePtr conditionValue = std::make_shared<IRValueNode>(*currentValue);

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
    // Generate IR for operand
    node.operand->accept(*this);
    IRValuePtr operand = std::make_shared<IRValueNode>(*currentValue);

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
  }
}
void IRGenerator::visit(ConditionalExpression &node) {
  if (node.condition) {
    // Generate IR for condition
    node.condition->accept(*this);
    IRValuePtr conditionValue = std::make_shared<IRValueNode>(*currentValue);

    // Generate labels for branching
    std::string falseLabel = generateLabelName();
    std::string endLabel = generateLabelName();

    // Create jump instruction based on condition
    auto jumpInst = IRInstructionNode::makeJumpIfZero(std::move(conditionValue),
                                                      falseLabel);
    currentFunction->addInstruction(std::move(jumpInst));

    // Generate IR for 'then' block
    if (node.trueExpr) {
      node.trueExpr->accept(*this);
    }
    IRValuePtr trueExprValue = std::make_shared<IRValueNode>(*currentValue);
    // Create a temporary variable to hold the result with proper type tracking
    IRValuePtr result = makeTackyVariable(*node.type);
    // Assign true expression value to result
    auto copyTrueInst = IRInstructionNode::makeCopy(
        std::move(trueExprValue), std::make_shared<IRValueNode>(result));
    currentFunction->addInstruction(std::move(copyTrueInst));

    // Jump to end after 'then' block
    auto jumpToEndInst = IRInstructionNode::makeJump(endLabel);
    currentFunction->addInstruction(std::move(jumpToEndInst));

    // False label
    auto falseLabelInst = IRInstructionNode::makeLabel(falseLabel);
    currentFunction->addInstruction(std::move(falseLabelInst));

    // Generate IR for 'false' block if it exists
    if (node.falseExpr) {
      (node.falseExpr)->accept(*this);
    }

    IRValuePtr falseExprValue = std::make_shared<IRValueNode>(*currentValue);
    // Assign false expression value to result
    auto copyFalseInst = IRInstructionNode::makeCopy(
        std::move(falseExprValue), std::make_shared<IRValueNode>(result));
    ;
    currentFunction->addInstruction(std::move(copyFalseInst));
    // End label
    auto endLabelInst = IRInstructionNode::makeLabel(endLabel);
    currentFunction->addInstruction(std::move(endLabelInst));
    currentValue = result;
  }
}

void IRGenerator::visit(ProgramNode &node) {
  // Process all declarations in the program
  for (const auto &declaration : node.Declarations) {
    declaration->accept(*this);
  }
}

void IRGenerator::visit(FunctionDefinitionNode &node) {
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

  // Local automatic variable - generate IR for it
  IRValuePtr var = IRValueNode::makeVariable(node.name);

  // If there's an initializer, generate IR for it
  if (node.init) {
    (*node.init)->accept(*this);
    IRValuePtr initValue = std::make_shared<IRValueNode>(*currentValue);
    // Create copy instruction to assign initializer value to variable
    auto copyInst =
        IRInstructionNode::makeCopy(std::move(initValue), std::move(var));
    currentFunction->addInstruction(std::move(copyInst));
  }
}

void IRGenerator::visit(FunDeclNode &node) {
  if (!node.body.has_value()) {
    return;
  }

  // Determine if function is global based on storage class
  bool isGlobal = true;
  if (node.storage_class.has_value()) {
    isGlobal = (node.storage_class.value() != TokenType::STATIC);
  }

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
    if (currentValue)
      returnValue = std::make_shared<IRValueNode>(*currentValue);
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
}

void IRGenerator::visit(VariableExpression &node) {
  // Create a variable reference
  currentValue = IRValueNode::makeVariable(node.identifier);
}

void IRGenerator::visit(UnaryExpression &node) {
  // Generate IR for operand
  node.operand->accept(*this);
  IRValuePtr operand = std::make_shared<IRValueNode>(*currentValue);

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

    // Generate IR for left operand
    node.left->accept(*this);
    IRValuePtr leftValue = std::make_shared<IRValueNode>(*currentValue);

    // Jump to false_label if left operand is zero
    auto jumpIfZero1 = IRInstructionNode::makeJumpIfZero(leftValue, falseLabel);
    currentFunction->addInstruction(std::move(jumpIfZero1));

    // Generate IR for right operand
    node.right->accept(*this);
    IRValuePtr rightValue = std::make_shared<IRValueNode>(*currentValue);

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

    // Generate IR for left operand
    node.left->accept(*this);
    IRValuePtr leftValue = std::make_shared<IRValueNode>(*currentValue);

    // Jump to true_label if left operand is non-zero
    auto jumpIfNotZero1 =
        IRInstructionNode::makeJumpIfNotZero(leftValue, trueLabel);
    currentFunction->addInstruction(std::move(jumpIfNotZero1));

    // Generate IR for right operand
    node.right->accept(*this);
    IRValuePtr rightValue = std::make_shared<IRValueNode>(*currentValue);

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
    return;
  }

  // Handle regular binary operations (non-short-circuiting)
  // Generate IR for left operand
  node.left->accept(*this);
  IRValuePtr leftValue = std::make_shared<IRValueNode>(*currentValue);

  // Generate IR for right operand
  node.right->accept(*this);
  IRValuePtr rightValue = std::make_shared<IRValueNode>(*currentValue);

  // Create temporary for result with proper type tracking
  IRValuePtr result = makeTackyVariable(*node.type);

  // Convert token type to IR operation
  IROpType irOp = tokenTypeToBinaryIR(node.op);

  // Create binary instruction
  auto inst = IRInstructionNode::makeBinary(irOp, result, std::move(leftValue),
                                            std::move(rightValue));
  currentFunction->addInstruction(std::move(inst));

  currentValue = std::move(result);
}

void IRGenerator::visit(AssignmentExpression &node) {
  // Generate IR for right side (value being assigned)
  node.right->accept(*this);
  IRValuePtr rightValue = std::make_shared<IRValueNode>(*currentValue);

  // Generate IR for left side (variable being assigned to)
  node.left->accept(*this);
  IRValuePtr leftValue = std::make_shared<IRValueNode>(*currentValue);

  // Create copy instruction (assignment)
  auto copyInst = IRInstructionNode::makeCopy(rightValue, leftValue);
  currentFunction->addInstruction(std::move(copyInst));

  // Assignment result is the assigned value
  currentValue = std::move(rightValue);
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
    conditionValue = std::make_shared<IRValueNode>(*currentValue);
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
    conditionValue = std::make_shared<IRValueNode>(*currentValue);
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
    conditionValue = std::make_shared<IRValueNode>(*currentValue);
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

  // Create temporary for result with proper type tracking
  IRValuePtr result = makeTackyVariable(*node.type);

  // Create call instruction
  auto callInst = IRInstructionNode::makeCall(
      funcValue, IRValueNode::makeArgs(argValues), result);
  currentFunction->addInstruction(std::move(callInst));

  currentValue = std::move(result);
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

// Valor class implementation
IRProgramPtr Valor::convertToIR(const ASTNodePtr &ast) {
  return generator.generateIR(ast);
}

// <------------------------------------------------------------------------------------->
void IRGenerator::visit(GotoStatement &node) { /* TODO: Implement jumps */ }
void IRGenerator::visit(LabelStatement &node) { /* TODO: Implement labels */ }

void IRGenerator::visit(CastExpression &node) {
  // Generate IR for the inner expression
  node.expression->accept(*this);
  IRValuePtr result = std::make_shared<IRValueNode>(*currentValue);

  // Get the type we're casting from (the inner expression's type)
  auto exp = dynamic_cast<ExpressionNode *>(node.expression.get());
  Type innerType = *exp->type;

  // If already the correct type, no cast needed
  if (node.targetType == innerType) {
    currentValue = result;
    return;
  }

  // Create destination variable with the target type
  IRValuePtr dst = makeTackyVariable(node.targetType);

  if (node.targetType.kind == TypeKind::DOUBLE &&
      (innerType.kind == TypeKind::INT || innerType.kind == TypeKind::LONG)) {
    // Int/Long to Double
    if (innerType.kind == TypeKind::INT) {
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
    return;
  } else if ((node.targetType.kind == TypeKind::INT ||
              node.targetType.kind == TypeKind::LONG) &&
             innerType.kind == TypeKind::DOUBLE) {
    // Double to Int/Long
    auto doubleToLongInst =
        IRInstructionNode::makeDoubleToLong(std::move(result), dst);
    currentFunction->addInstruction(std::move(doubleToLongInst));
    if (node.targetType.kind == TypeKind::INT) {
      // Then convert Int to Long
      IRValuePtr intTemp = makeTackyVariable(Type::Int());
      auto longToIntInst = IRInstructionNode::makeSignExtend(
          std::make_shared<IRValueNode>(*dst), intTemp);
      currentFunction->addInstruction(std::move(longToIntInst));
      dst = intTemp;
    }
    currentValue = dst;
    return;
  } else if (innerType.kind == TypeKind::DOUBLE &&
             (node.targetType.kind == TypeKind::UINT ||
              node.targetType.kind == TypeKind::ULONG)) {
    // Double to Unsigned Int/Long
    auto doubleToLongInst =
        IRInstructionNode::makeDoubleToLong(std::move(result), dst);
    currentFunction->addInstruction(std::move(doubleToLongInst));
    if (node.targetType.kind == TypeKind::UINT) {
      // Then convert Int to Long
      IRValuePtr intTemp = makeTackyVariable(Type::UInt());
      auto longToIntInst = IRInstructionNode::makeTruncate(
          std::make_shared<IRValueNode>(*dst), intTemp);
      currentFunction->addInstruction(std::move(longToIntInst));
      dst = intTemp;
    }
    currentValue = dst;
    return;
  } else if ((innerType.kind == TypeKind::UINT ||
              innerType.kind == TypeKind::ULONG) &&
             node.targetType.kind == TypeKind::DOUBLE) {
    // Unsigned Int/Long to Double
    if (innerType.kind == TypeKind::UINT) {
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
    return;
  }

  if (size(node.targetType.kind) == size(exp->type->kind)) {
    // Same size cast - use copy
    auto copyInst = IRInstructionNode::makeCopy(
        std::move(result), std::make_shared<IRValueNode>(*dst));
    currentFunction->addInstruction(std::move(copyInst));
  } else if (size(node.targetType.kind) < size(exp->type->kind)) {
    // Truncation
    auto truncInst = IRInstructionNode::makeTruncate(std::move(result), dst);
    currentFunction->addInstruction(std::move(truncInst));
  } else if (exp->type->kind == TypeKind::INT ||
             exp->type->kind == TypeKind::LONG ||
             exp->type->kind == TypeKind::DOUBLE) {
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
}

void IRGenerator::visit(
    DereferenceExpression &node) { /* TODO: Implement dereference */ }
void IRGenerator::visit(
    AddressOfExpression &node) { /* TODO: Implement address-of */ }
void IRGenerator::visit(ForInit &node) { /* TODO: Implement for loops */ }
void IRGenerator::visit(InitDecl &node) { /* TODO: Implement declarations */ }
void IRGenerator::visit(InitExp &node) { /* TODO: Implement expressions */ }
void IRGenerator::visit(Ident &node) { /* Not needed for basic IR generation */
}
void IRGenerator::visit(
    DeclaratorNode &node) { /* Not needed for basic IR generation */ }
void IRGenerator::visit(
    PointerDeclarator &node) { /* Not needed for basic IRgeneration */ }
void IRGenerator::visit(
    FunDeclarator &node) { /* Not needed for basic IR generation */ }
void IRGenerator::visit(
    paraminfo &node) { /* Not needed for basic IR generation */ }
void IRGenerator::visit(AbstractPointer &node) {}
void IRGenerator::visit(AbstractBase &node) {}
void IRGenerator::visit(Type &node) { /* Not needed for basic IR generation */ }
void IRGenerator::visit(
    DeclarationNode &node) { /* Handle declarations if needed */ }
void IRGenerator::visit(NullStatement &node) { /* Nothing to do */ }
void IRGenerator::visit(ArrayDeclarator &node) { (void)node; }
void IRGenerator::visit(AbstractArray &node) { (void)node; }
void IRGenerator::visit(InitializerNode &node) { 
   if(node.kind == InitializerKind::SINGLE_INIT) {
       auto &singleInit = std::get<SingleInit>(node.data);
       if(singleInit.expression) {
           singleInit.expression->accept(*this);
       }
   } else if(node.kind == InitializerKind::COMPOUND_INIT) {
       auto &initList = std::get<CompoundInit>(node.data);
       for(auto &init : initList.initializers) {
          init.accept(*this);
       }
    }
}
void IRGenerator::visit(SubscriptExpression &node) { 
  if (node.arrayExpr)
  {
    node.arrayExpr->accept(*this);
  }
  if (node.indexExpr)
  {
    node.indexExpr->accept(*this);
  }
}