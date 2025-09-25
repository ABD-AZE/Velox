#pragma once
#include "../token/token.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <variant>

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
class CastExpression;
class BlockNode;
class ForInit;
class InitDecl;
class InitExp;
class BreakNode;
class ContinueNode;
class WhileNode;
class DoWhileNode;
class ForNode;
class FunDeclNode;
class VarDeclNode;
class FunctionCallNode;
class Type;

using ASTNodePtr = std::unique_ptr<ASTNode>;
using ProgramNodePtr = std::unique_ptr<ProgramNode>;
using FunDeclNodePtr = std::unique_ptr<FunDeclNode>;
using StatementNodePtr = std::unique_ptr<StatementNode>;
using ExpressionNodePtr = std::unique_ptr<ExpressionNode>;
using BlockItemNodePtr = std::unique_ptr<BlockItemNode>;
using DeclarationNodePtr = std::unique_ptr<DeclarationNode>;
using BlockNodePtr = std::unique_ptr<BlockNode>;
using VarDeclNodePtr = std::unique_ptr<VarDeclNode>;
using FunDeclNodePtr = std::unique_ptr<FunDeclNode>;

// Visitor interface to use visitor pattern
class ASTVisitor
{
public:
  virtual ~ASTVisitor() = default;


  // enum classes
  virtual void visit(Type &node) = 0;
  // Visit methods for each node type
  virtual void visit(ProgramNode &node) = 0;
  virtual void visit(FunctionDefinitionNode &node) = 0;
  virtual void visit(BlockItemNode &node) = 0;
  virtual void visit(DeclarationNode &node) = 0;
  virtual void visit(BlockNode &node) = 0;
  virtual void visit(FunDeclNode &node) = 0;
  virtual void visit(VarDeclNode &node) = 0;
  virtual void visit(FunctionCallNode &node) = 0;

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
  virtual void visit(CastExpression &node) = 0;

  // loop
  virtual void visit(ForInit &node) = 0;
  virtual void visit(InitDecl &node) = 0;
  virtual void visit(InitExp &node) = 0;
  virtual void visit(BreakNode &node) = 0;
  virtual void visit(ContinueNode &node) = 0;
  virtual void visit(WhileNode &node) = 0;
  virtual void visit(DoWhileNode &node) = 0;
  virtual void visit(ForNode &node) = 0;
};

enum class StorageClass { STATIC, EXTERN };

struct FunType {
  std::vector<struct Type> params;
  std::unique_ptr<struct Type> ret;
};

enum class TypeKind {
  INT,
  LONG,
  FUNC,
  ERROR
};

class ASTNode
{
public:
  virtual ~ASTNode() = default;
  virtual void accept(ASTVisitor &visitor) = 0;
};

class Type: public ASTNode {
  public:
  TypeKind kind;
  std::variant<std::monostate, FunType> data;
  // default constructor for int type
  Type() : kind(TypeKind::INT), data(std::move(std::monostate{})) {}
  // constructor for specific type
  Type(TypeKind k, std::variant<std::monostate, FunType> d) : kind(k), data(std::move(d)) {}
  // move constructor
  Type(Type&& other) noexcept : kind(other.kind), data(std::move(other.data)) {}
  // move assignment
  Type& operator=(Type&& other) noexcept {
    if (this != &other) {
      kind = other.kind;
      data = std::move(other.data);
    }
    return *this;
  }

  static Type Int() { return Type{TypeKind::INT, std::monostate{}}; }
  static Type Long() { return Type{TypeKind::LONG, std::monostate{}}; }
  static Type Function(std::vector<Type> params, Type ret) {
    FunType ftype{std::move(params), std::make_unique<Type>(std::move(ret))};
    return Type{TypeKind::FUNC, std::move(ftype)};
  }
  static Type Error() { return Type{TypeKind::ERROR, std::monostate{}}; }

  void accept(ASTVisitor &visitor) override {
    visitor.visit(*this);
  }
};


class ProgramNode : public ASTNode
{
public:
  ProgramNode(std::vector<ASTNodePtr> &&Declarations)
      : Declarations(std::move(Declarations)) {}

  ~ProgramNode() override = default;

  // double dispatch to the correct accept method
  void accept(ASTVisitor &visitor) override
  {
    visitor.visit(*this); // single dispatch to the correct visit method
  }

  std::vector<ASTNodePtr> Declarations;
};

class FunctionDefinitionNode : public ASTNode
{
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
class StatementNode : public ASTNode
{
public:
  ~StatementNode() override = default;
  virtual void accept(ASTVisitor &visitor) = 0;
};

// Specific Statement types
class ReturnStatement : public StatementNode
{
public:
  ASTNodePtr expression;

  ReturnStatement(ASTNodePtr expr)
      : expression(std::move(expr)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class NullStatement : public StatementNode
{
public:
  NullStatement() = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ExpressionStatement : public StatementNode
{
public:
  ASTNodePtr expression;

  ExpressionStatement(ASTNodePtr expr)
      : expression(std::move(expr)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class IfStatement : public StatementNode
{
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

class GotoStatement : public StatementNode
{
public:
  std::string label;
  GotoStatement(std::string label) : label(std::move(label)) {}
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class LabelStatement : public StatementNode
{
public:
  std::string label;
  LabelStatement(std::string label) : label(std::move(label)) {}
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class CompoundStatement : public StatementNode
{
public:
  ASTNodePtr block;

  CompoundStatement(ASTNodePtr block)
      : block(std::move(block)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

// Base Expression class
class ExpressionNode : public ASTNode
{
public:
  ~ExpressionNode() override = default;
  virtual void accept(ASTVisitor &visitor) = 0;
};

// Specific Expression types
class BinaryExpression : public ExpressionNode
{
public:
  TokenType op;
  ASTNodePtr left;
  ASTNodePtr right;

  BinaryExpression(TokenType op, ASTNodePtr left,
                   ASTNodePtr right)
      : op(op), left(std::move(left)), right(std::move(right)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class UnaryExpression : public ExpressionNode
{
public:
  TokenType op;
  ASTNodePtr operand;

  UnaryExpression(TokenType op, ASTNodePtr operand)
      : op(op), operand(std::move(operand)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ConstantExpression : public ExpressionNode
{
public:
  std::variant<int, long> value;

  ConstantExpression(std::variant<int, long> value) : value(value) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class VariableExpression : public ExpressionNode
{
public:
  std::string identifier;

  VariableExpression(std::string identifier)
      : identifier(std::move(identifier)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class AssignmentExpression : public ExpressionNode
{
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

class PostfixExpression : public ExpressionNode
{
public:
  TokenType op; // INCREMENT_OPERATOR or DECREMENT_OPERATOR
  ASTNodePtr operand;

  PostfixExpression(ASTNodePtr operand, TokenType op)
      : op(op), operand(std::move(operand)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ConditionalExpression : public ExpressionNode
{
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

class CastExpression : public ExpressionNode
{ 
public:
  Type targetType;
  ASTNodePtr expression;

  CastExpression(Type targetType, ASTNodePtr expr)
      : targetType(std::move(targetType)), expression(std::move(expr)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class BlockItemNode : public ASTNode
{
public:
  ASTNodePtr block_item = nullptr;

  BlockItemNode(ASTNodePtr item) : block_item(std::move(item)) {}

  ~BlockItemNode() override = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class DeclarationNode : public ASTNode
{
public:
  ASTNodePtr declaration = nullptr; // var_decl or fun_decl
  DeclarationNode() = default;
  DeclarationNode(ASTNodePtr decl) : declaration(std::move(decl)) {}

  ~DeclarationNode() override = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class FunDeclNode : public ASTNode
{
public:
  std::string name;
  std::vector<std::string> params;
  std::optional<ASTNodePtr> body; // block
  Type type; // type specifier
  std::optional<TokenType> storage_class; // storage
  FunDeclNode() = default;
  FunDeclNode(std::string name, std::vector<std::string> params, ASTNodePtr body)
      : name(std::move(name)), params(std::move(params)), body(std::move(body)) {}
  FunDeclNode(std::string name, std::vector<std::string> params)
      : name(std::move(name)), params(std::move(params)), body(std::nullopt) {}
  ~FunDeclNode() override = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class VarDeclNode : public ASTNode
{
public:
  std::string name;
  std::optional<ASTNodePtr> init; // optional initializer expression
  Type type; // type specifier
  std::optional<TokenType> storage_class; // storage
  VarDeclNode() = default;
  VarDeclNode(std::string name, ASTNodePtr init)
      : name(std::move(name)), init(std::move(init)) {}
  VarDeclNode(std::string name)
      : name(std::move(name)), init(std::nullopt) {}
  ~VarDeclNode() override = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class BlockNode : public ASTNode
{
public:
  std::vector<ASTNodePtr> block_items;

  BlockNode(std::vector<ASTNodePtr> items)
      : block_items(std::move(items)) {}

  ~BlockNode() override = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ForInit : public ASTNode
{
public:
  ASTNodePtr init;
  ForInit() = default;
  ForInit(ASTNodePtr init) : init(std::move(init)) {}

  ~ForInit() override = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class InitDecl : public ForInit
{
public:
  ASTNodePtr init; // var_decl
  InitDecl(ASTNodePtr init) : init(std::move(init)) {}

  ~InitDecl() override = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class InitExp : public ForInit
{
public:
  std::optional<ASTNodePtr> init;
  InitExp(std::optional<ASTNodePtr> init) : init(std::move(init)) {}
  ~InitExp() override = default;
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class BreakNode : public StatementNode
{
public:
  std::string label;
  BreakNode() = default;
  BreakNode(std::string label) : label(std::move(label)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ContinueNode : public StatementNode
{
public:
  std::string label;
  ContinueNode() = default;
  ContinueNode(std::string label) : label(std::move(label)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class WhileNode : public StatementNode
{
public:
  ASTNodePtr condition; // exp
  ASTNodePtr body;      // statement
  std::string label;    // identifier
  WhileNode(ASTNodePtr condition, ASTNodePtr body, std::string label = "")
      : condition(std::move(condition)), body(std::move(body)), label(std::move(label)) {}

  WhileNode() = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class DoWhileNode : public StatementNode
{
public:
  ASTNodePtr condition; // exp
  ASTNodePtr body;      // statement
  std::string label;    // identifier
  DoWhileNode(ASTNodePtr condition, ASTNodePtr body, std::string label = "")
      : condition(std::move(condition)), body(std::move(body)), label(std::move(label)) {}

  DoWhileNode() = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ForNode : public StatementNode
{
public:
  ASTNodePtr init;                     // for_init
  std::optional<ASTNodePtr> condition; // exp
  std::optional<ASTNodePtr> post;      // exp
  ASTNodePtr body;                     // s tatement
  std::string label;
  ForNode(ASTNodePtr init, std::optional<ASTNodePtr> condition, std::optional<ASTNodePtr> post, ASTNodePtr body, std::string label = "")
      : init(std::move(init)), condition(std::move(condition)), post(std::move(post)), body(std::move(body)), label(std::move(label)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class FunctionCallNode : public ExpressionNode
{
public:
  std::string name;
  std::vector<ASTNodePtr> args;
  FunctionCallNode(std::string name, std::vector<ASTNodePtr> args)
      : name(std::move(name)), args(std::move(args)) {}
  ~FunctionCallNode() override = default;
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};
