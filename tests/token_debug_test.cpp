#include <gtest/gtest.h>
#include "../parser/parser.hpp"
#include <memory>
#include <vector>
#include <iostream>

// Mock implementations for missing parser functions to avoid linker errors
AST_Node_class_declarationPtr Parser::parseClassDeclaration() { return nullptr; }
AST_Node_enum_declarationPtr Parser::parseEnumDeclaration() { return nullptr; }
AST_Node_typedef_declarationPtr Parser::parseTypedefDeclaration() { return nullptr; }
AST_Node_union_declarationPtr Parser::parseUnionDeclaration() { return nullptr; }
AST_Node_function_declarationPtr Parser::parseFunctionDeclaration() { return nullptr; }
AST_Node_variable_declarationPtr Parser::parseVariableDeclaration() { return nullptr; }
AST_Node_typePtr Parser::parseType() { return nullptr; }

class TokenDebugTest : public ::testing::Test {
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
        return tokens;
    }

    // Test token type values
    void testTokenType(TokenType expected_type, const std::string& type_name) {
        std::cout << "Testing " << type_name << " (value=" << static_cast<int>(expected_type) << ")" << std::endl;
        
        auto tokens = createTokens({{expected_type, "test_value"}});
        
        std::cout << "Created token with type=" << static_cast<int>(tokens[0].GetType()) 
                  << ", lexeme='" << tokens[0].GetLexeme() << "'" << std::endl;
        
        // Verify the token type is set correctly
        EXPECT_EQ(tokens[0].GetType(), expected_type);
        
        // Test with parser
        Parser parser(tokens);
        auto expr = parser.parseExpression();
        
        if (expr != nullptr) {
            std::cout << "SUCCESS: " << type_name << " parsed correctly!" << std::endl;
            std::cout << "Expression type: " << static_cast<int>(expr->expr_type) << std::endl;
        } else {
            std::cout << "FAILURE: " << type_name << " parsing failed" << std::endl;
        }
        
        std::cout << "---" << std::endl;
    }
};

TEST_F(TokenDebugTest, TestTokenTypeValues) {
    std::cout << "=== Testing Token Type Values ===" << std::endl;
    
    // Test the specific token types we're using
    testTokenType(TokenType::IDENTIFIER, "IDENTIFIER");
    testTokenType(TokenType::INT_CONSTANT, "INT_CONSTANT");
    testTokenType(TokenType::STRING, "STRING");
    testTokenType(TokenType::CHARACTER, "CHARACTER");
    
    // Test some other token types for comparison
    testTokenType(TokenType::PLUS, "PLUS");
    testTokenType(TokenType::HYPHEN, "HYPHEN");
}

TEST_F(TokenDebugTest, TestTokenTypeComparison) {
    std::cout << "=== Testing Token Type Comparison ===" << std::endl;
    
    // Create an IDENTIFIER token
    auto tokens = createTokens({{TokenType::IDENTIFIER, "test_var"}});
    Token test_token = tokens[0];
    
    std::cout << "Test token type: " << static_cast<int>(test_token.GetType()) << std::endl;
    std::cout << "IDENTIFIER enum value: " << static_cast<int>(TokenType::IDENTIFIER) << std::endl;
    
    // Test direct comparison
    bool matches = (test_token.GetType() == TokenType::IDENTIFIER);
    std::cout << "Direct comparison result: " << (matches ? "MATCH" : "NO MATCH") << std::endl;
    
    // Test switch statement behavior
    switch (test_token.GetType()) {
        case TokenType::IDENTIFIER:
            std::cout << "Switch statement: IDENTIFIER case matched!" << std::endl;
            break;
        case TokenType::INT_CONSTANT:
            std::cout << "Switch statement: INT_CONSTANT case matched (unexpected)" << std::endl;
            break;
        default:
            std::cout << "Switch statement: Default case matched (token type not recognized)" << std::endl;
            break;
    }
}

TEST_F(TokenDebugTest, TestParserTokenAccess) {
    std::cout << "=== Testing Parser Token Access ===" << std::endl;
    
    // Create a simple token
    auto tokens = createTokens({{TokenType::IDENTIFIER, "test_var"}});
    
    std::cout << "Original token: Type=" << static_cast<int>(tokens[0].GetType()) 
              << ", Lexeme='" << tokens[0].GetLexeme() << "'" << std::endl;
    
    // Create parser and check if it can access the token correctly
    Parser parser(tokens);
    
    std::cout << "Parser created. Attempting to parse..." << std::endl;
    
    // Try parsing
    auto expr = parser.parseExpression();
    
    if (expr != nullptr) {
        std::cout << "SUCCESS: Expression parsed!" << std::endl;
        std::cout << "Expression type: " << static_cast<int>(expr->expr_type) << std::endl;
        
        // Check if it's the expected type
        if (expr->expr_type == ExprType::ExprVar) {
            std::cout << "Correct expression type: ExprVar" << std::endl;
        } else {
            std::cout << "Unexpected expression type: " << static_cast<int>(expr->expr_type) << std::endl;
        }
    } else {
        std::cout << "FAILURE: Expression parsing returned nullptr" << std::endl;
    }
}
