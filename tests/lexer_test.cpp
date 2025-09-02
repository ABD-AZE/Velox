#include "../lexer/lexer.hpp"
#include "../token/token.hpp"
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <vector>

class LexerTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Setup code for each test
  }

  void TearDown() override {
    // Cleanup code for each test
  }

  // Helper function to read test file content
  std::string readTestFile(const std::string &filename) {
    std::ifstream file("tests/" + filename);
    if (!file.is_open()) {
      throw std::runtime_error("Could not open test file: " + filename);
    }

    std::string content;
    std::string line;
    while (std::getline(file, line)) {
      content += line + "\n";
    }
    return content;
  }

  // Helper function to create a temporary test file
  std::string createTempTestFile(const std::string &content) {
    std::string tempFilename = "/tmp/velox_test_temp.vlx";
    std::ofstream tempFile(tempFilename);
    if (!tempFile.is_open()) {
      throw std::runtime_error("Could not create temporary test file");
    }
    tempFile << content;
    tempFile.close();
    return tempFilename;
  }

  // Helper function to run lexer and get tokens
  std::vector<Token> runLexer(const std::string &content) {
    std::string tempFile = createTempTestFile(content);
    Lexer lexer(tempFile);
    return lexer.GenerateTokens();
  }

  // Helper function to run lexer and check for success
  bool runLexerWithSuccess(const std::string &content) {
    std::string tempFile = createTempTestFile(content);
    Lexer lexer(tempFile);
    lexer.GenerateTokens();
    return lexer.success;
  }

  // Helper function to verify token sequence
  void verifyTokenSequence(const std::vector<Token> &tokens,
                           const std::vector<TokenType> &expectedTypes) {
    ASSERT_EQ(tokens.size(), expectedTypes.size()) << "Token count mismatch";

    for (size_t i = 0; i < tokens.size(); ++i) {
      EXPECT_EQ(tokens[i].GetType(), expectedTypes[i])
          << "Token type mismatch at position " << i << ": expected "
          << TokenTypeToString(expectedTypes[i]) << ", got "
          << TokenTypeToString(tokens[i].GetType()) << " (lexeme: '"
          << tokens[i].GetLexeme() << "')";
    }
  }

  // Helper function to verify specific token
  void verifyToken(const Token &token, TokenType expectedType,
                   const std::string &expectedLexeme) {
    EXPECT_EQ(token.GetType(), expectedType)
        << "Expected token type " << TokenTypeToString(expectedType) << ", got "
        << TokenTypeToString(token.GetType());
    EXPECT_EQ(token.GetLexeme(), expectedLexeme)
        << "Expected lexeme '" << expectedLexeme << "', got '"
        << token.GetLexeme() << "'";
  }
};

// Test 1: Basic keywords and identifiers
TEST_F(LexerTest, Test1_BasicKeywordsAndIdentifiers) {
  std::string content = readTestFile("test1.vlx");
  auto tokens = runLexer(content);

  // Verify lexer succeeded
  EXPECT_TRUE(runLexerWithSuccess(content));

  // Check for presence of key tokens
  bool foundMain = false, foundReturn = false, foundWhile = false;
  bool foundInt = false, foundIf = false, foundElse = false;

  for (const auto &token : tokens) {
    if (token.GetType() == TokenType::INT && token.GetLexeme() == "int")
      foundInt = true;
    if (token.GetType() == TokenType::IDENTIFIER && token.GetLexeme() == "main")
      foundMain = true;
    if (token.GetType() == TokenType::RETURN && token.GetLexeme() == "return")
      foundReturn = true;
    if (token.GetType() == TokenType::WHILE && token.GetLexeme() == "while")
      foundWhile = true;
    if (token.GetType() == TokenType::IF && token.GetLexeme() == "if")
      foundIf = true;
    if (token.GetType() == TokenType::ELSE && token.GetLexeme() == "else")
      foundElse = true;
  }

  EXPECT_TRUE(foundInt) << "Expected to find 'int' keyword";
  EXPECT_TRUE(foundMain) << "Expected to find 'main' identifier";
  EXPECT_TRUE(foundReturn) << "Expected to find 'return' keyword";
  EXPECT_TRUE(foundWhile) << "Expected to find 'while' keyword";
  EXPECT_TRUE(foundIf) << "Expected to find 'if' keyword";
  EXPECT_TRUE(foundElse) << "Expected to find 'else' keyword";
}

// Test 2: Numeric constants
TEST_F(LexerTest, Test2_NumericConstants) {
  std::string content = readTestFile("test2.vlx");
  auto tokens = runLexer(content);

  // Verify lexer succeeded
  EXPECT_TRUE(runLexerWithSuccess(content));

  // Check for different types of numeric constants
  bool foundIntConstant = false, foundUintConstant = false;
  bool foundLongConstant = false, foundUlongConstant = false;
  bool foundFloatConstant = false;

  for (const auto &token : tokens) {
    switch (token.GetType()) {
    case TokenType::INT_CONSTANT:
      foundIntConstant = true;
      break;
    case TokenType::UINT_CONSTANT:
      foundUintConstant = true;
      break;
    case TokenType::LONG_CONSTANT:
      foundLongConstant = true;
      break;
    case TokenType::ULONG_CONSTANT:
      foundUlongConstant = true;
      break;
    case TokenType::FLOAT_CONSTANT:
      foundFloatConstant = true;
      break;
    default:
      break;
    }
  }

  EXPECT_TRUE(foundIntConstant) << "Expected to find integer constants";
  EXPECT_TRUE(foundUintConstant)
      << "Expected to find unsigned integer constants";
  EXPECT_TRUE(foundLongConstant) << "Expected to find long constants";
  EXPECT_TRUE(foundUlongConstant) << "Expected to find unsigned long constants";
  EXPECT_TRUE(foundFloatConstant) << "Expected to find float constants";
}

// Test 3: Operators
TEST_F(LexerTest, Test3_Operators) {
  std::string content = readTestFile("test3.vlx");
  auto tokens = runLexer(content);

  // Verify lexer succeeded
  EXPECT_TRUE(runLexerWithSuccess(content));

  // Check for various operators
  std::vector<TokenType> expectedOperators = {TokenType::PLUS,
                                              TokenType::INCREMENT_OPERATOR,
                                              TokenType::COMPOUND_SUM,
                                              TokenType::HYPHEN,
                                              TokenType::DECREMENT_OPERATOR,
                                              TokenType::COMPOUND_DIFFERENCE,
                                              TokenType::ARROW_OPERATOR,
                                              TokenType::ASTERISK,
                                              TokenType::COMPOUND_PRODUCT,
                                              TokenType::FORWARD_SLASH,
                                              TokenType::COMPOUND_DIVISION,
                                              TokenType::LESSTHAN,
                                              TokenType::LESSTHANEQUAL,
                                              TokenType::LEFT_SHIFT,
                                              TokenType::COMPOUND_LEFTSHIFT,
                                              TokenType::GREATERTHAN,
                                              TokenType::GREATERTHANEQUAL,
                                              TokenType::RIGHT_SHIFT,
                                              TokenType::COMPOUND_RIGHTSHIFT,
                                              TokenType::PERCENT_SIGN,
                                              TokenType::COMPOUND_REMAINDER,
                                              TokenType::DOT,
                                              TokenType::ELLIPSIS};

  for (TokenType expectedOp : expectedOperators) {
    bool found = false;
    for (const auto &token : tokens) {
      if (token.GetType() == expectedOp) {
        found = true;
        break;
      }
    }
    EXPECT_TRUE(found) << "Expected to find operator: "
                       << TokenTypeToString(expectedOp);
  }
}

// Test 4: More operators and expressions
TEST_F(LexerTest, Test4_MoreOperators) {
  std::string content = readTestFile("test4.vlx");
  auto tokens = runLexer(content);

  // Verify lexer succeeded
  EXPECT_TRUE(runLexerWithSuccess(content));

  // This test should contain additional operator combinations
  EXPECT_GT(tokens.size(), 0) << "Expected non-empty token list";
}

// Test 5: Additional operator tests
TEST_F(LexerTest, Test5_AdditionalOperators) {
  std::string content = readTestFile("test5.vlx");
  auto tokens = runLexer(content);

  // Verify lexer succeeded
  EXPECT_TRUE(runLexerWithSuccess(content));

  EXPECT_GT(tokens.size(), 0) << "Expected non-empty token list";
}

// Test 6: Specific operator combinations
TEST_F(LexerTest, Test6_OperatorCombinations) {
  std::string content = readTestFile("test6.vlx");
  auto tokens = runLexer(content);

  // Verify lexer succeeded
  EXPECT_TRUE(runLexerWithSuccess(content));

  EXPECT_GT(tokens.size(), 0) << "Expected non-empty token list";
}

// Test 7: Strings and Characters
TEST_F(LexerTest, Test7_StringsAndChars) {
  std::string content = readTestFile("test7.vlx");
  auto tokens = runLexer(content);

  // Verify lexer succeeded
  EXPECT_TRUE(runLexerWithSuccess(content));

  // Define the expected token sequence for test7.vlx
  std::vector<std::tuple<TokenType, std::string, int, int>> expectedTokens = {
      {TokenType::STRING, "a", 1, 1},
      {TokenType::STRING, "hello", 1, 5},
      {TokenType::STRING, "quote: \" slash: \\ newline:\n tab:\t", 1, 13},
      {TokenType::CHARACTER, "c", 2, 1},
      {TokenType::CHARACTER, "\n", 2, 5},
      {TokenType::CHARACTER, "\t", 2, 10},
      {TokenType::CHARACTER, "\'", 2, 15},
      {TokenType::CHARACTER, "\"", 2, 20}};

  // Verify token count matches exactly
  ASSERT_EQ(tokens.size(), expectedTokens.size())
      << "Token count mismatch: expected " << expectedTokens.size() << ", got "
      << tokens.size();

  // Verify each token matches exactly (type, lexeme, line, column)
  for (size_t i = 0; i < expectedTokens.size(); ++i) {
    const auto &[expectedType, expectedLexeme, expectedLine, expectedCol] =
        expectedTokens[i];
    const auto &actualToken = tokens[i];

    EXPECT_EQ(actualToken.GetType(), expectedType)
        << "Token type mismatch at position " << i << ": expected "
        << TokenTypeToString(expectedType) << ", got "
        << TokenTypeToString(actualToken.GetType()) << " (lexeme: '"
        << actualToken.GetLexeme() << "')";

    EXPECT_EQ(actualToken.GetLexeme(), expectedLexeme)
        << "Token lexeme mismatch at position " << i << ": expected '"
        << expectedLexeme << "', got '" << actualToken.GetLexeme() << "'";

    EXPECT_EQ(actualToken.GetLineNumber(), expectedLine)
        << "Token line mismatch at position " << i << ": expected line "
        << expectedLine << ", got line " << actualToken.GetLineNumber()
        << " (token: '" << actualToken.GetLexeme() << "')";

    EXPECT_EQ(actualToken.GetColumnNumber(), expectedCol)
        << "Token column mismatch at position " << i << ": expected column "
        << expectedCol << ", got column " << actualToken.GetColumnNumber()
        << " (token: '" << actualToken.GetLexeme() << "')";
  }
}

// Test 8: Comprehensive C code test
TEST_F(LexerTest, Test8_ComprehensiveCCode) {
  std::string content = readTestFile("test8.vlx");
  auto tokens = runLexer(content);

  // Verify lexer succeeded
  EXPECT_TRUE(runLexerWithSuccess(content));

  // Define the complete expected token sequence based on the required output
  std::vector<std::tuple<TokenType, std::string, int, int>> expectedTokens = {
      // Line 1
      {TokenType::TYPEDEF, "typedef", 1, 1},
      {TokenType::STRUCT, "struct", 1, 9},
      {TokenType::IDENTIFIER, "Node", 1, 16},
      {TokenType::OPEN_BRACE, "{", 1, 21},
      {TokenType::INT, "int", 1, 23},
      {TokenType::IDENTIFIER, "v", 1, 27},
      {TokenType::SEMICOLON, ";", 1, 28},
      {TokenType::STRUCT, "struct", 1, 30},
      {TokenType::IDENTIFIER, "Node", 1, 37},
      {TokenType::ASTERISK, "*", 1, 41},
      {TokenType::IDENTIFIER, "next", 1, 43},
      {TokenType::SEMICOLON, ";", 1, 47},
      {TokenType::CLOSE_BRACE, "}", 1, 49},
      {TokenType::IDENTIFIER, "Node", 1, 51},
      {TokenType::SEMICOLON, ";", 1, 55},
      // Line 2
      {TokenType::INT, "int", 2, 1},
      {TokenType::IDENTIFIER, "main", 2, 5},
      {TokenType::OPEN_PARENTHESES, "(", 2, 9},
      {TokenType::CLOSE_PARENTHESES, ")", 2, 10},
      {TokenType::OPEN_BRACE, "{", 2, 11},
      // Line 3
      {TokenType::INT, "int", 3, 3},
      {TokenType::IDENTIFIER, "a", 3, 7},
      {TokenType::ASSIGNMENT, "=", 3, 9},
      {TokenType::INT_CONSTANT, "7", 3, 11},
      {TokenType::COMMA, ",", 3, 12},
      {TokenType::IDENTIFIER, "b", 3, 14},
      {TokenType::ASSIGNMENT, "=", 3, 16},
      {TokenType::UINT_CONSTANT, "42U", 3, 18},
      {TokenType::COMMA, ",", 3, 21},
      {TokenType::IDENTIFIER, "c", 3, 23},
      {TokenType::ASSIGNMENT, "=", 3, 25},
      {TokenType::LONG_CONSTANT, "9l", 3, 27},
      {TokenType::COMMA, ",", 3, 29},
      {TokenType::IDENTIFIER, "d", 3, 31},
      {TokenType::ASSIGNMENT, "=", 3, 33},
      {TokenType::ULONG_CONSTANT, "11Ul", 3, 35},
      {TokenType::COMMA, ",", 3, 39},
      {TokenType::IDENTIFIER, "e", 3, 41},
      {TokenType::ASSIGNMENT, "=", 3, 43},
      {TokenType::ULONG_CONSTANT, "13LU", 3, 45},
      {TokenType::COMMA, ",", 3, 49},
      {TokenType::IDENTIFIER, "f", 3, 51},
      {TokenType::ASSIGNMENT, "=", 3, 53},
      {TokenType::INT_CONSTANT, "0", 3, 55},
      {TokenType::COMMA, ",", 3, 56},
      {TokenType::IDENTIFIER, "g", 3, 58},
      {TokenType::ASSIGNMENT, "=", 3, 60},
      {TokenType::INT_CONSTANT, "1", 3, 62},
      {TokenType::SEMICOLON, ";", 3, 63},
      // Line 4
      {TokenType::DOUBLE, "double", 4, 3},
      {TokenType::IDENTIFIER, "x", 4, 10},
      {TokenType::ASSIGNMENT, "=", 4, 12},
      {TokenType::FLOAT_CONSTANT, ".5", 4, 14},
      {TokenType::COMMA, ",", 4, 16},
      {TokenType::IDENTIFIER, "y", 4, 18},
      {TokenType::ASSIGNMENT, "=", 4, 20},
      {TokenType::FLOAT_CONSTANT, "7.", 4, 22},
      {TokenType::COMMA, ",", 4, 24},
      {TokenType::IDENTIFIER, "z", 4, 26},
      {TokenType::ASSIGNMENT, "=", 4, 28},
      {TokenType::FLOAT_CONSTANT, "12.345", 4, 30},
      {TokenType::COMMA, ",", 4, 36},
      {TokenType::IDENTIFIER, "w", 4, 38},
      {TokenType::ASSIGNMENT, "=", 4, 40},
      {TokenType::FLOAT_CONSTANT, ".0", 4, 42},
      {TokenType::SEMICOLON, ";", 4, 44},
      // Line 5
      {TokenType::CHAR, "char", 5, 3},
      {TokenType::IDENTIFIER, "ch", 5, 8},
      {TokenType::ASSIGNMENT, "=", 5, 11},
      {TokenType::CHARACTER, "\n", 5, 13},
      {TokenType::SEMICOLON, ";", 5, 17},
      {TokenType::CHAR, "char", 5, 19},
      {TokenType::IDENTIFIER, "q", 5, 24},
      {TokenType::ASSIGNMENT, "=", 5, 26},
      {TokenType::CHARACTER, "'", 5, 28},
      {TokenType::SEMICOLON, ";", 5, 32},
      {TokenType::CHAR, "char", 5, 34},
      {TokenType::IDENTIFIER, "dq", 5, 39},
      {TokenType::ASSIGNMENT, "=", 5, 42},
      {TokenType::CHARACTER, "\"", 5, 44},
      {TokenType::SEMICOLON, ";", 5, 48},
      {TokenType::CHAR, "char", 5, 50},
      {TokenType::IDENTIFIER, "bs", 5, 55},
      {TokenType::ASSIGNMENT, "=", 5, 58},
      {TokenType::CHARACTER, "\\", 5, 60},
      {TokenType::SEMICOLON, ";", 5, 64},
      // Line 6
      {TokenType::CONST, "const", 6, 3},
      {TokenType::CHAR, "char", 6, 9},
      {TokenType::ASTERISK, "*", 6, 13},
      {TokenType::IDENTIFIER, "s", 6, 15},
      {TokenType::ASSIGNMENT, "=", 6, 17},
      {TokenType::STRING, "hello\tworld\n\"q\" \\\\ ok", 6, 19},
      {TokenType::SEMICOLON, ";", 6, 48},
      // Line 7
      {TokenType::IDENTIFIER, "a", 7, 2},
      {TokenType::ASSIGNMENT, "=", 7, 4},
      {TokenType::IDENTIFIER, "a", 7, 6},
      {TokenType::PLUS, "+", 7, 8},
      {TokenType::INT_CONSTANT, "1", 7, 10},
      {TokenType::SEMICOLON, ";", 7, 11},
      {TokenType::IDENTIFIER, "a", 7, 13},
      {TokenType::COMPOUND_SUM, "+=", 7, 15},
      {TokenType::INT_CONSTANT, "2", 7, 18},
      {TokenType::SEMICOLON, ";", 7, 19},
      {TokenType::INCREMENT_OPERATOR, "++", 7, 21},
      {TokenType::IDENTIFIER, "a", 7, 23},
      {TokenType::SEMICOLON, ";", 7, 24},
      {TokenType::IDENTIFIER, "a", 7, 26},
      {TokenType::INCREMENT_OPERATOR, "++", 7, 27},
      {TokenType::SEMICOLON, ";", 7, 29},
      {TokenType::IDENTIFIER, "b", 7, 31},
      {TokenType::ASSIGNMENT, "=", 7, 33},
      {TokenType::IDENTIFIER, "b", 7, 35},
      {TokenType::HYPHEN, "-", 7, 37},
      {TokenType::INT_CONSTANT, "1", 7, 39},
      {TokenType::SEMICOLON, ";", 7, 40},
      {TokenType::IDENTIFIER, "b", 7, 42},
      {TokenType::COMPOUND_DIFFERENCE, "-=", 7, 44},
      {TokenType::INT_CONSTANT, "3", 7, 47},
      {TokenType::SEMICOLON, ";", 7, 48},
      {TokenType::DECREMENT_OPERATOR, "--", 7, 50},
      {TokenType::IDENTIFIER, "b", 7, 52},
      {TokenType::SEMICOLON, ";", 7, 53},
      {TokenType::IDENTIFIER, "b", 7, 55},
      {TokenType::DECREMENT_OPERATOR, "--", 7, 56},
      {TokenType::SEMICOLON, ";", 7, 58},
      {TokenType::IDENTIFIER, "c", 7, 60},
      {TokenType::ASSIGNMENT, "=", 7, 62},
      {TokenType::IDENTIFIER, "c", 7, 64},
      {TokenType::ASTERISK, "*", 7, 66},
      {TokenType::INT_CONSTANT, "2", 7, 68},
      {TokenType::SEMICOLON, ";", 7, 69},
      {TokenType::IDENTIFIER, "c", 7, 71},
      {TokenType::COMPOUND_PRODUCT, "*=", 7, 73},
      {TokenType::INT_CONSTANT, "2", 7, 76},
      {TokenType::SEMICOLON, ";", 7, 77},
      {TokenType::IDENTIFIER, "d", 7, 79},
      {TokenType::ASSIGNMENT, "=", 7, 81},
      {TokenType::IDENTIFIER, "d", 7, 83},
      {TokenType::FORWARD_SLASH, "/", 7, 85},
      {TokenType::INT_CONSTANT, "2", 7, 87},
      {TokenType::SEMICOLON, ";", 7, 88},
      {TokenType::IDENTIFIER, "d", 7, 90},
      {TokenType::COMPOUND_DIVISION, "/=", 7, 92},
      {TokenType::INT_CONSTANT, "2", 7, 95},
      {TokenType::SEMICOLON, ";", 7, 96},
      {TokenType::IDENTIFIER, "e", 7, 98},
      {TokenType::ASSIGNMENT, "=", 7, 100},
      {TokenType::IDENTIFIER, "e", 7, 102},
      {TokenType::PERCENT_SIGN, "%", 7, 104},
      {TokenType::INT_CONSTANT, "5", 7, 106},
      {TokenType::SEMICOLON, ";", 7, 107},
      {TokenType::IDENTIFIER, "e", 7, 109},
      {TokenType::COMPOUND_REMAINDER, "%=", 7, 111},
      {TokenType::INT_CONSTANT, "2", 7, 114},
      {TokenType::SEMICOLON, ";", 7, 115},
      // Line 8
      {TokenType::IF, "if", 8, 3},
      {TokenType::OPEN_PARENTHESES, "(", 8, 6},
      {TokenType::OPEN_PARENTHESES, "(", 8, 8},
      {TokenType::IDENTIFIER, "a", 8, 9},
      {TokenType::EQUAL, "==", 8, 11},
      {TokenType::IDENTIFIER, "b", 8, 14},
      {TokenType::CLOSE_PARENTHESES, ")", 8, 15},
      {TokenType::LOR, "||", 8, 17},
      {TokenType::OPEN_PARENTHESES, "(", 8, 20},
      {TokenType::IDENTIFIER, "c", 8, 21},
      {TokenType::NOTEQUAL, "!=", 8, 23},
      {TokenType::IDENTIFIER, "d", 8, 26},
      {TokenType::CLOSE_PARENTHESES, ")", 8, 27},
      {TokenType::LAND, "&&", 8, 29},
      {TokenType::OPEN_PARENTHESES, "(", 8, 32},
      {TokenType::IDENTIFIER, "e", 8, 33},
      {TokenType::LESSTHAN, "<", 8, 35},
      {TokenType::IDENTIFIER, "f", 8, 37},
      {TokenType::CLOSE_PARENTHESES, ")", 8, 38},
      {TokenType::LOR, "||", 8, 40},
      {TokenType::OPEN_PARENTHESES, "(", 8, 43},
      {TokenType::IDENTIFIER, "g", 8, 44},
      {TokenType::GREATERTHAN, ">", 8, 46},
      {TokenType::IDENTIFIER, "a", 8, 48},
      {TokenType::CLOSE_PARENTHESES, ")", 8, 49},
      {TokenType::CLOSE_PARENTHESES, ")", 8, 51},
      {TokenType::OPEN_BRACE, "{", 8, 53},
      // Line 9
      {TokenType::IDENTIFIER, "a", 9, 5},
      {TokenType::ASSIGNMENT, "=", 9, 7},
      {TokenType::OPEN_PARENTHESES, "(", 9, 9},
      {TokenType::IDENTIFIER, "a", 9, 10},
      {TokenType::LEFT_SHIFT, "<<", 9, 12},
      {TokenType::INT_CONSTANT, "1", 9, 15},
      {TokenType::CLOSE_PARENTHESES, ")", 9, 16},
      {TokenType::SEMICOLON, ";", 9, 17},
      {TokenType::IDENTIFIER, "b", 9, 19},
      {TokenType::ASSIGNMENT, "=", 9, 21},
      {TokenType::OPEN_PARENTHESES, "(", 9, 23},
      {TokenType::IDENTIFIER, "b", 9, 24},
      {TokenType::RIGHT_SHIFT, ">>", 9, 26},
      {TokenType::INT_CONSTANT, "2", 9, 29},
      {TokenType::CLOSE_PARENTHESES, ")", 9, 30},
      {TokenType::SEMICOLON, ";", 9, 31},
      {TokenType::IDENTIFIER, "a", 9, 33},
      {TokenType::COMPOUND_LEFTSHIFT, "<<=", 9, 35},
      {TokenType::INT_CONSTANT, "1", 9, 39},
      {TokenType::SEMICOLON, ";", 9, 40},
      {TokenType::IDENTIFIER, "b", 9, 42},
      {TokenType::COMPOUND_RIGHTSHIFT, ">>=", 9, 44},
      {TokenType::INT_CONSTANT, "2", 9, 48},
      {TokenType::SEMICOLON, ";", 9, 49},
      // Line 10
      {TokenType::IDENTIFIER, "a", 10, 5},
      {TokenType::ASSIGNMENT, "=", 10, 7},
      {TokenType::OPEN_PARENTHESES, "(", 10, 9},
      {TokenType::IDENTIFIER, "a", 10, 10},
      {TokenType::AAND, "&", 10, 12},
      {TokenType::IDENTIFIER, "b", 10, 14},
      {TokenType::CLOSE_PARENTHESES, ")", 10, 15},
      {TokenType::AOR, "|", 10, 17},
      {TokenType::OPEN_PARENTHESES, "(", 10, 19},
      {TokenType::IDENTIFIER, "c", 10, 20},
      {TokenType::XOR, "^", 10, 22},
      {TokenType::IDENTIFIER, "d", 10, 24},
      {TokenType::CLOSE_PARENTHESES, ")", 10, 25},
      {TokenType::SEMICOLON, ";", 10, 26},
      {TokenType::IDENTIFIER, "a", 10, 28},
      {TokenType::COMPOUND_AND, "&=", 10, 30},
      {TokenType::IDENTIFIER, "b", 10, 33},
      {TokenType::SEMICOLON, ";", 10, 34},
      {TokenType::IDENTIFIER, "b", 10, 36},
      {TokenType::COMPOUND_OR, "|=", 10, 38},
      {TokenType::IDENTIFIER, "c", 10, 41},
      {TokenType::SEMICOLON, ";", 10, 42},
      {TokenType::IDENTIFIER, "c", 10, 44},
      {TokenType::COMPOUND_XOR, "^=", 10, 46},
      {TokenType::IDENTIFIER, "d", 10, 49},
      {TokenType::SEMICOLON, ";", 10, 50},
      // Line 11
      {TokenType::IF, "if", 11, 5},
      {TokenType::OPEN_PARENTHESES, "(", 11, 8},
      {TokenType::NOT, "!", 11, 9},
      {TokenType::NOT, "!", 11, 10},
      {TokenType::IDENTIFIER, "a", 11, 11},
      {TokenType::LAND, "&&", 11, 13},
      {TokenType::TILDE, "~", 11, 16},
      {TokenType::IDENTIFIER, "b", 11, 17},
      {TokenType::CLOSE_PARENTHESES, ")", 11, 18},
      {TokenType::IDENTIFIER, "a", 11, 20},
      {TokenType::ASSIGNMENT, "=", 11, 22},
      {TokenType::IDENTIFIER, "a", 11, 24},
      {TokenType::QUESTION_MARK, "?", 11, 26},
      {TokenType::IDENTIFIER, "b", 11, 28},
      {TokenType::COLON, ":", 11, 30},
      {TokenType::IDENTIFIER, "c", 11, 32},
      {TokenType::SEMICOLON, ";", 11, 33},
      // Line 12
      {TokenType::CLOSE_BRACE, "}", 12, 3},
      // Line 13
      {TokenType::PRINTF, "printf", 13, 3},
      {TokenType::OPEN_PARENTHESES, "(", 13, 9},
      {TokenType::STRING, "x=\%f y=\%f\n", 13, 10},
      {TokenType::COMMA, ",", 13, 23},
      {TokenType::IDENTIFIER, "x", 13, 25},
      {TokenType::COMMA, ",", 13, 26},
      {TokenType::IDENTIFIER, "y", 13, 28},
      {TokenType::CLOSE_PARENTHESES, ")", 13, 29},
      {TokenType::SEMICOLON, ";", 13, 30},
      // Line 14
      {TokenType::IDENTIFIER, "Node", 14, 3},
      {TokenType::IDENTIFIER, "n", 14, 8},
      {TokenType::SEMICOLON, ";", 14, 9},
      {TokenType::IDENTIFIER, "n", 14, 11},
      {TokenType::DOT, ".", 14, 12},
      {TokenType::IDENTIFIER, "v", 14, 13},
      {TokenType::ASSIGNMENT, "=", 14, 15},
      {TokenType::INT_CONSTANT, "10", 14, 17},
      {TokenType::SEMICOLON, ";", 14, 19},
      {TokenType::IDENTIFIER, "n", 14, 21},
      {TokenType::DOT, ".", 14, 22},
      {TokenType::IDENTIFIER, "next", 14, 23},
      {TokenType::ASSIGNMENT, "=", 14, 28},
      {TokenType::AAND, "&", 14, 30},
      {TokenType::IDENTIFIER, "n", 14, 31},
      {TokenType::SEMICOLON, ";", 14, 32},
      // Line 15
      {TokenType::IDENTIFIER, "Node", 15, 3},
      {TokenType::ASTERISK, "*", 15, 7},
      {TokenType::IDENTIFIER, "p", 15, 9},
      {TokenType::ASSIGNMENT, "=", 15, 11},
      {TokenType::AAND, "&", 15, 13},
      {TokenType::IDENTIFIER, "n", 15, 14},
      {TokenType::SEMICOLON, ";", 15, 15},
      {TokenType::IF, "if", 15, 17},
      {TokenType::OPEN_PARENTHESES, "(", 15, 20},
      {TokenType::IDENTIFIER, "p", 15, 21},
      {TokenType::ARROW_OPERATOR, "->", 15, 22},
      {TokenType::IDENTIFIER, "next", 15, 24},
      {TokenType::ARROW_OPERATOR, "->", 15, 28},
      {TokenType::IDENTIFIER, "next", 15, 30},
      {TokenType::EQUAL, "==", 15, 35},
      {TokenType::AAND, "&", 15, 38},
      {TokenType::IDENTIFIER, "n", 15, 39},
      {TokenType::CLOSE_PARENTHESES, ")", 15, 40},
      {TokenType::OPEN_BRACE, "{", 15, 42},
      {TokenType::ELLIPSIS, "...", 15, 44},
      {TokenType::CLOSE_BRACE, "}", 15, 48},
      // Line 16
      {TokenType::IDENTIFIER, "w", 16, 3},
      {TokenType::ASSIGNMENT, "=", 16, 5},
      {TokenType::FLOAT_CONSTANT, ".123", 16, 7},
      {TokenType::SEMICOLON, ";", 16, 11},
      {TokenType::IDENTIFIER, "z", 16, 13},
      {TokenType::ASSIGNMENT, "=", 16, 15},
      {TokenType::FLOAT_CONSTANT, "7.", 16, 17},
      {TokenType::SEMICOLON, ";", 16, 19},
      {TokenType::IDENTIFIER, "x", 16, 21},
      {TokenType::ASSIGNMENT, "=", 16, 23},
      {TokenType::FLOAT_CONSTANT, ".9", 16, 25},
      {TokenType::SEMICOLON, ";", 16, 27},
      // Line 17
      {TokenType::IDENTIFIER, "a", 17, 3},
      {TokenType::ASSIGNMENT, "=", 17, 5},
      {TokenType::IDENTIFIER, "b", 17, 7},
      {TokenType::FORWARD_SLASH, "/", 17, 9},
      {TokenType::IDENTIFIER, "c", 17, 11},
      {TokenType::SEMICOLON, ";", 17, 12},
      // Line 18
      {TokenType::RETURN, "return", 18, 3},
      {TokenType::INT_CONSTANT, "0", 18, 10},
      {TokenType::SEMICOLON, ";", 18, 11},
      // Line 19
      {TokenType::CLOSE_BRACE, "}", 19, 1}};

  // Verify token count matches exactly
  ASSERT_EQ(tokens.size(), expectedTokens.size())
      << "Token count mismatch: expected " << expectedTokens.size() << ", got "
      << tokens.size();

  // Verify each token matches exactly (type, lexeme, line, column)
  for (size_t i = 0; i < expectedTokens.size(); ++i) {
    const auto &[expectedType, expectedLexeme, expectedLine, expectedCol] =
        expectedTokens[i];
    const auto &actualToken = tokens[i];

    EXPECT_EQ(actualToken.GetType(), expectedType)
        << "Token type mismatch at position " << i << ": expected "
        << TokenTypeToString(expectedType) << ", got "
        << TokenTypeToString(actualToken.GetType()) << " (lexeme: '"
        << actualToken.GetLexeme() << "')";

    EXPECT_EQ(actualToken.GetLexeme(), expectedLexeme)
        << "Token lexeme mismatch at position " << i << ": expected '"
        << expectedLexeme << "', got '" << actualToken.GetLexeme() << "'";

    EXPECT_EQ(actualToken.GetLineNumber(), expectedLine)
        << "Token line mismatch at position " << i << ": expected line "
        << expectedLine << ", got line " << actualToken.GetLineNumber()
        << " (token: '" << actualToken.GetLexeme() << "')";

    EXPECT_EQ(actualToken.GetColumnNumber(), expectedCol)
        << "Token column mismatch at position " << i << ": expected column "
        << expectedCol << ", got column " << actualToken.GetColumnNumber()
        << " (token: '" << actualToken.GetLexeme() << "')";
  }
}

// Test 9: Additional test case
TEST_F(LexerTest, Test9_AdditionalCase) {
  std::string content = readTestFile("test9.vlx");
  auto tokens = runLexer(content);

  // Verify lexer succeeded
  EXPECT_TRUE(runLexerWithSuccess(content));

  EXPECT_GT(tokens.size(), 0) << "Expected non-empty token list";
}

// Test 10: Another test case
TEST_F(LexerTest, Test10_AnotherCase) {
  std::string content = readTestFile("test10.vlx");
  auto tokens = runLexer(content);

  // Verify lexer succeeded
  EXPECT_TRUE(runLexerWithSuccess(content));

  EXPECT_GT(tokens.size(), 0) << "Expected non-empty token list";
}

// Test 11: Final test case
TEST_F(LexerTest, Test11_FinalCase) {
  std::string content = readTestFile("test11.vlx");
  auto tokens = runLexer(content);

  // Verify lexer succeeded
  EXPECT_TRUE(runLexerWithSuccess(content));

  EXPECT_GT(tokens.size(), 0) << "Expected non-empty token list";
}

// Test for error handling
TEST_F(LexerTest, ErrorHandling_InvalidTokens) {
  std::string invalidContent = "@ # $ invalid tokens here";

  // This should fail lexical analysis
  EXPECT_FALSE(runLexerWithSuccess(invalidContent));
}

TEST_F(LexerTest, EmptyInput) {
  std::string emptyContent = "";
  auto tokens = runLexer(emptyContent);

  EXPECT_TRUE(runLexerWithSuccess(emptyContent));
  EXPECT_EQ(tokens.size(), 0) << "Expected empty token list for empty input";
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
