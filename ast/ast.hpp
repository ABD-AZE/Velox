#pragma once
#include "../token/token.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

// Forward declarations
class ASTNode;
class ProgramNode;
class FunctionDefinitionNode;
class StatementNode;
class ExpressionNode;
class BlockItemNode;
class DeclarationNode;
class ReturnStatement;
class NullStatement;
class IfStatement;
class GotoStatement;
class LabelStatement;
class ExpressionStatement;
class CompoundStatement;
class BinaryExpression;
class UnaryExpression;
class ConstantExpression;
class VariableExpression;
class AssignmentExpression;
class PostfixExpression;
class ConditionalExpression;
class BlockNode;

using ASTNodePtr = std::unique_ptr<ASTNode>;
using ProgramNodePtr = std::unique_ptr<ProgramNode>;
using FunctionDefinitionNodePtr = std::unique_ptr<FunctionDefinitionNode>;
using StatementNodePtr = std::unique_ptr<StatementNode>;
using ExpressionNodePtr = std::unique_ptr<ExpressionNode>;
using BlockItemNodePtr = std::unique_ptr<BlockItemNode>;
using DeclarationNodePtr = std::unique_ptr<DeclarationNode>;
using BlockNodePtr = std::unique_ptr<BlockNode>;

// Visitor interface to use visitor pattern
class ASTVisitor {
public:
  virtual ~ASTVisitor() = default;

  // Visit methods for each node type
  virtual void visit(ProgramNode &node) = 0;
  virtual void visit(FunctionDefinitionNode &node) = 0;
  virtual void visit(BlockItemNode &node) = 0;
  virtual void visit(DeclarationNode &node) = 0;
  virtual void visit(BlockNode &node) = 0;

  // Statement visitors
  virtual void visit(ReturnStatement &node) = 0;
  virtual void visit(NullStatement &node) = 0;
  virtual void visit(ExpressionStatement &node) = 0;
  virtual void visit(IfStatement &node) = 0;
  virtual void visit(GotoStatement &node) = 0;
  virtual void visit(LabelStatement &node) = 0;
  virtual void visit(CompoundStatement &node) = 0;

  // Expression visitors
  virtual void visit(BinaryExpression &node) = 0;
  virtual void visit(UnaryExpression &node) = 0;
  virtual void visit(ConstantExpression &node) = 0;
  virtual void visit(VariableExpression &node) = 0;
  virtual void visit(AssignmentExpression &node) = 0;
  virtual void visit(PostfixExpression &node) = 0;
  virtual void visit(ConditionalExpression &node) = 0;
};

class ASTNode {
public:
  virtual ~ASTNode() = default;
  virtual void accept(ASTVisitor &visitor) = 0;
};

class ProgramNode : public ASTNode {
public:
  ProgramNode(ASTNodePtr &&functionDefinition)
      : functionDefinition(std::move(functionDefinition)) {}

  ~ProgramNode() override = default;

  // double dispatch to the correct accept method
  void accept(ASTVisitor &visitor) override {
    visitor.visit(*this); // single dispatch to the correct visit method
  }

  ASTNodePtr functionDefinition;
};

class FunctionDefinitionNode : public ASTNode {
public:
  std::string name;
  ASTNodePtr body;

  FunctionDefinitionNode() = default;
  FunctionDefinitionNode(std::string name,
                         ASTNodePtr body)
      : name(std::move(name)), body(std::move(body)) {}

  ~FunctionDefinitionNode() override = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

};

// Base Statement class
class StatementNode : public ASTNode {
public:
  ~StatementNode() override = default;
  virtual void accept(ASTVisitor &visitor) = 0;
};

// Specific Statement types
class ReturnStatement : public StatementNode {
public:
  ASTNodePtr expression;

  ReturnStatement(ASTNodePtr expr)
      : expression(std::move(expr)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class NullStatement : public StatementNode {
public:
  NullStatement() = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ExpressionStatement : public StatementNode {
public:
  ASTNodePtr expression;

  ExpressionStatement(ASTNodePtr expr)
      : expression(std::move(expr)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class IfStatement : public StatementNode {
public:
  ASTNodePtr condition;
  ASTNodePtr thenBranch;
  std::optional<ASTNodePtr> elseBranch; 
  IfStatement(ASTNodePtr cond, ASTNodePtr thenBr,
              std::optional<ASTNodePtr> elseBr = std::nullopt)
      : condition(std::move(cond)), thenBranch(std::move(thenBr)),
        elseBranch(std::move(elseBr)) {}
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class GotoStatement : public StatementNode {
public:
  std::string label;
  GotoStatement(std::string label) : label(std::move(label)) {}
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class LabelStatement : public StatementNode {
public:
  std::string label;
  LabelStatement(std::string label) : label(std::move(label)) {}
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};  

class CompoundStatement : public StatementNode {
public:
  ASTNodePtr block;

  CompoundStatement(ASTNodePtr block)
      : block(std::move(block)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

// Base Expression class
class ExpressionNode : public ASTNode {
public:
  ~ExpressionNode() override = default;
  virtual void accept(ASTVisitor &visitor) = 0;
};

// Specific Expression types
class BinaryExpression : public ExpressionNode {
public:
  TokenType op;
  ASTNodePtr left;
  ASTNodePtr right;

  BinaryExpression(TokenType op, ASTNodePtr left,
                   ASTNodePtr right)
      : op(op), left(std::move(left)), right(std::move(right)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class UnaryExpression : public ExpressionNode {
public:
  TokenType op;
  ASTNodePtr operand;

  UnaryExpression(TokenType op, ASTNodePtr operand)
      : op(op), operand(std::move(operand)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ConstantExpression : public ExpressionNode {
public:
  int value;

  ConstantExpression(int value) : value(value) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class VariableExpression : public ExpressionNode {
public:
  std::string identifier;

  VariableExpression(std::string identifier)
      : identifier(std::move(identifier)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class AssignmentExpression : public ExpressionNode {
public:
  ASTNodePtr left;
  ASTNodePtr right;
  TokenType type; // Type of assignment (e.g., compound_sum , compound_difference or simple assignment)

  AssignmentExpression(ASTNodePtr left,
                       ASTNodePtr right,
                       TokenType type)
      : left(std::move(left)), right(std::move(right)), type(type) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class PostfixExpression : public ExpressionNode {
public:
  TokenType op; // INCREMENT_OPERATOR or DECREMENT_OPERATOR
  ASTNodePtr operand;

  PostfixExpression(ASTNodePtr operand, TokenType op)
      : op(op), operand(std::move(operand)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ConditionalExpression : public ExpressionNode {
public:
  ASTNodePtr condition;
  ASTNodePtr trueExpr;
  ASTNodePtr falseExpr; 
  ConditionalExpression(ASTNodePtr cond,
                        ASTNodePtr trueE,
                        ASTNodePtr falseE)
      : condition(std::move(cond)), trueExpr(std::move(trueE)),
        falseExpr(std::move(falseE)) {}
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class BlockItemNode : public ASTNode {
public:
  ASTNodePtr block_item = nullptr;

  BlockItemNode(ASTNodePtr item) : block_item(std::move(item)) {}

  ~BlockItemNode() override = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class DeclarationNode : public ASTNode {
public:
  std::string name;
  std::optional<ASTNodePtr> init; // optional initializer expression
  DeclarationNode(std::string name, ASTNodePtr init)
      : name(std::move(name)), init(std::move(init)) {}
  DeclarationNode(std::string name)
      : name(std::move(name)), init(std::nullopt) {}

  ~DeclarationNode() override = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class BlockNode : public ASTNode {
public:
  std::vector<ASTNodePtr> block_items;

  BlockNode(std::vector<ASTNodePtr> items)
      : block_items(std::move(items)) {}

  ~BlockNode() override = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};