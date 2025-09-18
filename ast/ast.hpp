#pragma once
#include "../token/token.hpp"
#include <memory>
#include<vector> 
#include<string>
#include<optional>

class ASTNode;
class ProgramNode;
class FunctionDefinitionNode;
class StatementNode;
class ExpressionNode;
class BlockItemNode;
class DeclarationNode;

using ASTNodePtr = std::unique_ptr<ASTNode>;
using ProgramNodePtr = std::unique_ptr<ProgramNode>;
using FunctionDefinitionNodePtr = std::unique_ptr<FunctionDefinitionNode>;
using StatementNodePtr = std::unique_ptr<StatementNode>;
using ExpressionNodePtr = std::unique_ptr<ExpressionNode>;
using BlockItemNodePtr = std::unique_ptr<BlockItemNode>;
using DeclarationNodePtr = std::unique_ptr<DeclarationNode>;

class ASTNode {
public:
    virtual ~ASTNode() = default;
};

class ProgramNode : public ASTNode {
public:
    ProgramNode(std::unique_ptr<ASTNode>&& functionDefinition)
        : functionDefinition(std::move(functionDefinition)) {}

    ~ProgramNode() override = default;

    std::unique_ptr<ASTNode> functionDefinition;
};

class FunctionDefinitionNode : public ASTNode {
public:
    FunctionDefinitionNode() = default;
    FunctionDefinitionNode(std::string name, std::vector<std::unique_ptr<ASTNode>>& block_items)
        : name(std::move(name)), block_items(std::move(block_items)) {}

    ~FunctionDefinitionNode() override = default;     

    std::string name; 
    std::vector<std::unique_ptr<ASTNode>> block_items;
};

class StatementNode : public ASTNode{
  public:
    bool isreturn = false;
    bool isnull = false;
    StatementNode(): isreturn(false), isnull(true), expression(nullptr) {}
    StatementNode(std::unique_ptr<ASTNode> expression)
        : expression(std::move(expression)) {}
    StatementNode(bool isreturn, std::unique_ptr<ASTNode> expression)
        : isreturn(isreturn), expression(std::move(expression)) {}

    ~StatementNode() override = default;

    std::unique_ptr<ASTNode> expression;
};

class ExpressionNode : public ASTNode {
  public:
    TokenType op = TokenType::WS; // WS used as default
    ASTNodePtr left = nullptr;
    ASTNodePtr right = nullptr;
    int const_value = 0;
    std::string identifier;
    bool isconst = false;
    bool isunary = false;
    bool isbinary = false;
    bool isvar = false;
    bool isassignment = false;

  public:
    ExpressionNode() = default;
    ExpressionNode(int const_value)
        : const_value(const_value), isconst(true) {}

    ExpressionNode(TokenType op, std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right) // for binary operator
        : op(op), left(std::move(left)), right(std::move(right)), isbinary(true) {}

    ExpressionNode(TokenType op, ASTNodePtr exp)  // for unary operator
        : op(op), right(std::move(exp)), isunary(true) {}

    ExpressionNode(std::string identifier) // for variable
        : identifier(std::move(identifier)), isvar(true) {}

    // for assignment
    ExpressionNode(ASTNodePtr left, ASTNodePtr right)
        : left(std::move(left)), right(std::move(right)), isassignment(true) {}

    ~ExpressionNode() override = default;
};

class BlockItemNode : public ASTNode {
public:
    ASTNodePtr block_item = nullptr;
    
    BlockItemNode(ASTNodePtr item)
        : block_item(std::move(item)) {}

    ~BlockItemNode() override = default;
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
};