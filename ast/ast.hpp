#pragma once
#include "../token/token.hpp"
#include <memory>
#include <optional>
#include <string>
#include <variant>
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
class CastExpression;
class DereferenceExpression;
class AddressOfExpression;
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
class DeclaratorNode;
class Ident;
class PointerDeclarator;
class FunDeclarator;
class AbstractDeclarator;
class AbstractPointer;
class AbstractBase;
class paraminfo;
enum class TypeKind;
class FunType;
class PointerType;

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
using DeclaratorNodePtr = std::unique_ptr<DeclaratorNode>;
using AbstractDeclaratorPtr = std::unique_ptr<AbstractDeclarator>;
using AbstractPointerPtr = std::unique_ptr<AbstractPointer>;
using AbstractBasePtr = std::unique_ptr<AbstractBase>;
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
  virtual void visit(Ident &node) = 0;
  virtual void visit(DeclaratorNode &node) = 0;
  virtual void visit(PointerDeclarator &node) = 0;
  virtual void visit(FunDeclarator &node) = 0;
  virtual void visit(BlockNode &node) = 0;
  virtual void visit(FunDeclNode &node) = 0;
  virtual void visit(VarDeclNode &node) = 0;
  virtual void visit(FunctionCallNode &node) = 0;
  virtual void visit(AbstractPointer &node) = 0;
  virtual void visit(AbstractBase &node) = 0;
  virtual void visit(paraminfo &node) = 0;

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
  virtual void visit(DereferenceExpression &node) = 0;
  virtual void visit(AddressOfExpression &node) = 0;

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

class ASTNode
{
public:
  virtual ~ASTNode() = default;
  virtual void accept(ASTVisitor &visitor) = 0;
  virtual std::unique_ptr<ASTNode> clone() const = 0;
};

enum class StorageClass
{
  STATIC,
  EXTERN,
  AUTO
};

constexpr bool operator==(const TokenType &a, const StorageClass &b)
{
  if (b == StorageClass::STATIC && a == TokenType::STATIC)
  {
    return true;
  }
  if (b == StorageClass::EXTERN && a == TokenType::EXTERN)
  {
    return true;
  }
  return false;
}

struct FunType
{
  std::vector<Type> params;
  std::shared_ptr<Type> ret;
  FunType(std::vector<Type> params, std::shared_ptr<Type> ret)
      : params((params)), ret((ret)) {}
};

struct PointerType
{
  std::shared_ptr<Type> base;
  PointerType(std::shared_ptr<Type> base) : base(base) {}
};

enum class TypeKind
{
  INT,
  LONG,
  UINT,
  ULONG,
  DOUBLE,
  FUNC,
  POINTER,
  ERROR
};
constexpr int size(TypeKind &kind)
{
  switch (kind)
  {
  case TypeKind::INT:
    return 4;
  case TypeKind::UINT:
    return 4;
  case TypeKind::LONG:
    return 8;
  case TypeKind::ULONG:
    return 8;
  case TypeKind::DOUBLE:
    return 8;
  default:
    return 0;
  }
};

class Type : public ASTNode
{
public:
  TypeKind kind;
  std::variant<std::monostate, FunType, PointerType> data;
  // default constructor for int type
  Type() : kind(TypeKind::INT), data(std::move(std::monostate{})) {}
  // constructor for other types
  Type(TypeKind k, std::variant<std::monostate, FunType, PointerType> d)
      : kind(k), data(std::move(d)) {}
  // remove this
  // copy constructor
  Type(const Type &other) : kind(other.kind)
  {
    switch (other.kind)
    {
    case TypeKind::FUNC:
    {
      const auto &funType = std::get<FunType>(other.data);
      std::vector<Type> params_copy = funType.params;
      auto ret_copy = funType.ret;
      data = FunType{params_copy, ret_copy};
      break;
    }
    case TypeKind::POINTER:
    {
      const auto &ptrType = std::get<PointerType>(other.data);
      auto base_copy = ptrType.base;
      data = PointerType{base_copy};
      break;
    }
    default:
      data = std::monostate{};
      break;
    }
  }
  // copy assignment
  Type &operator=(const Type &other)
  {
    if (this != &other)
    {
      kind = other.kind;
      switch (other.kind)
      {
      case TypeKind::FUNC:
      {
        const auto &funType = std::get<FunType>(other.data);
        std::vector<Type> params_copy = funType.params;
        auto ret_copy = funType.ret;
        data = FunType{params_copy, ret_copy};
        break;
      }
      case TypeKind::POINTER:
      {
        const auto &ptrType = std::get<PointerType>(other.data);
        auto base_copy = ptrType.base;
        data = PointerType{base_copy};
        break;
      }
      default:
        data = std::monostate{};
        break;
      }
    }
    return *this;
  }
  // move constructor
  Type(Type &&other) noexcept
      : kind(other.kind),
        data(std::move(other.data)) {}
  // move assignment
  Type &operator=(Type &&other) noexcept
  {
    if (this != &other)
    {
      kind = other.kind;
      data = std::move(other.data);
    }
    return *this;
  }

  static Type Int() { return Type{TypeKind::INT, std::monostate{}}; }
  static Type Long() { return Type{TypeKind::LONG, std::monostate{}}; }
  static Type UInt() { return Type{TypeKind::UINT, std::monostate{}}; }
  static Type ULong() { return Type{TypeKind::ULONG, std::monostate{}}; }
  static Type Double() { return Type{TypeKind::DOUBLE, std::monostate{}}; }
  static Type Function(std::vector<Type> params, Type ret)
  {
    FunType ftype{std::move(params), std::make_unique<Type>(std::move(ret))};
    return Type{TypeKind::FUNC, std::move(ftype)};
  }
  static Type Pointer(std::unique_ptr<Type> base)
  {
    PointerType ptype{std::move(base)};
    return Type{TypeKind::POINTER, std::move(ptype)};
  }
  static Type Error() { return Type{TypeKind::ERROR, std::monostate{}}; }

  bool operator==(Type &other)
  {
    if (other.kind != this->kind)
    {
      return false;
    }
    if (this->kind == TypeKind::FUNC)
    {
      auto &this_fun = std::get<FunType>(this->data);
      auto &other_fun = std::get<FunType>(other.data);
      if (this_fun.params.size() != other_fun.params.size())
      {
        return false;
      }
      for (size_t i = 0; i < this_fun.params.size(); i++)
      {
        if (!(this_fun.params[i] == other_fun.params[i]))
        {
          return false;
        }
      }
      return *(this_fun.ret) == *(other_fun.ret);
    }
    else if (this->kind == TypeKind::POINTER)
    {
      auto &this_ptr = std::get<PointerType>(this->data);
      auto &other_ptr = std::get<PointerType>(other.data);
      return *(this_ptr.base) == *(other_ptr.base);
    }
    return true;
  }

  bool operator!=(Type &other)
  {
    return !(*this == other);
  }

  static Type getCommonType(Type &first, Type &second)
  {
    if (first.kind == second.kind)
    {
      return first;
    }
    if (size(first.kind) == size(second.kind))
    {
      if (first.kind == TypeKind::INT || first.kind == TypeKind::LONG)
      {
        return second;
      }
      else
      {
        return first;
      }
    }
    if (size(first.kind) > size(second.kind))
    {
      return first;
    }
    else
    {
      return second;
    }
    return Type::Int();
  }

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
  std::unique_ptr<ASTNode> clone() const override
  {
    return std::make_unique<Type>(*this);
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

  std::unique_ptr<ASTNode> clone() const override
  {
    std::vector<ASTNodePtr> clonedDecls;
    for (const auto &decl : Declarations)
    {
      clonedDecls.push_back(decl->clone());
    }
    return std::make_unique<ProgramNode>(std::move(clonedDecls));
  }

  std::vector<ASTNodePtr> Declarations;
};

class FunctionDefinitionNode : public ASTNode
{
public:
  std::string name;
  ASTNodePtr body;

  FunctionDefinitionNode() = default;
  FunctionDefinitionNode(std::string name, ASTNodePtr body)
      : name(std::move(name)), body(std::move(body)) {}

  ~FunctionDefinitionNode() override = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    return std::make_unique<FunctionDefinitionNode>(name, body ? body->clone() : nullptr);
  }
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
  std::shared_ptr<Type> type;
  ReturnStatement(ASTNodePtr expr) : expression(std::move(expr)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    auto cloned = std::make_unique<ReturnStatement>(expression ? expression->clone() : nullptr);
    if (type)
      cloned->type = std::make_shared<Type>(*type);
    return cloned;
  }
};

class NullStatement : public StatementNode
{
public:
  NullStatement() = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    return std::make_unique<NullStatement>();
  }
};

class ExpressionStatement : public StatementNode
{
public:
  ASTNodePtr expression;

  ExpressionStatement(ASTNodePtr expr) : expression(std::move(expr)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    return std::make_unique<ExpressionStatement>(expression ? expression->clone() : nullptr);
  }
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

  std::unique_ptr<ASTNode> clone() const override
  {
    std::optional<ASTNodePtr> clonedElse = std::nullopt;
    if (elseBranch)
    {
      clonedElse = (*elseBranch)->clone();
    }
    return std::make_unique<IfStatement>(
        condition ? condition->clone() : nullptr,
        thenBranch ? thenBranch->clone() : nullptr,
        std::move(clonedElse));
  }
};

class GotoStatement : public StatementNode
{
public:
  std::string label;
  GotoStatement(std::string label) : label(std::move(label)) {}
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    return std::make_unique<GotoStatement>(label);
  }
};

class LabelStatement : public StatementNode
{
public:
  std::string label;
  LabelStatement(std::string label) : label(std::move(label)) {}
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    return std::make_unique<LabelStatement>(label);
  }
};

class CompoundStatement : public StatementNode
{
public:
  ASTNodePtr block;

  CompoundStatement(ASTNodePtr block) : block(std::move(block)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    return std::make_unique<CompoundStatement>(block ? block->clone() : nullptr);
  }
};

// Base Expression class
class ExpressionNode : public ASTNode
{
public:
  std::shared_ptr<Type> type;
  ExpressionNode() = default;
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
  // std::shared_ptr<Type> type;
  BinaryExpression(TokenType op, ASTNodePtr left, ASTNodePtr right)
      : op(op), left(std::move(left)), right(std::move(right)) {}
  BinaryExpression() = default;
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    auto cloned = std::make_unique<BinaryExpression>(
        op,
        left ? left->clone() : nullptr,
        right ? right->clone() : nullptr);
    cloned->type = type ? std::make_shared<Type>(*type) : nullptr;
    return cloned;
  }
};

class UnaryExpression : public ExpressionNode
{
public:
  TokenType op;
  ASTNodePtr operand;
  // std::shared_ptr<Type> type;
  UnaryExpression(TokenType op, ASTNodePtr operand)
      : op(op), operand(std::move(operand)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    auto cloned = std::make_unique<UnaryExpression>(
        op,
        operand ? operand->clone() : nullptr);
    cloned->type = type ? std::make_shared<Type>(*type) : nullptr;
    return cloned;
  }
};

class ConstantExpression : public ExpressionNode
{
public:
  std::variant<int, long, unsigned long, unsigned int, double> value;
  // std::shared_ptr<Type> type;
  ConstantExpression(
      std::variant<int, long, unsigned long, unsigned int, double> value)
      : value(value) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    auto cloned = std::make_unique<ConstantExpression>(value);
    cloned->type = type ? std::make_shared<Type>(*type) : nullptr;
    return cloned;
  }
};

class VariableExpression : public ExpressionNode
{
public:
  std::string identifier;
  // std::shared_ptr<Type> type;
  VariableExpression(std::string identifier)
      : identifier(std::move(identifier)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    auto cloned = std::make_unique<VariableExpression>(identifier);
    cloned->type = type ? std::make_shared<Type>(*type) : nullptr;
    return cloned;
  }
};

class AssignmentExpression : public ExpressionNode
{
public:
  ASTNodePtr left;
  ASTNodePtr right;
  TokenType assignment_type; // Type of assignment (e.g., compound_sum ,
                             // compound_difference or simple assignment)
                             // std::shared_ptr<Type> type;
  AssignmentExpression(ASTNodePtr left, ASTNodePtr right, TokenType assignment_type)
      : left(std::move(left)), right(std::move(right)), assignment_type(assignment_type) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    auto cloned = std::make_unique<AssignmentExpression>(
        left ? left->clone() : nullptr,
        right ? right->clone() : nullptr,
        assignment_type);
    cloned->type = type ? std::make_shared<Type>(*type) : nullptr;
    return cloned;
  }
};

class PostfixExpression : public ExpressionNode
{
public:
  TokenType op; // INCREMENT_OPERATOR or DECREMENT_OPERATOR
  ASTNodePtr operand;
  // std::shared_ptr<Type> type;
  PostfixExpression(ASTNodePtr operand, TokenType op)
      : op(op), operand(std::move(operand)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    auto cloned = std::make_unique<PostfixExpression>(
        operand ? operand->clone() : nullptr,
        op);
    cloned->type = type ? std::make_shared<Type>(*type) : nullptr;
    return cloned;
  }
};

class ConditionalExpression : public ExpressionNode
{
public:
  ASTNodePtr condition;
  ASTNodePtr trueExpr;
  ASTNodePtr falseExpr;
  // std::shared_ptr<Type> type;
  ConditionalExpression(ASTNodePtr cond, ASTNodePtr trueE, ASTNodePtr falseE)
      : condition(std::move(cond)), trueExpr(std::move(trueE)),
        falseExpr(std::move(falseE)) {}
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    auto cloned = std::make_unique<ConditionalExpression>(
        condition ? condition->clone() : nullptr,
        trueExpr ? trueExpr->clone() : nullptr,
        falseExpr ? falseExpr->clone() : nullptr);
    cloned->type = type ? std::make_shared<Type>(*type) : nullptr;
    return cloned;
  }
};

class CastExpression : public ExpressionNode
{
public:
  Type targetType;
  ASTNodePtr expression;
  CastExpression(Type targetType, ASTNodePtr expr)
      : targetType(std::move(targetType)), expression(std::move(expr)) {}
  CastExpression() = default;
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    auto cloned = std::make_unique<CastExpression>(
        targetType,
        expression ? expression->clone() : nullptr);
    cloned->type = type ? std::make_shared<Type>(*type) : nullptr;
    return cloned;
  }
};

class DereferenceExpression : public ExpressionNode
{
public:
  ASTNodePtr pointerExpr;
  // std::shared_ptr<Type> type;
  DereferenceExpression(ASTNodePtr ptrExpr) : pointerExpr(std::move(ptrExpr)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    auto cloned = std::make_unique<DereferenceExpression>(
        pointerExpr ? pointerExpr->clone() : nullptr);
    cloned->type = type ? std::make_shared<Type>(*type) : nullptr;
    return cloned;
  }
};

class AddressOfExpression : public ExpressionNode
{
public:
  ASTNodePtr variableExpr;
  // std::shared_ptr<Type> type;
  AddressOfExpression(ASTNodePtr varExpr) : variableExpr(std::move(varExpr)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    auto cloned = std::make_unique<AddressOfExpression>(
        variableExpr ? variableExpr->clone() : nullptr);
    cloned->type = type ? std::make_shared<Type>(*type) : nullptr;
    return cloned;
  }
};

class BlockItemNode : public ASTNode
{
public:
  ASTNodePtr block_item = nullptr;

  BlockItemNode(ASTNodePtr item) : block_item(std::move(item)) {}

  ~BlockItemNode() override = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    return std::make_unique<BlockItemNode>(block_item ? block_item->clone() : nullptr);
  }
};

class DeclarationNode : public ASTNode
{
public:
  ASTNodePtr declaration = nullptr; // var_decl or fun_decl
  DeclarationNode() = default;
  DeclarationNode(ASTNodePtr decl) : declaration(std::move(decl)) {}

  ~DeclarationNode() override = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    return std::make_unique<DeclarationNode>(declaration ? declaration->clone() : nullptr);
  }
};

class DeclaratorNode : public ASTNode
{
public:
  virtual ~DeclaratorNode() = default;
  DeclaratorNode(DeclaratorNode &&other) = default;
  DeclaratorNode() = default;
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
  std::unique_ptr<ASTNode> clone() const override = 0;
};

class Ident : public DeclaratorNode
{
public:
  std::string identifier;

  Ident(std::string identifier) : identifier(std::move(identifier)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
  
  std::unique_ptr<ASTNode> clone() const override
  {
    return std::make_unique<Ident>(identifier);
  }
};

class PointerDeclarator : public DeclaratorNode
{
public:
  ASTNodePtr declarator;

  PointerDeclarator(ASTNodePtr declarator)
      : declarator(std::move(declarator)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    return std::make_unique<PointerDeclarator>(declarator ? declarator->clone() : nullptr);
  }
};

class paraminfo : public ASTNode
{
public:
  Type type; // parameter type
  ASTNodePtr declarator;
  paraminfo(Type type, ASTNodePtr declarator)
      : type(std::move(type)), declarator(std::move(declarator)) {}
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    return std::make_unique<paraminfo>(type, declarator ? declarator->clone() : nullptr);
  }
};

class FunDeclarator : public DeclaratorNode
{
public:
  std::vector<paraminfo> params;
  ASTNodePtr declarator;

  FunDeclarator(std::vector<paraminfo> params, ASTNodePtr declarator)
      : params(std::move(params)), declarator(std::move(declarator)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    std::vector<paraminfo> clonedParams;
    for (const auto &param : params)
    {
      clonedParams.push_back(paraminfo(param.type, param.declarator ? param.declarator->clone() : nullptr));
    }
    return std::make_unique<FunDeclarator>(std::move(clonedParams), declarator ? declarator->clone() : nullptr);
  }
};

class AbstractDeclarator : public ASTNode
{
public:
  virtual ~AbstractDeclarator() = default;
  std::unique_ptr<ASTNode> clone() const override = 0;
};

class AbstractBase : public AbstractDeclarator
{
public:
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    return std::make_unique<AbstractBase>();
  }
};

class AbstractPointer : public AbstractDeclarator
{
public:
  ASTNodePtr base; // AbstractDeclarator type
  AbstractPointer(ASTNodePtr base) : base(std::move(base)) {}
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    return std::make_unique<AbstractPointer>(base ? base->clone() : nullptr);
  }
};


class FunDeclNode : public ASTNode
{
public:
  std::string name;
  std::vector<std::string> param_names;
  std::optional<ASTNodePtr> body;         // block
  Type type;                              // return type
  std::optional<TokenType> storage_class; // storage class
  std::vector<Type> param_types;          // parameter info
  FunDeclNode() = default;
  FunDeclNode(std::string name, std::vector<std::string> param_names,
              ASTNodePtr body)
      : name(std::move(name)), param_names(std::move(param_names)),
        body(std::move(body)) {}
  FunDeclNode(std::string name, std::vector<std::string> param_names)
      : name(std::move(name)), param_names(std::move(param_names)),
        body(std::nullopt) {}
  ~FunDeclNode() override = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    auto cloned = std::make_unique<FunDeclNode>();
    cloned->name = name;
    cloned->param_names = param_names;
    cloned->body = body.has_value() && body.value() ? std::optional<ASTNodePtr>(body.value()->clone()) : std::nullopt;
    cloned->type = type;
    cloned->storage_class = storage_class;
    cloned->param_types = param_types;
    return cloned;
  }
};

class VarDeclNode : public ASTNode
{
public:
  std::string name;
  std::optional<ASTNodePtr> init;         // optional initializer expression
  Type type;                              // type specifier
  std::optional<TokenType> storage_class; // storage
  VarDeclNode() = default;
  VarDeclNode(std::string name, ASTNodePtr init)
      : name(std::move(name)), init(std::move(init)) {}
  VarDeclNode(std::string name) : name(std::move(name)), init(std::nullopt) {}
  VarDeclNode(std::string name, Type type)
      : name(std::move(name)), type(std::move(type)) {}
  ~VarDeclNode() override = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    auto cloned = std::make_unique<VarDeclNode>();
    cloned->name = name;
    cloned->init = init.has_value() && init.value() ? std::optional<ASTNodePtr>(init.value()->clone()) : std::nullopt;
    cloned->type = type;
    cloned->storage_class = storage_class;
    return cloned;
  }
};

class BlockNode : public ASTNode
{
public:
  std::vector<ASTNodePtr> block_items;

  BlockNode(std::vector<ASTNodePtr> items) : block_items(std::move(items)) {}

  ~BlockNode() override = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    std::vector<ASTNodePtr> clonedItems;
    for (const auto &item : block_items)
    {
      clonedItems.push_back(item ? item->clone() : nullptr);
    }
    return std::make_unique<BlockNode>(std::move(clonedItems));
  }
};

class ForInit : public ASTNode
{
public:
  ASTNodePtr init;
  ForInit() = default;
  ForInit(ASTNodePtr init) : init(std::move(init)) {}

  ~ForInit() override = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    return std::make_unique<ForInit>(init ? init->clone() : nullptr);
  }
};

class InitDecl : public ForInit
{
public:
  ASTNodePtr init; // var_decl
  InitDecl(ASTNodePtr init) : init(std::move(init)) {}

  ~InitDecl() override = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    return std::make_unique<InitDecl>(init ? init->clone() : nullptr);
  }
};

class InitExp : public ForInit
{
public:
  std::optional<ASTNodePtr> init;
  InitExp(std::optional<ASTNodePtr> init) : init(std::move(init)) {}
  ~InitExp() override = default;
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    return std::make_unique<InitExp>(
        init.has_value() && init.value() ? std::optional<ASTNodePtr>(init.value()->clone()) : std::nullopt);
  }
};

class BreakNode : public StatementNode
{
public:
  std::string label;
  BreakNode() = default;
  BreakNode(std::string label) : label(std::move(label)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    return std::make_unique<BreakNode>(label);
  }
};

class ContinueNode : public StatementNode
{
public:
  std::string label;
  ContinueNode() = default;
  ContinueNode(std::string label) : label(std::move(label)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    return std::make_unique<ContinueNode>(label);
  }
};

class WhileNode : public StatementNode
{
public:
  ASTNodePtr condition; // exp
  ASTNodePtr body;      // statement
  std::string label;    // identifier
  WhileNode(ASTNodePtr condition, ASTNodePtr body, std::string label = "")
      : condition(std::move(condition)), body(std::move(body)),
        label(std::move(label)) {}

  WhileNode() = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    return std::make_unique<WhileNode>(
        condition ? condition->clone() : nullptr,
        body ? body->clone() : nullptr,
        label);
  }
};

class DoWhileNode : public StatementNode
{
public:
  ASTNodePtr condition; // exp
  ASTNodePtr body;      // statement
  std::string label;    // identifier
  DoWhileNode(ASTNodePtr condition, ASTNodePtr body, std::string label = "")
      : condition(std::move(condition)), body(std::move(body)),
        label(std::move(label)) {}

  DoWhileNode() = default;

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    return std::make_unique<DoWhileNode>(
        condition ? condition->clone() : nullptr,
        body ? body->clone() : nullptr,
        label);
  }
};

class ForNode : public StatementNode
{
public:
  ASTNodePtr init;                     // for_init
  std::optional<ASTNodePtr> condition; // exp
  std::optional<ASTNodePtr> post;      // exp
  ASTNodePtr body;                     // s tatement
  std::string label;
  ForNode(ASTNodePtr init, std::optional<ASTNodePtr> condition,
          std::optional<ASTNodePtr> post, ASTNodePtr body,
          std::string label = "")
      : init(std::move(init)), condition(std::move(condition)),
        post(std::move(post)), body(std::move(body)), label(std::move(label)) {}

  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    return std::make_unique<ForNode>(
        init ? init->clone() : nullptr,
        condition.has_value() && condition.value() ? std::optional<ASTNodePtr>(condition.value()->clone()) : std::nullopt,
        post.has_value() && post.value() ? std::optional<ASTNodePtr>(post.value()->clone()) : std::nullopt,
        body ? body->clone() : nullptr,
        label);
  }
};

class FunctionCallNode : public ExpressionNode
{
public:
  std::string name;
  std::vector<ASTNodePtr> args;
  std::vector<std::shared_ptr<Type>> param_types; // parameter types after semantic analysis
  FunctionCallNode(std::string name, std::vector<ASTNodePtr> args)
      : name(std::move(name)), args(std::move(args)) {}
  ~FunctionCallNode() override = default;
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  std::unique_ptr<ASTNode> clone() const override
  {
    std::vector<ASTNodePtr> clonedArgs;
    for (const auto &arg : args)
    {
      clonedArgs.push_back(arg ? arg->clone() : nullptr);
    }
    auto cloned = std::make_unique<FunctionCallNode>(name, std::move(clonedArgs));
    // Clone param_types
    for (const auto &paramType : param_types)
    {
      cloned->param_types.push_back(paramType ? std::make_shared<Type>(*paramType) : nullptr);
    }
    cloned->type = type ? std::make_shared<Type>(*type) : nullptr;
    return cloned;
  }
};
