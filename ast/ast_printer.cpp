#include "ast_printer.hpp"
#include <iostream>

void ASTPrinter::print(const ASTNodePtr& node, int indent) {
    if (!node) {
        return;
    }
    
    // Use dynamic_cast to determine the actual node type
    if (auto programNode = dynamic_cast<const ProgramNode*>(node.get())) {
        printProgramNode(programNode, indent);
    } else if (auto functionNode = dynamic_cast<const FunctionDefinitionNode*>(node.get())) {
        printFunctionDefinitionNode(functionNode, indent);
    } else if (auto statementNode = dynamic_cast<const StatementNode*>(node.get())) {
        printStatementNode(statementNode, indent);
    } else if (auto expressionNode = dynamic_cast<const ExpressionNode*>(node.get())) {
        printExpressionNode(expressionNode, indent);
    } else if (auto blockItemNode = dynamic_cast<const BlockItemNode*>(node.get())) {
        printBlockItemNode(blockItemNode, indent);
    } else if (auto declarationNode = dynamic_cast<const DeclarationNode*>(node.get())) {
        printDeclarationNode(declarationNode, indent);
    }
}

void ASTPrinter::printIndent(int indent) {
    for (int i = 0; i < indent; ++i) {
        std::cout << "  "; // 2 spaces per indent level
    }
}

void ASTPrinter::printProgramNode(const ProgramNode* node, int indent) {
    printIndent(indent);
    std::cout << "Program(" << std::endl;
    
    // Print the function definition
    if (node->functionDefinition) {
        print(node->functionDefinition, indent + 1);
    }
    
    printIndent(indent);
    std::cout << ")" << std::endl;
}

void ASTPrinter::printFunctionDefinitionNode(const FunctionDefinitionNode* node, int indent) {
    printIndent(indent);
    std::cout << "Function(" << std::endl;
    
    // Print function name
    printIndent(indent + 1);
    std::cout << "name=\"" << node->name << "\"," << std::endl;
    
    // Print function body (block items)
    printIndent(indent + 1);
    std::cout << "body=[" << std::endl;
    for (const auto& blockItem : node->block_items) {
        if (blockItem) {
            print(blockItem, indent + 2);
        }
    }
    printIndent(indent + 1);
    std::cout << "]" << std::endl;
    
    printIndent(indent);
    std::cout << ")" << std::endl;
}

void ASTPrinter::printStatementNode(const StatementNode* node, int indent) {
    printIndent(indent);
    
    if (node->isnull) {
        std::cout << "NullStatement()" << std::endl;
    } else if (node->isreturn) {
        std::cout << "Return(";
        if (node->expression) {
            print(node->expression, indent + 1);
        }
        printIndent(indent);
        std::cout << ")" << std::endl;
    } else {
        std::cout << "ExpressionStatement(";
        if (node->expression) {
            print(node->expression, indent + 1);
        }
        printIndent(indent);
        std::cout << ")" << std::endl;
    }
}

void ASTPrinter::printExpressionNode(const ExpressionNode* node, int indent) {
    // printIndent(indent);
    
    if (node->isconst) {
        std::cout << "Constant(" << node->const_value << ")" << std::endl;
    } else if (node->isunary) {
        std::cout << "Unary(" << std::endl;
        printIndent(indent + 1);
        std::cout << "op=" << TokenTypeToString(node->op) << "," << std::endl;
        printIndent(indent + 1);
        std::cout << "operand=";
        if (node->right) {
            print(node->right, indent+2);
        }
        printIndent(indent);
        std::cout << ")" << std::endl;
    } else if (node->isbinary) {
        std::cout << "Binary(" << std::endl;
        printIndent(indent + 1);
        std::cout << "op=" << TokenTypeToString(node->op) << "," << std::endl;
        printIndent(indent + 1);
        std::cout << "left=" ;
        if (node->left) {
            print(node->left, indent + 2);
        }
        printIndent(indent + 1);
        std::cout << "right=";
        if (node->right) {
            print(node->right, indent + 2);
        }
        printIndent(indent);
        std::cout << ")" << std::endl;
    } else if (node->isvar) {
        std::cout << "Var(" << node->identifier << ")" << std::endl;
    } else if (node->isassignment) {
        std::cout << "Assignment(" << std::endl;
        printIndent(indent + 1);
        std::cout << "left=" ;
        if (node->left) {
            print(node->left, indent + 2);  
        }
        printIndent(indent + 1);
        std::cout << "right=";
        if (node->right) {
            print(node->right, indent + 2);
        }
        printIndent(indent);
        std::cout << ")" << std::endl;
    }
    else {
        std::cout << "Const(" << node->const_value << ")" << std::endl;
    }
}

void ASTPrinter::printBlockItemNode(const BlockItemNode* node, int indent) {
    printIndent(indent);
    std::cout << "BlockItem("<<std::endl;
    
    if (node->block_item) {
        print(node->block_item, indent + 1);
    }
    
    printIndent(indent);
    std::cout << ")" << std::endl;
}

void ASTPrinter::printDeclarationNode(const DeclarationNode* node, int indent) {
    printIndent(indent);
    std::cout << "Declaration(" << std::endl;
    
    // Print variable name
    printIndent(indent + 1);
    std::cout << "name=\"" << node->name << "\"";
    
    // Print initializer if present
    if (node->init.has_value() && node->init.value()) {
        std::cout << "," << std::endl;
        printIndent(indent + 1);
        std::cout << "init=";
        print(node->init.value(), indent + 2);
    } else {
        std::cout << std::endl;
    }
    
    printIndent(indent);
    std::cout << ")" << std::endl;
}