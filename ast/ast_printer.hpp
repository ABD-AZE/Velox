#pragma once
#include "ast.hpp"
#include <iostream>
#include <string>

class ASTPrinter : public ASTVisitor {
public:
  // Main entry point - static method for easy usage
  static void print(const ASTNodePtr &node, int indent = 0);

  // Constructor for visitor instance
  ASTPrinter(int indent = 0) : indent_(indent) {}

  // Visitor pattern methods
  void visit(ProgramNode &node) override;
  void visit(FunctionDefinitionNode &node) override;
  void visit(BlockItemNode &node) override;
  void visit(DeclarationNode &node) override;

  // Statement visitors
  void visit(ReturnStatement &node) override;
  void visit(NullStatement &node) override;
  void visit(ExpressionStatement &node) override;

  // Expression visitors
  void visit(BinaryExpression &node) override;
  void visit(UnaryExpression &node) override;
  void visit(ConstantExpression &node) override;
  void visit(VariableExpression &node) override;
  void visit(AssignmentExpression &node) override;
  void visit(PostfixExpression &node) override;

private:
  int indent_ = 0;

  void printIndent();
  void increaseIndent() { indent_++; }
  void decreaseIndent() { indent_--; }
};