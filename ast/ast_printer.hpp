#pragma once
#include "ast.hpp"
#include "../utils/token_classifier.hpp"
#include <iostream>
#include <string>
#include <typeinfo>

class ASTPrinter : public ASTVisitor {
public:
  // Main entry point - static method for easy usage
  static void print(const ASTNodePtr &node, int indent = 0);

  // Constructor for visitor instance
  ASTPrinter(int indent = 0) : indent_(indent) {}


  // enum classes
  void visit(Type &node) override;
  // Visitor pattern methods
  void visit(ProgramNode &node) override;
  void visit(FunctionDefinitionNode &node) override;
  void visit(BlockItemNode &node) override;
  void visit(DeclarationNode &node) override;
  void visit(BlockNode &node) override;
  void visit(FunDeclNode &node) override;
  void visit(VarDeclNode &node) override;
  void visit(FunctionCallNode &node) override;

  // Statement visitors
  void visit(ReturnStatement &node) override;
  void visit(NullStatement &node) override;
  void visit(ExpressionStatement &node) override;
  void visit(IfStatement &node) override;
  void visit(GotoStatement &node) override;
  void visit(LabelStatement &node) override;
  void visit(CompoundStatement &node) override;

  // Expression visitors
  void visit(BinaryExpression &node) override;
  void visit(UnaryExpression &node) override;
  void visit(ConstantExpression &node) override;
  void visit(VariableExpression &node) override;
  void visit(AssignmentExpression &node) override;
  void visit(PostfixExpression &node) override;
  void visit(ConditionalExpression &node) override;
  void visit(CastExpression &node) override;
  
  // loop
  void visit(ForInit &node) override;
  void visit(InitDecl &node) override;
  void visit(InitExp &node) override;
  void visit(BreakNode &node) override;
  void visit(ContinueNode &node) override;
  void visit(WhileNode &node) override;
  void visit(DoWhileNode &node) override;
  void visit(ForNode &node) override;

private:
  int indent_ = 0;

  void printIndent();
  void increaseIndent() { indent_++; }
  void decreaseIndent() { indent_--; }
};