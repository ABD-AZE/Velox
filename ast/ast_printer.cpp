#include "ast_printer.hpp"

void ASTPrinter::print(const ASTNodePtr &node, int indent) {
  if (!node) {
    return;
  }

  // Create visitor instance and use visitor pattern
  ASTPrinter printer(indent);
  node->accept(printer);
}

void ASTPrinter::printIndent() {
  for (int i = 0; i < indent_; ++i) {
    std::cout << "  "; // 2 spaces per indent level
  }
}

void ASTPrinter::visit(ProgramNode &node) {
  printIndent();
  std::cout << "Program(" << std::endl;

  // Print the function declarations
  if (node.Declarations.size() > 0) {
    increaseIndent();
    for (const auto &funcDecl : node.Declarations) {
      funcDecl->accept(*this);
    }
    decreaseIndent();
  }

  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(FunctionDefinitionNode &node) {
  printIndent();
  std::cout << "Function(" << std::endl;

  // Print function name
  increaseIndent();
  printIndent();
  std::cout << "name=\"" << node.name << "\"," << std::endl;

  // Print function body
  if (node.body) {
    increaseIndent();
    node.body->accept(*this);
    decreaseIndent();
  }

  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(ReturnStatement &node) {
  printIndent();
  std::cout << "Return(" << std::endl;
  if (node.expression) {
    increaseIndent();
    node.expression->accept(*this);
    decreaseIndent();
  }
  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(NullStatement &) {
  printIndent();
  std::cout << "NullStatement()" << std::endl;
}

void ASTPrinter::visit(ExpressionStatement &node) {
  printIndent();
  std::cout << "ExpressionStatement(" << std::endl;
  if (node.expression) {
    increaseIndent();
    node.expression->accept(*this);
    decreaseIndent();
  }
  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(IfStatement &node) {
  printIndent();
  std::cout << "If(" << std::endl;
  increaseIndent();
  printIndent();
  std::cout << "condition=" << std::endl;
  if (node.condition) {
    increaseIndent();
    node.condition->accept(*this);
    decreaseIndent();
  }
  printIndent();
  std::cout << "then=" << std::endl;
  if (node.thenBranch) {
    increaseIndent();
    node.thenBranch->accept(*this);
    decreaseIndent();
  }
  if (node.elseBranch.has_value() && node.elseBranch.value()) {
    printIndent();
    std::cout << "else=" << std::endl;
    increaseIndent();
    node.elseBranch.value()->accept(*this);
    decreaseIndent();
  }
  decreaseIndent();
  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(GotoStatement &node) {
  printIndent();
  std::cout << "Goto(label=\"" << node.label << "\")" << std::endl;
}

void ASTPrinter::visit(LabelStatement &node) {
  printIndent();
  std::cout << "Label(label=\"" << node.label << "\")" << std::endl;
}

void ASTPrinter::visit(CompoundStatement &node) {
  printIndent();
  std::cout << "CompoundStatement(" << std::endl;
  if (node.block) {
    increaseIndent();
    node.block->accept(*this);
    decreaseIndent();
  }
  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(BinaryExpression &node) {
  printIndent();
  std::cout << "Binary(" << std::endl;
  increaseIndent();
  printIndent();
  std::cout << "op=" << TokenTypeToString(node.op) << "," << std::endl;
  printIndent();
  std::cout << "left=" << std::endl;
  if (node.left) {
    increaseIndent();
    node.left->accept(*this);
    decreaseIndent();
  }
  printIndent();
  std::cout << "right=" << std::endl;
  if (node.right) {
    increaseIndent();
    node.right->accept(*this);
    decreaseIndent();
  }
  decreaseIndent();
  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(UnaryExpression &node) {
  printIndent();
  std::cout << "Unary(" << std::endl;
  increaseIndent();
  printIndent();
  std::cout << "op=" << TokenTypeToString(node.op) << "," << std::endl;
  printIndent();
  std::cout << "operand=" << std::endl;
  if (node.operand) {
    increaseIndent();
    node.operand->accept(*this);
    decreaseIndent();
  }
  decreaseIndent();
  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(ConstantExpression &node) {
  printIndent();
  std::cout << "Constant(\n";
  increaseIndent();
  printIndent();
  std::visit([](const auto& value) {
    std::cout << "value: " << value << ", "<< "type: "<< typeid(value).name();
  }, node.value);
  decreaseIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(VariableExpression &node) {
  printIndent();
  std::cout << "Variable(" << node.identifier << ")" << std::endl;
}

void ASTPrinter::visit(AssignmentExpression &node) {
  printIndent();
  std::cout << "Assignment(" << std::endl;
  increaseIndent();
  printIndent();
  std::cout << "left=" << std::endl;
  if (node.left) {
    increaseIndent();
    node.left->accept(*this);
    decreaseIndent();
  }
  printIndent();
  std::cout << "right=" << std::endl;
  if (node.right) {
    increaseIndent();
    node.right->accept(*this);
    decreaseIndent();
  }
  if (node.type) {
    printIndent();
    std::cout << "type=" << std::endl;
    increaseIndent();
    printIndent();
    std::cout << TokenTypeToString(node.type) << std::endl;
    decreaseIndent();
  }
  decreaseIndent();
  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(PostfixExpression &node) {
  printIndent();
  std::cout << "Postfix(" << std::endl;
  increaseIndent();
  printIndent();
  std::cout << "op=" << TokenTypeToString(node.op) << "," << std::endl;
  printIndent();
  std::cout << "operand=" << std::endl;
  if (node.operand) {
    increaseIndent();
    node.operand->accept(*this);
    decreaseIndent();
  }
  decreaseIndent();
  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(ConditionalExpression &node) {
  printIndent();
  std::cout << "Conditional(" << std::endl;
  increaseIndent();
  printIndent();
  std::cout << "condition=" << std::endl;
  if (node.condition) {
    increaseIndent();
    node.condition->accept(*this);
    decreaseIndent();
  }
  printIndent();
  std::cout << "trueExp=" << std::endl;
  if (node.trueExpr) {
    increaseIndent();
    node.trueExpr->accept(*this);
    decreaseIndent();
  }
  printIndent();
  std::cout << "falseExp=" << std::endl;
  if (node.falseExpr) {
    increaseIndent();
    node.falseExpr->accept(*this);
    decreaseIndent();
  }
  decreaseIndent();
  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(DereferenceExpression &node) {
  printIndent();
  std::cout << "Dereference(" << std::endl;
  increaseIndent();
  printIndent();
  std::cout << "pointerExpr=" << std::endl;
  if (node.pointerExpr) {
    increaseIndent();
    node.pointerExpr->accept(*this);
    decreaseIndent();
  }
  decreaseIndent();
  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(AddressOfExpression &node) {
  printIndent();
  std::cout << "AddressOf(" << std::endl;
  increaseIndent();
  printIndent();
  std::cout << "variableExpr=" << std::endl;
  if (node.variableExpr) {
    increaseIndent();
    node.variableExpr->accept(*this);
    decreaseIndent();
  }
  decreaseIndent();
  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(BlockItemNode &node) {
  printIndent();
  std::cout << "BlockItem(" << std::endl;

  if (node.block_item) {
    increaseIndent();
    node.block_item->accept(*this);
    decreaseIndent();
  }

  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(DeclarationNode &node) {
  printIndent();
  std::cout << "Declaration(" << std::endl;
  if (node.declaration) {
    increaseIndent();
    node.declaration->accept(*this);
    decreaseIndent();
  }
  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(BlockNode &node) {
  printIndent();
  std::cout << "Block(" << std::endl;

  // Print block items
  increaseIndent();
  for (const auto &item : node.block_items) {
    if (item) {
      item->accept(*this);
    }
  }
  decreaseIndent();

  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(InitDecl &node) {
  printIndent();
  std::cout << "InitDecl(" << std::endl;
  if (node.init) {
    increaseIndent();
    node.init->accept(*this);
    decreaseIndent();
  }
  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(InitExp &node) {
  printIndent();
  std::cout << "InitExp(" << std::endl;
  if (node.init) {
    increaseIndent();
    node.init.value()->accept(*this);
    decreaseIndent();
  }
  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(ForInit &node) {
  printIndent();
  std::cout << "ForInit(" << std::endl;
  if (node.init) {
    increaseIndent();
    node.init->accept(*this);
    decreaseIndent();
  }
  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(BreakNode &node) {
  printIndent();
  std::cout << "Break(label=\"" << node.label << "\")" << std::endl;
}

void ASTPrinter::visit(ContinueNode &node) {
  printIndent();
  std::cout << "Continue(label=\"" << node.label << "\")" << std::endl;
}

void ASTPrinter::visit(WhileNode &node) {
  printIndent();
  std::cout << "While(" << std::endl;
  increaseIndent();
  printIndent();
  std::cout << "condition=" << std::endl;
  if (node.condition) {
    increaseIndent();
    node.condition->accept(*this);
    decreaseIndent();
  }
  printIndent();
  std::cout << "body=" << std::endl;
  if (node.body) {
    increaseIndent();
    node.body->accept(*this);
    decreaseIndent();
  }
  decreaseIndent();
  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(DoWhileNode &node) {
  printIndent();
  std::cout << "DoWhile(" << std::endl;
  increaseIndent();
  printIndent();
  std::cout << "body=" << std::endl;
  if (node.body) {
    increaseIndent();
    node.body->accept(*this);
    decreaseIndent();
  }
  printIndent();
  std::cout << "condition=" << std::endl;
  if (node.condition) {
    increaseIndent();
    node.condition->accept(*this);
    decreaseIndent();
  }
  decreaseIndent();
  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(ForNode &node) {
  printIndent();
  std::cout << "For(" << std::endl;
  increaseIndent();
  printIndent();
  std::cout << "init=" << std::endl;
  if (node.init) {
    increaseIndent();
    node.init->accept(*this);
    decreaseIndent();
  }
  printIndent();
  std::cout << "condition=" << std::endl;
  if (node.condition) {
    increaseIndent();
    node.condition.value()->accept(*this);
    decreaseIndent();
  }
  printIndent();
  std::cout << "post=" << std::endl;
  if (node.post) {
    increaseIndent();
    node.post.value()->accept(*this);
    decreaseIndent();
  }
  printIndent();
  std::cout << "body=" << std::endl;
  if (node.body) {
    increaseIndent();
    node.body->accept(*this);
    decreaseIndent();
  }
  decreaseIndent();
  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(FunDeclNode &node) {
  printIndent();
  std::cout << "FunDecl(" << std::endl;

  // Print function name
  increaseIndent();
  printIndent();
  std::cout << "name=\"" << node.name << "\"," <<"type="<< std::endl;
  printIndent();
  increaseIndent();
  node.type.accept(*this);
  decreaseIndent();

  // Print parameters and storage class specifier if present
  if(node.storage_class.has_value()) {
    printIndent();
    std::cout << "storage_class=" << TokenTypeToString(node.storage_class.value()) << "," << std::endl;
  }
  printIndent();
  std::cout << "parameters=[" << std::endl;
  if (node.params.size() > 0) {
    increaseIndent();
    for (const auto &param : node.params) {
      printIndent();
      std::cout << "\"" << param << "\"," << std::endl;
    }
    decreaseIndent();
  }
  printIndent();
  std::cout << "]," << std::endl;
  
  // Print body if present
  if (node.body) {
    printIndent();
    std::cout << "body=" << std::endl;
    increaseIndent();
    node.body.value()->accept(*this);
    decreaseIndent();
  } else {
    printIndent();
    std::cout << "body=null" << std::endl;
  }
  decreaseIndent();
  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(VarDeclNode &node) {
  printIndent();
  std::cout << "VarDecl(" << std::endl;

  // Print variable name
  increaseIndent();
  printIndent();
  std::cout << "name=\"" << node.name << "\"," <<"type="<< std::endl;
  printIndent();
  increaseIndent();
  node.type.accept(*this);
  decreaseIndent();

  if (node.storage_class.has_value()) {
    printIndent();
    std::cout << "storage_class=" << TokenTypeToString(node.storage_class.value()) <<"," <<std::endl;
  }
  // Print initializer and storage class specifier if present
  if (node.init.has_value()) {
    printIndent();
    std::cout << "init="<<std::endl;
    increaseIndent();
    node.init.value()->accept(*this);
    decreaseIndent();
  } else {
    std::cout << std::endl;
  }
  
  decreaseIndent();

  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(FunctionCallNode &node) {
  printIndent();
  std::cout << "FunctionCall(" << std::endl;

  // Print function name
  increaseIndent();
  printIndent();
  std::cout << "name=\"" << node.name << "\"," << std::endl;

  // Print arguments
  printIndent();
  std::cout << "arguments=[" << std::endl;
  if (node.args.size() > 0) {
    increaseIndent();
    for (const auto &arg : node.args) {
      arg->accept(*this);
    }
    decreaseIndent();
  }
  printIndent();
  std::cout << "]" << std::endl;

  decreaseIndent();
  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(Type &node) {
  printIndent();
  std::cout << "Type(" << std::endl;
  increaseIndent();
  printIndent();
  std::cout << "kind=" << TypeKindToString(node.kind) << std::endl;
  decreaseIndent();
  printIndent();
  std::cout << ")" << std::endl;
}

void ASTPrinter::visit(CastExpression &node) {
  printIndent();
  std::cout << "Cast(" << std::endl;
  increaseIndent();
  printIndent();
  std::cout << "targetType=" << std::endl;
  increaseIndent();
  node.targetType.accept(*this);
  decreaseIndent();
  printIndent();
  std::cout << "expression=" << std::endl;
  if (node.expression) {
    increaseIndent();
    node.expression->accept(*this);
    decreaseIndent();
  }
  decreaseIndent();
  printIndent();
  std::cout << ")" << std::endl;
}