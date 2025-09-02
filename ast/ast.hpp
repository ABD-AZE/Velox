#pragma once
#include "../token/token.hpp"

class ASTNode;
class ProgramNode;
class FunctionDefinitionNode;
class StatementNode;
class ExpressionNode;

using ASTNodePtr = std::unique_ptr<ASTNode>;
using ProgramNodePtr = std::unique_ptr<ProgramNode>;
using FunctionDefinitionNodePtr = std::unique_ptr<FunctionDefinitionNode>;
using StatementNodePtr = std::unique_ptr<StatementNode>;
using ExpressionNodePtr = std::unique_ptr<ExpressionNode>;

class ASTNode {
public:
    virtual ~ASTNode() = default;
};

class ProgramNode : public ASTNode {
public:
    ProgramNode(std::unique_ptr<ASTNode>&& functionDefinition)
        : functionDefinition(std::move(functionDefinition)) {}

    ~ProgramNode() override = default;

private:
    std::unique_ptr<ASTNode> functionDefinition;
};

class FunctionDefinitionNode : public ASTNode {
public:
    FunctionDefinitionNode() = default;
    FunctionDefinitionNode(std::string name, std::unique_ptr<ASTNode> statement)
        : name(std::move(name)), statement(std::move(statement)) {}

    ~FunctionDefinitionNode() override = default;     

    std::string name; 
    std::unique_ptr<ASTNode> statement;
};

class StatementNode : public ASTNode{
  public:
    StatementNode() = default;
    StatementNode(std::unique_ptr<ASTNode> expression)
        : expression(std::move(expression)) {}

    ~StatementNode() override = default;

    std::unique_ptr<ASTNode> expression;
};

class ExpressionNode : public ASTNode {
  public:
  TokenType op = TokenType::WS; // WS used as default
    ASTNodePtr left = nullptr;
    ASTNodePtr right = nullptr;
    int const_token;

  public:
    ExpressionNode() = default;
    ExpressionNode(int const_token)
        : const_token(const_token) {}

    ExpressionNode(TokenType op, std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right)
        : op(op), left(std::move(left)), right(std::move(right)) {}

    ExpressionNode(TokenType op, ASTNodePtr exp)
        : op(op), right(std::move(exp)) {}

    ~ExpressionNode() override = default;

};  
