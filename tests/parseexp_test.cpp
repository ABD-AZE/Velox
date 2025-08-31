#include <gtest/gtest.h>
#include "../parser/parser.hpp"
#include <memory>
#include <vector>

// Mock implementations for missing parser functions to avoid linker errors
AST_Node_class_declarationPtr Parser::parseClassDeclaration() { return nullptr; }
AST_Node_enum_declarationPtr Parser::parseEnumDeclaration() { return nullptr; }
AST_Node_typedef_declarationPtr Parser::parseTypedefDeclaration() { return nullptr; }
AST_Node_union_declarationPtr Parser::parseUnionDeclaration() { return nullptr; }
AST_Node_function_declarationPtr Parser::parseFunctionDeclaration() { return nullptr; }
AST_Node_variable_declarationPtr Parser::parseVariableDeclaration() { return nullptr; }
AST_Node_typePtr Parser::parseType() { return nullptr; }

class ParseExpressionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }

    // Helper function to create tokens manually for testing
    std::vector<Token> createTokens(const std::vector<std::pair<TokenType, std::string>>& token_data) {
        std::vector<Token> tokens;
        for (const auto& [type, lexeme] : token_data) {
            Token token;
            token.SetType(type);
            token.SetLineNumber(1);
            token.SetColumnNumber(1);
            // Set lexeme by pushing characters
            for (char c : lexeme) {
                token.push(c);
            }
            tokens.push_back(token);
        }
        // Add EOF token
        Token eof_token;
        eof_token.SetType(TokenType::END_OF_FILE);
        eof_token.SetLineNumber(1);
        eof_token.SetColumnNumber(1);
        tokens.push_back(eof_token);
        return tokens;
    }

    // Helper function to parse expression from tokens
    AST_Node_expPtr parseExpressionFromTokens(const std::vector<Token>& tokens) {
        if (tokens.empty()) {
            return nullptr;
        }
        
        Parser parser(tokens);
        return parser.parseExpression();
    }
};

// ============================================================================
// BASIC EXPRESSION PARSING TESTS
// ============================================================================

TEST_F(ParseExpressionTest, ParseIdentifier) {
    auto tokens = createTokens({{TokenType::IDENTIFIER, "variable"}});
    auto expr = parseExpressionFromTokens(tokens);
    
    ASSERT_NE(expr, nullptr);
    EXPECT_EQ(expr->expr_type, ExprType::ExprVar);
    
    // Access variable node using std::get
    auto& var_node = std::get<std::unique_ptr<AST_Node_var>>(expr->info);
    ASSERT_NE(var_node, nullptr);
    EXPECT_EQ(var_node->identifier, "variable");
}

TEST_F(ParseExpressionTest, ParseIntegerConstant) {
    auto tokens = createTokens({{TokenType::INT_CONSTANT, "42"}});
    auto expr = parseExpressionFromTokens(tokens);
    
    ASSERT_NE(expr, nullptr);
    EXPECT_EQ(expr->expr_type, ExprType::ExprConstant);
    
    // Access constant node using std::get
    auto& const_node = std::get<std::unique_ptr<AST_Node_const>>(expr->info);
    ASSERT_NE(const_node, nullptr);
    EXPECT_EQ(const_node->type, ConstType::ConstInt);
    EXPECT_EQ(const_node->value.intValue, 42);
}

TEST_F(ParseExpressionTest, ParseStringLiteral) {
    auto tokens = createTokens({{TokenType::STRING, "\"hello\""}});
    auto expr = parseExpressionFromTokens(tokens);
    
    ASSERT_NE(expr, nullptr);
    EXPECT_EQ(expr->expr_type, ExprType::ExprString);
    
    // Access string node using std::get
    auto& string_node = std::get<std::unique_ptr<AST_Node_string>>(expr->info);
    ASSERT_NE(string_node, nullptr);
    EXPECT_EQ(string_node->value, "\"hello\"");
}

TEST_F(ParseExpressionTest, ParseUnaryMinus) {
    auto tokens = createTokens({
        {TokenType::HYPHEN, "-"},
        {TokenType::INT_CONSTANT, "42"}
    });
    auto expr = parseExpressionFromTokens(tokens);
    
    ASSERT_NE(expr, nullptr);
    EXPECT_EQ(expr->expr_type, ExprType::ExprUnary);
}

TEST_F(ParseExpressionTest, ParseLogicalNot) {
    auto tokens = createTokens({
        {TokenType::NOT, "!"},
        {TokenType::IDENTIFIER, "flag"}
    });
    auto expr = parseExpressionFromTokens(tokens);
    
    ASSERT_NE(expr, nullptr);
    EXPECT_EQ(expr->expr_type, ExprType::ExprUnary);
}

TEST_F(ParseExpressionTest, ParseDereference) {
    auto tokens = createTokens({
        {TokenType::ASTERISK, "*"},
        {TokenType::IDENTIFIER, "ptr"}
    });
    auto expr = parseExpressionFromTokens(tokens);
    
    ASSERT_NE(expr, nullptr);
    EXPECT_EQ(expr->expr_type, ExprType::ExprDereference);
}

TEST_F(ParseExpressionTest, ParseAddressOf) {
    auto tokens = createTokens({
        {TokenType::AAND, "&"},
        {TokenType::IDENTIFIER, "variable"}
    });
    auto expr = parseExpressionFromTokens(tokens);
    
    ASSERT_NE(expr, nullptr);
    EXPECT_EQ(expr->expr_type, ExprType::ExprAddrof);
}

TEST_F(ParseExpressionTest, ParseFunctionCall) {
    auto tokens = createTokens({
        {TokenType::IDENTIFIER, "func"},
        {TokenType::OPEN_PARENTHESES, "("},
        {TokenType::CLOSE_PARENTHESES, ")"}
    });
    auto expr = parseExpressionFromTokens(tokens);
    
    ASSERT_NE(expr, nullptr);
    EXPECT_EQ(expr->expr_type, ExprType::ExprFunctionCall);
}

TEST_F(ParseExpressionTest, ParseArraySubscript) {
    auto tokens = createTokens({
        {TokenType::IDENTIFIER, "array"},
        {TokenType::OPEN_BRACKET, "["},
        {TokenType::IDENTIFIER, "index"},
        {TokenType::CLOSE_BRACKET, "]"}
    });
    auto expr = parseExpressionFromTokens(tokens);
    
    ASSERT_NE(expr, nullptr);
    EXPECT_EQ(expr->expr_type, ExprType::ExprSubscript);
}

TEST_F(ParseExpressionTest, ParseMemberAccess) {
    auto tokens = createTokens({
        {TokenType::IDENTIFIER, "obj"},
        {TokenType::DOT, "."},
        {TokenType::IDENTIFIER, "member"}
    });
    auto expr = parseExpressionFromTokens(tokens);
    
    ASSERT_NE(expr, nullptr);
    EXPECT_EQ(expr->expr_type, ExprType::ExprDot);
}

TEST_F(ParseExpressionTest, ParseBinaryAddition) {
    auto tokens = createTokens({
        {TokenType::IDENTIFIER, "a"},
        {TokenType::PLUS, "+"},
        {TokenType::IDENTIFIER, "b"}
    });
    auto expr = parseExpressionFromTokens(tokens);
    
    ASSERT_NE(expr, nullptr);
    EXPECT_EQ(expr->expr_type, ExprType::ExprBinary);
    
    // Access binary expression node using std::get
    auto& binary_node = std::get<std::unique_ptr<AST_Node_binary_exp>>(expr->info);
    ASSERT_NE(binary_node, nullptr);
    EXPECT_EQ(binary_node->op->op, BinOpType::BinOpAdd);
    
    // Check left operand
    EXPECT_EQ(binary_node->left->expr_type, ExprType::ExprVar);
    auto& left_var = std::get<std::unique_ptr<AST_Node_var>>(binary_node->left->info);
    EXPECT_EQ(left_var->identifier, "a");
    
    // Check right operand
    EXPECT_EQ(binary_node->right->expr_type, ExprType::ExprVar);
    auto& right_var = std::get<std::unique_ptr<AST_Node_var>>(binary_node->right->info);
    EXPECT_EQ(right_var->identifier, "b");
}

TEST_F(ParseExpressionTest, ParseBinaryMultiplication) {
    auto tokens = createTokens({
        {TokenType::IDENTIFIER, "a"},
        {TokenType::ASTERISK, "*"},
        {TokenType::IDENTIFIER, "b"}
    });
    auto expr = parseExpressionFromTokens(tokens);
    
    ASSERT_NE(expr, nullptr);
    EXPECT_EQ(expr->expr_type, ExprType::ExprBinary);
}

TEST_F(ParseExpressionTest, ParseAssignment) {
    auto tokens = createTokens({
        {TokenType::IDENTIFIER, "a"},
        {TokenType::ASSIGNMENT, "="},
        {TokenType::IDENTIFIER, "b"}
    });
    auto expr = parseExpressionFromTokens(tokens);
    
    ASSERT_NE(expr, nullptr);
    EXPECT_EQ(expr->expr_type, ExprType::ExprAssignment);
}

TEST_F(ParseExpressionTest, ParseTernaryConditional) {
    auto tokens = createTokens({
        {TokenType::IDENTIFIER, "condition"},
        {TokenType::QUESTION_MARK, "?"},
        {TokenType::IDENTIFIER, "true_val"},
        {TokenType::COLON, ":"},
        {TokenType::IDENTIFIER, "false_val"}
    });
    auto expr = parseExpressionFromTokens(tokens);
    
    ASSERT_NE(expr, nullptr);
    EXPECT_EQ(expr->expr_type, ExprType::ExprConditional);
    
    // Access conditional node using std::get
    auto& conditional_node = std::get<std::unique_ptr<AST_Node_conditional>>(expr->info);
    ASSERT_NE(conditional_node, nullptr);
    
    // Check condition
    EXPECT_EQ(conditional_node->condition->expr_type, ExprType::ExprVar);
    auto& condition_var = std::get<std::unique_ptr<AST_Node_var>>(conditional_node->condition->info);
    EXPECT_EQ(condition_var->identifier, "condition");
    
    // Check true expression
    EXPECT_EQ(conditional_node->true_expr->expr_type, ExprType::ExprVar);
    auto& true_var = std::get<std::unique_ptr<AST_Node_var>>(conditional_node->true_expr->info);
    EXPECT_EQ(true_var->identifier, "true_val");
    
    // Check false expression
    EXPECT_EQ(conditional_node->false_expr->expr_type, ExprType::ExprVar);
    auto& false_var = std::get<std::unique_ptr<AST_Node_var>>(conditional_node->false_expr->info);
    EXPECT_EQ(false_var->identifier, "false_val");
}

// ============================================================================
// PRECEDENCE TESTS
// ============================================================================

TEST_F(ParseExpressionTest, MultiplicationPrecedenceOverAddition) {
    // a + b * c should parse as a + (b * c), so top level should be addition
    auto tokens = createTokens({
        {TokenType::IDENTIFIER, "a"},
        {TokenType::PLUS, "+"},
        {TokenType::IDENTIFIER, "b"},
        {TokenType::ASTERISK, "*"},
        {TokenType::IDENTIFIER, "c"}
    });
    auto expr = parseExpressionFromTokens(tokens);
    
    ASSERT_NE(expr, nullptr);
    EXPECT_EQ(expr->expr_type, ExprType::ExprBinary);
}

TEST_F(ParseExpressionTest, AssignmentWithArithmetic) {
    // Test assignment with arithmetic expression: a = b + c * d
    auto tokens = createTokens({
        {TokenType::IDENTIFIER, "a"},
        {TokenType::ASSIGNMENT, "="},
        {TokenType::IDENTIFIER, "b"},
        {TokenType::PLUS, "+"},
        {TokenType::IDENTIFIER, "c"},
        {TokenType::ASTERISK, "*"},
        {TokenType::IDENTIFIER, "d"}
    });
    auto expr = parseExpressionFromTokens(tokens);
    
    ASSERT_NE(expr, nullptr);
    EXPECT_EQ(expr->expr_type, ExprType::ExprAssignment);
    
    // Access assignment node using std::get
    auto& assignment = std::get<std::unique_ptr<AST_Node_assignment>>(expr->info);
    ASSERT_NE(assignment, nullptr);
    
    // Check left side (should be variable 'a')
    EXPECT_EQ(assignment->left->expr_type, ExprType::ExprVar);
    auto& left_var = std::get<std::unique_ptr<AST_Node_var>>(assignment->left->info);
    EXPECT_EQ(left_var->identifier, "a");
    
    // Check assignment operator
    EXPECT_EQ(assignment->op, AssignOpType::Assign);
    
    // Check right side (should be binary expression 'b + c * d')
    EXPECT_EQ(assignment->right->expr_type, ExprType::ExprBinary);
    auto& right_binary = std::get<std::unique_ptr<AST_Node_binary_exp>>(assignment->right->info);
    EXPECT_EQ(right_binary->op->op, BinOpType::BinOpAdd);
    
    // Check left operand of addition (should be 'b')
    EXPECT_EQ(right_binary->left->expr_type, ExprType::ExprVar);
    auto& b_var = std::get<std::unique_ptr<AST_Node_var>>(right_binary->left->info);
    EXPECT_EQ(b_var->identifier, "b");
    
    // Check right operand of addition (should be 'c * d')
    EXPECT_EQ(right_binary->right->expr_type, ExprType::ExprBinary);
    auto& mult_binary = std::get<std::unique_ptr<AST_Node_binary_exp>>(right_binary->right->info);
    EXPECT_EQ(mult_binary->op->op, BinOpType::BinOpMul);
    
    // Check operands of multiplication
    EXPECT_EQ(mult_binary->left->expr_type, ExprType::ExprVar);
    auto& c_var = std::get<std::unique_ptr<AST_Node_var>>(mult_binary->left->info);
    EXPECT_EQ(c_var->identifier, "c");
    
    EXPECT_EQ(mult_binary->right->expr_type, ExprType::ExprVar);
    auto& d_var = std::get<std::unique_ptr<AST_Node_var>>(mult_binary->right->info);
    EXPECT_EQ(d_var->identifier, "d");
}

TEST_F(ParseExpressionTest, FunctionCallArrayAccess) {
    // func()[0] should parse as (func())[0], so top level should be subscript
    auto tokens = createTokens({
        {TokenType::IDENTIFIER, "func"},
        {TokenType::OPEN_PARENTHESES, "("},
        {TokenType::CLOSE_PARENTHESES, ")"},
        {TokenType::OPEN_BRACKET, "["},
        {TokenType::INT_CONSTANT, "0"},
        {TokenType::CLOSE_BRACKET, "]"}
    });
    auto expr = parseExpressionFromTokens(tokens);
    
    ASSERT_NE(expr, nullptr);
    EXPECT_EQ(expr->expr_type, ExprType::ExprSubscript);
}
