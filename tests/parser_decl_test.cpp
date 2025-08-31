#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include "../parser/parser.hpp"
#include "../lexer/lexer.hpp"
#include "../token/token.hpp"

class ParseDeclarationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup common test data if needed
    }

    void TearDown() override {
        // Cleanup after each test
    }

    // Helper function to create tokens from a string
    std::vector<Token> tokenize(const std::string& input) {
        Lexer lexer(input);
        std::vector<Token> tokens;
        Token token;
        do {
            token = lexer.GetNextToken();
            tokens.push_back(token);
        } while (token.GetType() != TokenType::END_OF_FILE);
        return tokens;
    }

    // Helper function to create parser and parse declaration
    AST_Node_declarationPtr parseDeclarationFromString(const std::string& input) {
        auto tokens = tokenize(input);
        Parser parser(tokens);
        return parser.parseDeclaration();
    }
};

// Test parsing simple variable declarations
TEST_F(ParseDeclarationTest, ParseSimpleVariableDeclarations) {
    // Test basic int declaration
    {
        auto decl = parseDeclarationFromString("int x;");
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->decl_type, DeclType::DeclVariable);
        ASSERT_NE(decl->info.variable, nullptr);
        EXPECT_EQ(decl->info.variable->identifier, "x");
        EXPECT_EQ(decl->info.variable->var_type->type, TypeType::TypeInt);
    }

    // Test char declaration
    {
        auto decl = parseDeclarationFromString("char c;");
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->decl_type, DeclType::DeclVariable);
        ASSERT_NE(decl->info.variable, nullptr);
        EXPECT_EQ(decl->info.variable->identifier, "c");
        EXPECT_EQ(decl->info.variable->var_type->type, TypeType::TypeChar);
    }

    // Test double declaration
    {
        auto decl = parseDeclarationFromString("double d;");
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->decl_type, DeclType::DeclVariable);
        ASSERT_NE(decl->info.variable, nullptr);
        EXPECT_EQ(decl->info.variable->identifier, "d");
        EXPECT_EQ(decl->info.variable->var_type->type, TypeType::TypeDouble);
    }
}

// Test parsing complex type specifiers
TEST_F(ParseDeclarationTest, ParseComplexTypeSpecifiers) {
    // Test unsigned long
    {
        auto decl = parseDeclarationFromString("unsigned long ul;");
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->decl_type, DeclType::DeclVariable);
        ASSERT_NE(decl->info.variable, nullptr);
        EXPECT_EQ(decl->info.variable->identifier, "ul");
        EXPECT_EQ(decl->info.variable->var_type->type, TypeType::TypeULong);
    }

    // Test const int
    {
        auto decl = parseDeclarationFromString("const int ci;");
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->decl_type, DeclType::DeclVariable);
        ASSERT_NE(decl->info.variable, nullptr);
        EXPECT_EQ(decl->info.variable->identifier, "ci");
        EXPECT_EQ(decl->info.variable->var_type->type, TypeType::TypeInt);
        EXPECT_TRUE(decl->info.variable->var_type->is_const);
    }

    // Test static extern unsigned long
    {
        auto decl = parseDeclarationFromString("static extern unsigned long seul;");
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->decl_type, DeclType::DeclVariable);
        ASSERT_NE(decl->info.variable, nullptr);
        EXPECT_EQ(decl->info.variable->identifier, "seul");
        EXPECT_EQ(decl->info.variable->var_type->type, TypeType::TypeULong);
        EXPECT_TRUE(decl->info.variable->var_type->is_static);
        EXPECT_TRUE(decl->info.variable->var_type->is_extern);
    }
}

// Test parsing variable declarations with initializers
TEST_F(ParseDeclarationTest, ParseVariableDeclarationsWithInitializers) {
    // Test simple initializer
    {
        auto decl = parseDeclarationFromString("int x = 42;");
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->decl_type, DeclType::DeclVariable);
        ASSERT_NE(decl->info.variable, nullptr);
        EXPECT_EQ(decl->info.variable->identifier, "x");
        EXPECT_TRUE(decl->info.variable->initializer.has_value());
        ASSERT_NE(decl->info.variable->initializer.value(), nullptr);
        EXPECT_EQ(decl->info.variable->initializer.value()->init_type, InitializerType::InitializerExp);
    }

    // Test unsigned long with initializer
    {
        auto decl = parseDeclarationFromString("unsigned long a = 10;");
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->decl_type, DeclType::DeclVariable);
        ASSERT_NE(decl->info.variable, nullptr);
        EXPECT_EQ(decl->info.variable->identifier, "a");
        EXPECT_EQ(decl->info.variable->var_type->type, TypeType::TypeULong);
        EXPECT_TRUE(decl->info.variable->initializer.has_value());
    }
}

// Test parsing function declarations
TEST_F(ParseDeclarationTest, ParseFunctionDeclarations) {
    // Test simple function declaration
    {
        auto decl = parseDeclarationFromString("int func();");
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->decl_type, DeclType::DeclFunction);
        ASSERT_NE(decl->info.function, nullptr);
        EXPECT_EQ(decl->info.function->identifier, "func");
        EXPECT_EQ(decl->info.function->fun_type->type, TypeType::TypeFunction);
    }

    // Test function with parameters
    {
        auto decl = parseDeclarationFromString("void process(int x, char c);");
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->decl_type, DeclType::DeclFunction);
        ASSERT_NE(decl->info.function, nullptr);
        EXPECT_EQ(decl->info.function->identifier, "process");
        EXPECT_EQ(decl->info.function->params.size(), 2);
        EXPECT_EQ(decl->info.function->params[0], "x");
        EXPECT_EQ(decl->info.function->params[1], "c");
    }

    // Test function with body
    {
        auto decl = parseDeclarationFromString("int main() { return 0; }");
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->decl_type, DeclType::DeclFunction);
        ASSERT_NE(decl->info.function, nullptr);
        EXPECT_EQ(decl->info.function->identifier, "main");
        EXPECT_TRUE(decl->info.function->body.has_value());
    }
}

// Test parsing struct declarations
TEST_F(ParseDeclarationTest, ParseStructDeclarations) {
    // Test simple struct declaration
    {
        auto decl = parseDeclarationFromString("struct Point;");
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->decl_type, DeclType::DeclStruct);
        ASSERT_NE(decl->info.structure, nullptr);
        EXPECT_EQ(decl->info.structure->tag, "Point");
        EXPECT_EQ(decl->info.structure->members.size(), 0);
    }

    // Test struct with members
    {
        auto decl = parseDeclarationFromString("struct Point { int x; int y; };");
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->decl_type, DeclType::DeclStruct);
        ASSERT_NE(decl->info.structure, nullptr);
        EXPECT_EQ(decl->info.structure->tag, "Point");
        EXPECT_EQ(decl->info.structure->members.size(), 2);
        EXPECT_EQ(decl->info.structure->members[0]->identifier, "x");
        EXPECT_EQ(decl->info.structure->members[1]->identifier, "y");
    }
}

// Test parsing class declarations
TEST_F(ParseDeclarationTest, ParseClassDeclarations) {
    // Test simple class declaration
    {
        auto decl = parseDeclarationFromString("class MyClass;");
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->decl_type, DeclType::DeclClass);
        ASSERT_NE(decl->info.class_decl, nullptr);
        EXPECT_EQ(decl->info.class_decl->name, "MyClass");
    }

    // Test class with members
    {
        auto decl = parseDeclarationFromString("class MyClass { public: int x; private: void method(); };");
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->decl_type, DeclType::DeclClass);
        ASSERT_NE(decl->info.class_decl, nullptr);
        EXPECT_EQ(decl->info.class_decl->name, "MyClass");
        EXPECT_GT(decl->info.class_decl->members.size(), 0);
    }
}

// Test parsing enum declarations
TEST_F(ParseDeclarationTest, ParseEnumDeclarations) {
    // Test simple enum declaration
    {
        auto decl = parseDeclarationFromString("enum Color;");
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->decl_type, DeclType::DeclEnum);
        ASSERT_NE(decl->info.enum_decl, nullptr);
        EXPECT_TRUE(decl->info.enum_decl->tag.has_value());
        EXPECT_EQ(decl->info.enum_decl->tag.value(), "Color");
    }

    // Test enum with values
    {
        auto decl = parseDeclarationFromString("enum Color { RED, GREEN, BLUE };");
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->decl_type, DeclType::DeclEnum);
        ASSERT_NE(decl->info.enum_decl, nullptr);
        EXPECT_EQ(decl->info.enum_decl->enumerators.size(), 3);
        EXPECT_EQ(decl->info.enum_decl->enumerators[0]->name, "RED");
        EXPECT_EQ(decl->info.enum_decl->enumerators[1]->name, "GREEN");
        EXPECT_EQ(decl->info.enum_decl->enumerators[2]->name, "BLUE");
    }
}

// Test parsing union declarations
TEST_F(ParseDeclarationTest, ParseUnionDeclarations) {
    // Test simple union declaration
    {
        auto decl = parseDeclarationFromString("union Data;");
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->decl_type, DeclType::DeclUnion);
        ASSERT_NE(decl->info.union_decl, nullptr);
        EXPECT_EQ(decl->info.union_decl->tag, "Data");
    }

    // Test union with members
    {
        auto decl = parseDeclarationFromString("union Data { int i; float f; };");
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->decl_type, DeclType::DeclUnion);
        ASSERT_NE(decl->info.union_decl, nullptr);
        EXPECT_EQ(decl->info.union_decl->tag, "Data");
        EXPECT_EQ(decl->info.union_decl->members.size(), 2);
    }
}

// Test parsing typedef declarations
TEST_F(ParseDeclarationTest, ParseTypedefDeclarations) {
    // Test simple typedef
    {
        auto decl = parseDeclarationFromString("typedef int Integer;");
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->decl_type, DeclType::DeclTypedef);
        ASSERT_NE(decl->info.typedef_decl, nullptr);
        EXPECT_EQ(decl->info.typedef_decl->entries.size(), 1);
        EXPECT_EQ(decl->info.typedef_decl->entries[0]->name, "Integer");
    }

    // Test typedef with struct
    {
        auto decl = parseDeclarationFromString("typedef struct Point PointType;");
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->decl_type, DeclType::DeclTypedef);
        ASSERT_NE(decl->info.typedef_decl, nullptr);
        EXPECT_EQ(decl->info.typedef_decl->entries[0]->name, "PointType");
    }
}

// Test error cases
TEST_F(ParseDeclarationTest, ParseDeclarationErrorCases) {
    // Test invalid token at start
    {
        auto decl = parseDeclarationFromString("123 invalid;");
        // Should handle error gracefully - exact behavior depends on error handling implementation
        // This test ensures the parser doesn't crash on invalid input
    }

    // Test missing semicolon
    {
        auto decl = parseDeclarationFromString("int x");
        // Should handle missing semicolon error
    }

    // Test empty input
    {
        auto decl = parseDeclarationFromString("");
        // Should handle empty input gracefully
    }
}

// Test struct variable declarations (not struct definitions)
TEST_F(ParseDeclarationTest, ParseStructVariableDeclarations) {
    // Test struct variable declaration
    {
        auto decl = parseDeclarationFromString("struct Point p;");
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->decl_type, DeclType::DeclVariable);
        ASSERT_NE(decl->info.variable, nullptr);
        EXPECT_EQ(decl->info.variable->identifier, "p");
        EXPECT_EQ(decl->info.variable->var_type->type, TypeType::TypeStructure);
    }

    // Test struct variable with initializer
    {
        auto decl = parseDeclarationFromString("struct Point p = {1, 2};");
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->decl_type, DeclType::DeclVariable);
        ASSERT_NE(decl->info.variable, nullptr);
        EXPECT_EQ(decl->info.variable->identifier, "p");
        EXPECT_TRUE(decl->info.variable->initializer.has_value());
    }
}