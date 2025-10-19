#include "valor.hpp"
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
  default:
    ss << "unknown_op";
    break;
  }

  return ss.str();
}

// IRFunctionNode implementation
std::string IRFunctionNode::toString() const {
  std::stringstream ss;
  ss << "function " << identifier << "() {\n";
  for (const auto &instruction : instructions) {
    if(instruction->opType == IROpType::LABEL){
      ss << instruction->toString()<< ":\n";
      continue;
    }
    ss << "    " << instruction->toString() << "\n";
  }
  ss << "}\n";
  return ss.str();
}

// IRProgramNode implementation
std::string IRProgramNode::toString() const {
  std::stringstream ss;
  for (const auto &function : functions) {
    ss << function->toString() << "\n";
  }
  return ss.str();
}

// IRGenerator implementation
IRProgramPtr IRGenerator::generateIR(const ASTNodePtr &ast) {
  program = std::make_unique<IRProgramNode>();
  tempCounter = 0;
  labelCounter = 0;

  ast->accept(*this);

  return std::move(program);
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
    auto jumpInst = IRInstructionNode::makeJumpIfZero(
        std::move(conditionValue), elseLabel);
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

    // Create temporary for result
    IRValuePtr result = createTemporary();

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

    // Create postfix instruction
    auto inst = IRInstructionNode::makeBinary(
        irOp, result, std::move(operand), std::move(one));
    currentFunction->addInstruction(std::move(inst));

    // Update current value to the result
    currentValue = result;
  }

}
void IRGenerator::visit(
    ConditionalExpression &node) { 
    if (node.condition) {
    // Generate IR for condition
    node.condition->accept(*this);
    IRValuePtr conditionValue = std::make_shared<IRValueNode>(*currentValue);

    // Generate labels for branching
    std::string falseLabel = generateLabelName();
    std::string endLabel = generateLabelName();

    // Create jump instruction based on condition
    auto jumpInst = IRInstructionNode::makeJumpIfZero(
        std::move(conditionValue), falseLabel);
    currentFunction->addInstruction(std::move(jumpInst));

    // Generate IR for 'then' block
    if (node.trueExpr) {
      node.trueExpr->accept(*this);
    }
    IRValuePtr trueExprValue = std::make_shared<IRValueNode>(*currentValue);
    // Create a temporary variable to hold the result of the true expression
    IRValuePtr result = createTemporary();
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
        std::move(falseExprValue), std::make_shared<IRValueNode>(result));;
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
  IRValuePtr var = IRValueNode::makeVariable(node.name);
  // If there's an initializer, generate IR for it
  if (node.init) {
    (*node.init)->accept(*this);
    IRValuePtr initValue = std::make_shared<IRValueNode>(*currentValue);
    // Create copy instruction to assign initializer value to variable
    auto copyInst =
        IRInstructionNode::makeCopy(std::move(initValue), std::move(var));
    if (currentFunction) {
      currentFunction->addInstruction(std::move(copyInst));
    }
  }
}

void IRGenerator::visit(FunDeclNode &node) {
  std::shared_ptr<IRFunctionNode> func = std::make_shared<IRFunctionNode>();
  func->identifier = node.name;
  currentFunction = func;
  // Generate IR for function body
  if (node.body) {
    (*node.body)->accept(*this);
  }
  func->addInstruction(
      IRInstructionNode::makeReturn(IRValueNode::makeConstant(0))); // Ensure function ends with return
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
  int intValue = std::visit(
      [](auto &&arg) -> int { return static_cast<int>(arg); }, node.value);
  currentValue = IRValueNode::makeConstant(intValue);
}

void IRGenerator::visit(VariableExpression &node) {
  // Create a variable reference
  currentValue = IRValueNode::makeVariable(node.identifier);
}

void IRGenerator::visit(UnaryExpression &node) {
  // Generate IR for operand
  node.operand->accept(*this);
  IRValuePtr operand = std::make_shared<IRValueNode>(*currentValue);

  // Create temporary for result
  IRValuePtr result = createTemporary();

  // Convert token type to IR operation
  IROpType irOp = tokenTypeToUnaryIR(node.op);

  // Create unary instruction
  auto inst = IRInstructionNode::makeUnary(irOp, result, std::move(operand));
  currentFunction->addInstruction(std::move(inst));

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
    IRValuePtr result = createTemporary();

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
    IRValuePtr result = createTemporary();

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

  // Create temporary for result
  IRValuePtr result = createTemporary();

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

void IRGenerator::visit(
    CompoundStatement &node) {
  node.block->accept(*this);
}

void IRGenerator::visit(BreakNode &node) { 
  IRInstructionPtr breakInst = IRInstructionNode::makeJump("break_" + node.label);
  currentFunction->addInstruction(std::move(breakInst));
}

void IRGenerator::visit(ContinueNode &node) { 
  IRInstructionPtr continueInst = IRInstructionNode::makeJump("continue_" + node.label);
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
  auto continueLabel =  IRInstructionNode::makeLabel("continue_" + node.label);
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
  auto breakLabel = "break_"+node.label;
  auto startInstr = IRInstructionNode::makeLabel(continueLabel);
  currentFunction->addInstruction(std::move(startInstr));

  // condition instructions
  auto conditionValue = std::make_shared<IRValueNode>();
  // always true for while and do while
  if(node.condition){
    node.condition->accept(*this);
    conditionValue = std::make_shared<IRValueNode>(*currentValue);
  }
  IRInstructionPtr jumpInstr;
  jumpInstr = IRInstructionNode::makeJumpIfZero(conditionValue,breakLabel);
  currentFunction->addInstruction(std::move(jumpInstr));
  // body instructions
  if(node.body){
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
  if(node.init){
    node.init->accept(*this);
  }
  // start label
  auto startLabel = generateLabelName();
  auto continueLabel = "continue_"+node.label;
  auto breakLabel = "break_"+node.label;
  auto startLabelInstr = IRInstructionNode::makeLabel(startLabel);
  currentFunction->addInstruction(std::move(startLabelInstr));
  // condition instructions
  IRInstructionPtr jumpInstr;
  IRValuePtr conditionValue;
  // if condition is not present then no need for an additional jump instruction
  if(node.condition){
    (*node.condition)->accept(*this);
    conditionValue = std::make_shared<IRValueNode>(*currentValue);
    jumpInstr = IRInstructionNode::makeJumpIfZero(conditionValue,breakLabel);
    currentFunction->addInstruction(std::move(jumpInstr));
  }
  // body instructions
  if(node.body){
    node.body->accept(*this);
  }
  // continue label
  auto continueLabelInstr = IRInstructionNode::makeLabel(continueLabel);
  currentFunction->addInstruction(std::move(continueLabelInstr));
  // post instructions
  if(node.post){
    (*node.post)->accept(*this);
  }
  // jump back to start
  auto jumpBackInstr = IRInstructionNode::makeJump(startLabel);
  currentFunction->addInstruction(std::move(jumpBackInstr));
  // break label
  auto breakInstr = IRInstructionNode::makeLabel(breakLabel);
  currentFunction->addInstruction(std::move(breakInstr));
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

// Valor class implementation
IRProgramPtr Valor::convertToIR(const ASTNodePtr &ast) {
  return generator.generateIR(ast);
}


// <------------------------------------------------------------------------------------->
void IRGenerator::visit(GotoStatement &node) { /* TODO: Implement jumps */ }
void IRGenerator::visit(LabelStatement &node) { /* TODO: Implement labels */ }
void IRGenerator::visit(CastExpression &node) { /* TODO: Implement casts */ }
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
void IRGenerator::visit(FunctionCallNode &node) { /* Handle function calls */ }
void IRGenerator::visit(NullStatement &node) { /* Nothing to do */ }
