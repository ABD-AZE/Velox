#pragma once

#include<string>
#include<vector>
#include<memory>
#include <algorithm>
#include "../ast/ast.hpp"
#include "../token/token.hpp"

// Type aliases for unique_ptr AST node types
using AST_Node_programPtr = std::unique_ptr<AST_Node_program>;
using AST_Node_declarationPtr = std::unique_ptr<AST_Node_declaration>;
using AST_Node_variable_declarationPtr = std::unique_ptr<AST_Node_variable_declaration>;
using AST_Node_function_declarationPtr = std::unique_ptr<AST_Node_function_declaration>;
using AST_Node_struct_declarationPtr = std::unique_ptr<AST_Node_struct_declaration>;
using AST_Node_union_declarationPtr = std::unique_ptr<AST_Node_union_declaration>;
using AST_Node_enum_declarationPtr = std::unique_ptr<AST_Node_enum_declaration>;
using AST_Node_typedef_declarationPtr = std::unique_ptr<AST_Node_typedef_declaration>;
using AST_Node_class_declarationPtr = std::unique_ptr<AST_Node_class_declaration>;
using AST_Node_typePtr = std::unique_ptr<AST_Node_type>;
using AST_Node_member_declarationPtr = std::unique_ptr<AST_Node_member_declaration>;
using AST_Node_initializerPtr = std::unique_ptr<AST_Node_initializer>;
using AST_Node_expPtr = std::unique_ptr<AST_Node_exp>;
using AST_Node_for_initPtr = std::unique_ptr<AST_Node_for_init>;
using AST_Node_statementPtr = std::unique_ptr<AST_Node_statement>;
using AST_Node_expression_statementPtr = std::unique_ptr<AST_Node_expression_statement>;
using AST_Node_return_statementPtr = std::unique_ptr<AST_Node_return_statement>;
using AST_Node_for_statementPtr = std::unique_ptr<AST_Node_for_statement>;
using AST_Node_while_statementPtr = std::unique_ptr<AST_Node_while_statement>;
using AST_Node_do_while_statementPtr = std::unique_ptr<AST_Node_do_while_statement>;
using AST_Node_if_statementPtr = std::unique_ptr<AST_Node_if_statement>;
using AST_Node_labeled_statementPtr = std::unique_ptr<AST_Node_labeled_statement>;
using AST_Node_switch_statementPtr = std::unique_ptr<AST_Node_switch_statement>;
using AST_Node_blockPtr = std::unique_ptr<AST_Node_block>;
using AST_Node_block_itemPtr = std::unique_ptr<AST_Node_block_item>;
using AST_Node_dotPtr = std::unique_ptr<AST_Node_dot>;
using AST_Node_arrowPtr = std::unique_ptr<AST_Node_arrow>;
using AST_Node_subscriptPtr = std::unique_ptr<AST_Node_subscript>;
using AST_Node_function_callPtr = std::unique_ptr<AST_Node_function_call>;
using AST_Node_conditionalPtr = std::unique_ptr<AST_Node_conditional>;
using AST_Node_assignmentPtr = std::unique_ptr<AST_Node_assignment>;
using AST_Node_binary_expPtr = std::unique_ptr<AST_Node_binary_exp>;
using AST_Node_unary_expPtr = std::unique_ptr<AST_Node_unary_exp>;
using AST_Node_castPtr = std::unique_ptr<AST_Node_cast>;
using AST_Node_varPtr = std::unique_ptr<AST_Node_var>;
using AST_Node_stringPtr = std::unique_ptr<AST_Node_string>;
using AST_Node_constPtr = std::unique_ptr<AST_Node_const>;
using AST_Node_type_structurePtr = std::unique_ptr<AST_Node_type_structure>;
using AST_Node_type_pointerPtr = std::unique_ptr<AST_Node_type_pointer>;
using AST_Node_type_arrayPtr = std::unique_ptr<AST_Node_type_array>;
using AST_Node_type_functionPtr = std::unique_ptr<AST_Node_type_function>;
using AST_Node_unary_operatorPtr = std::unique_ptr<AST_Node_unary_operator>;
using AST_Node_binary_operatorPtr = std::unique_ptr<AST_Node_binary_operator>;
using AST_Node_VaStartPtr = std::unique_ptr<AST_Node_VaStart>;
using AST_Node_VaEndPtr = std::unique_ptr<AST_Node_VaEnd>;
using AST_Node_VaArgPtr = std::unique_ptr<AST_Node_VaArg>;
using AST_Node_labelPtr = std::unique_ptr<AST_Node_label>;
using AST_Node_enumeratorPtr = std::unique_ptr<AST_Node_enumerator>;
using AST_Node_typedef_entryPtr = std::unique_ptr<AST_Node_typedef_entry>;
using AST_Node_field_memberPtr = std::unique_ptr<AST_Node_field_member>;
using AST_Node_method_memberPtr = std::unique_ptr<AST_Node_method_member>;
using AST_Node_constructor_memberPtr = std::unique_ptr<AST_Node_constructor_member>;
using AST_Node_class_memberPtr = std::unique_ptr<AST_Node_class_member>;
using AST_Node_array_sizePtr = std::unique_ptr<AST_Node_array_size>;

// unary operators to be handled separately while parsing
int GetPrecedence(TokenType token);
std::pair<int, int> GetBindingPower(TokenType token);
int GetUnaryPrecedence(TokenType token);
std::pair<int, int> GetUnaryBindingPower(TokenType token);

struct ParserErrorInfo{
  int lineNumber;
  int columnNumber;
  TokenType expectedToken;
  TokenType actualToken;

  ParserErrorInfo(int line, int column, TokenType expected, TokenType actual)
      : lineNumber(line), columnNumber(column), expectedToken(expected), actualToken(actual) {}
};

class Parser {
  public:
    Parser(const std::vector<Token>& tokens);
    void parseProgram();
    AST_Node_programPtr program;
    
    // Expression parsing - public for testing
    AST_Node_expPtr parseExpression(int min_bp = 0);

  private:
    std::vector<Token> tokens;
    size_t tokenSize = 0;
    size_t currentIndex = tokenSize-1; // Add this for parsing position
    bool success = 1;
    std::vector<ParserErrorInfo> errors;

    // advances the index
    Token get();
    // token at current index
    Token peek();
    Token peekNext();
    Token consume();
    void reset();
    bool expect(TokenType expected, TokenType actual);

    AST_Node_declarationPtr parseDeclaration();
    // declaration
    AST_Node_variable_declarationPtr parseVariableDeclaration();
    AST_Node_function_declarationPtr parseFunctionDeclaration();
    AST_Node_struct_declarationPtr parseStructDeclaration();

    AST_Node_typePtr parseType();

    AST_Node_member_declarationPtr parseMemberDeclaration(){ return nullptr; }
    AST_Node_initializerPtr parseInitializer();

    // to be handled using pratt

private:
    AST_Node_expPtr parsePrimary();

    AST_Node_for_initPtr parseForInit();

    AST_Node_statementPtr parseStatement();

    // Node statement
    AST_Node_expression_statementPtr parseExpressionStatement();
    AST_Node_return_statementPtr parseReturnStatement();
    AST_Node_for_statementPtr parseForStatement();
    AST_Node_while_statementPtr parseWhileStatement();
    AST_Node_do_while_statementPtr parseDoWhileStatement();
    AST_Node_if_statementPtr parseIfStatement();
    AST_Node_blockPtr parseBlock();

    // block
    AST_Node_block_itemPtr parseBlockItem();

    AST_Node_dotPtr parseDotExpression();
    AST_Node_arrowPtr parseArrowExpression(); 
    AST_Node_subscriptPtr parseSubscriptExpression();
    AST_Node_function_callPtr parseFunctionCallExpression();
    AST_Node_conditionalPtr parseConditionalExpression();
    AST_Node_assignmentPtr parseAssignmentExpression();
    AST_Node_binary_expPtr parseBinaryExpression();
    AST_Node_unary_expPtr parseUnaryExpression();
    AST_Node_castPtr parseCastExpression();
    AST_Node_varPtr parseVariable();
    AST_Node_stringPtr parseStringLiteral();
    // int constant, double const .. 
    AST_Node_constPtr parseConstant();

    AST_Node_stringPtr parseString();
    AST_Node_type_structurePtr parseTypeStructure();
    AST_Node_type_pointerPtr parseTypePointer();
    AST_Node_type_arrayPtr parseTypeArray();
    AST_Node_type_functionPtr parseTypeFunction();
    AST_Node_unary_operatorPtr parseUnaryOperator();
    AST_Node_binary_operatorPtr parseBinaryOperator();

    // // Missing parse functions for additional AST node types
    AST_Node_union_declarationPtr parseUnionDeclaration();
    AST_Node_enum_declarationPtr parseEnumDeclaration();
    AST_Node_typedef_declarationPtr parseTypedefDeclaration();
    AST_Node_class_declarationPtr parseClassDeclaration();
    AST_Node_enumeratorPtr parseEnumerator();
    AST_Node_typedef_entryPtr parseTypedefEntry();
    
    // Class member parsing functions
    AST_Node_field_memberPtr parseFieldMember();
    AST_Node_method_memberPtr parseMethodMember();
    AST_Node_constructor_memberPtr parseConstructorMember();
    AST_Node_class_memberPtr parseClassMember();
    
    // Statement parsing functions
    AST_Node_labeled_statementPtr parseLabeledStatement();
    AST_Node_switch_statementPtr parseSwitchStatement();
    AST_Node_labelPtr parseLabel();
    
    // Variadic argument functions
    AST_Node_VaStartPtr parseVaStart();
    AST_Node_VaEndPtr parseVaEnd();
    AST_Node_VaArgPtr parseVaArg();
    
    // Array size parsing
    AST_Node_array_sizePtr parseArraySize();
};