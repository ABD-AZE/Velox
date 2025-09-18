#pragma once
#include "ast.hpp"
#include <iostream>
#include <string>

class ASTPrinter {
public:
    static void print(const ASTNodePtr& node, int indent = 0);
    
private:
    static void printIndent(int indent);
    static void printProgramNode(const ProgramNode* node, int indent);
    static void printFunctionDefinitionNode(const FunctionDefinitionNode* node, int indent);
    static void printStatementNode(const StatementNode* node, int indent);
    static void printExpressionNode(const ExpressionNode* node, int indent);
    static void printBlockItemNode(const BlockItemNode* node, int indent);
    static void printDeclarationNode(const DeclarationNode* node, int indent);
};