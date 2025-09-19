#include "ast_printer.hpp"
#include <iostream>

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

  // Print the function definition
  if (node.functionDefinition) {
    increaseIndent();
    node.functionDefinition->accept(*this);
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

  // Print function body (block items)
  printIndent();
  std::cout << "body=[" << std::endl;
  increaseIndent();
  for (const auto &blockItem : node.block_items) {
    if (blockItem) {
      blockItem->accept(*this);
    }
  }
  decreaseIndent();
  printIndent();
  std::cout << "]" << std::endl;
  decreaseIndent();

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
  std::cout << "Constant(" << node.value << ")" << std::endl;
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

  // Print variable name
  increaseIndent();
  printIndent();
  std::cout << "name=\"" << node.name << "\"";

  // Print initializer if present
  if (node.init.has_value() && node.init.value()) {
    std::cout << "," << std::endl;
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