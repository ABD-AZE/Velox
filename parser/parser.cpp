#include "parser.hpp"

Token Parser::consume()
{
  if (currentIndex < tokenSize)
  {
    currentToken = tokens[currentIndex--];
    tokens.pop_back();
    return currentToken;
  }
  return Token(); // return a default token if out of bounds
}

Token Parser::peek()
{
  if (currentIndex < tokenSize)
  {
    currentToken = tokens[currentIndex];
  }
  return currentToken;
}

bool Parser::expect(TokenType actual, TokenType expected)
{
  if (expected != actual)
  {
    success = false;
    errors.emplace_back(currentToken.GetLineNumber(),
                        currentToken.GetColumnNumber(), actual,
                        "expected " + TokenTypeToString(expected));
    return false;
  }
  return true;
}

void Parser::printErrors() const
{
  for (const auto &error : errors)
  {
    std::cerr << "Error at line " << error.lineNumber << ", column "
              << error.columnNumber << error.expected << ", but got "
              << TokenTypeToString(error.actualToken) << std::endl;
  }
}

// for postfix expression parsing
std::map<TokenType, int> Precedence = {
    {TokenType::END_OF_FILE, 0},
    {TokenType::ASSIGNMENT, 2},          // right associative
    {TokenType::COMPOUND_SUM, 2},        // right associative
    {TokenType::COMPOUND_DIFFERENCE, 2}, // right associative
    {TokenType::COMPOUND_PRODUCT, 2},    // right associative
    {TokenType::COMPOUND_DIVISION, 2},   // right associative
    {TokenType::COMPOUND_REMAINDER, 2},  // right associative
    {TokenType::COMPOUND_LEFTSHIFT, 2},  // right associative
    {TokenType::COMPOUND_RIGHTSHIFT, 2}, // right associative
    {TokenType::COMPOUND_AND, 2},        // right associative
    {TokenType::COMPOUND_XOR, 2},        // right associative
    {TokenType::COMPOUND_OR, 2},         // right associative
    {TokenType::QUESTION_MARK, 3},       // right associative
    {TokenType::COLON, 3},               // right associative
    {TokenType::LOR, 5},
    {TokenType::LAND, 10},
    {TokenType::AOR, 15},
    {TokenType::XOR, 20},
    {TokenType::AAND, 25},
    {TokenType::EQUAL, 30},
    {TokenType::NOTEQUAL, 30},
    {TokenType::GREATERTHAN, 35},
    {TokenType::LESSTHAN, 35},
    {TokenType::GREATERTHANEQUAL, 35},
    {TokenType::LESSTHANEQUAL, 35},
    {TokenType::LEFT_SHIFT, 40},
    {TokenType::RIGHT_SHIFT, 40},
    {TokenType::PLUS, 45},
    {TokenType::HYPHEN, 45},
    {TokenType::ASTERISK, 50},
    {TokenType::FORWARD_SLASH, 50},
    {TokenType::PERCENT_SIGN, 50},
    {TokenType::INCREMENT_OPERATOR, 60},
    {TokenType::DECREMENT_OPERATOR, 60},
};

Parser::Parser(const std::vector<Token> &tokens)
    : tokens(tokens), tokenSize(tokens.size() + 1)
{
  this->tokens.push_back(Token());
  this->tokens.back().SetType(TokenType::END_OF_FILE);
  std::reverse(this->tokens.begin(), this->tokens.end());
  currentIndex = this->tokens.size() - 1;
}

const ASTNodePtr &Parser::parseProgram()
{
  ProgramNodePtr programNode =
      std::make_unique<ProgramNode>(parseFunctionDefinition());
  ast = std::move(programNode);
  if (tokens.size() > 1)
  {
    success = false;
    errors.emplace_back(currentToken.GetLineNumber(),
                        currentToken.GetColumnNumber(), consume().GetType(),
                        " end of file expected");
  }
  return ast;
}

ASTNodePtr Parser::parseFunctionDefinition()
{
  FunctionDefinitionNodePtr functionDefNode =
      std::make_unique<FunctionDefinitionNode>();
  expect(consume().GetType(), TokenType::INT);
  expect(consume().GetType(), TokenType::IDENTIFIER);
  functionDefNode->name = currentToken.GetLexeme();
  expect(consume().GetType(), TokenType::OPEN_PARENTHESES);
  expect(consume().GetType(), TokenType::VOID);
  expect(consume().GetType(), TokenType::CLOSE_PARENTHESES);
  functionDefNode->body = parseBlock();
  return functionDefNode;
}

ASTNodePtr Parser::parseStatement()
{
  ASTNodePtr statementNode;
  switch (peek().GetType())
  {
  case TokenType::SEMICOLON:
    consume();
    statementNode = std::make_unique<NullStatement>();
    break;
  case TokenType::RETURN:
    expect(consume().GetType(), TokenType::RETURN);
    statementNode = std::make_unique<ReturnStatement>(parseExpression(0));
    expect(consume().GetType(), TokenType::SEMICOLON);
    break;
  case TokenType::IF:
  {
    expect(consume().GetType(), TokenType::IF);
    expect(consume().GetType(), TokenType::OPEN_PARENTHESES);
    ASTNodePtr condition = parseExpression(0);
    expect(consume().GetType(), TokenType::CLOSE_PARENTHESES);
    ASTNodePtr thenBranch = parseStatement();
    std::optional<std::unique_ptr<ASTNode>> elseBranch = std::nullopt;
    if (peek().GetType() == TokenType::ELSE)
    {
      expect(consume().GetType(), TokenType::ELSE);
      elseBranch = parseStatement();
    }
    statementNode = std::make_unique<IfStatement>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
    break;
  }
  case TokenType::GOTO:
  {
    expect(consume().GetType(), TokenType::GOTO);
    expect(consume().GetType(), TokenType::IDENTIFIER);
    std::string label = currentToken.GetLexeme();
    statementNode = std::make_unique<GotoStatement>(label);
    expect(consume().GetType(), TokenType::SEMICOLON);
    break;
  }
  case TokenType::IDENTIFIER:
  {
    // Could be a label or an expression statement
    if (currentIndex - 1 >= 0)
    {
      Token nextToken = tokens[currentIndex - 1];
      if (nextToken.GetType() == TokenType::COLON)
      {
        // It's a label
        expect(consume().GetType(), TokenType::IDENTIFIER);
        std::string label = currentToken.GetLexeme(); // store label name
        expect(consume().GetType(), TokenType::COLON);
        statementNode = std::make_unique<LabelStatement>(label);
      }
      else
      {
        // It's an expression statement
        statementNode = std::make_unique<ExpressionStatement>(parseExpression(0));
        expect(consume().GetType(), TokenType::SEMICOLON);
      }
    }
    else
    {
      // It's an expression statement
      statementNode = std::make_unique<ExpressionStatement>(parseExpression(0));
      expect(consume().GetType(), TokenType::SEMICOLON);
    }
    break;
  }
  case TokenType::OPEN_BRACE:
  {
    statementNode = std::make_unique<CompoundStatement>(parseBlock());
    break;
  }
  // Default case: treat as an expression statement
  // expression
  default:
  {
    statementNode = std::make_unique<ExpressionStatement>(parseExpression(0));
    expect(consume().GetType(), TokenType::SEMICOLON);
    break;
  }
  }
  return statementNode;
}

ASTNodePtr Parser::parseFactor()
{
  ASTNodePtr factorNode;
  switch (peek().GetType())
  {
  case TokenType::INT_CONSTANT:
    consume();
    try
    {
      std::string lexeme = currentToken.GetLexeme();
      int value = 0;
      if (!lexeme.empty())
      {
        value = std::stoi(lexeme);
      }
      factorNode = std::make_unique<ConstantExpression>(value);
    }
    catch (const std::exception &e)
    {
      factorNode = std::make_unique<ConstantExpression>(0);
      success = 0;
      errors.push_back(ParserErrorInfo(
          currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
          currentToken.GetType(), "integer constant"));
    }
    break;

  case TokenType::HYPHEN:
  case TokenType::TILDE:
  case TokenType::NOT:
    consume();
    {
      TokenType op = currentToken.GetType();
      ASTNodePtr operand = parseFactor();
      factorNode = std::make_unique<UnaryExpression>(op, std::move(operand));
    }
    break;
  case TokenType::OPEN_PARENTHESES:
    consume();
    factorNode = parseExpression(0);
    expect(consume().GetType(), TokenType::CLOSE_PARENTHESES);
    break;
  case TokenType::IDENTIFIER:
    consume();
    factorNode = std::make_unique<VariableExpression>(currentToken.GetLexeme());
    break;
  case TokenType::INCREMENT_OPERATOR:
  case TokenType::DECREMENT_OPERATOR:
    consume();
    {
      TokenType op = currentToken.GetType();
      ASTNodePtr operand = parseFactor();
      factorNode = std::make_unique<UnaryExpression>(op, std::move(operand));
    }
    break;
  default:
    success = 0;
    while (currentToken.GetType() != TokenType::SEMICOLON &&
           currentToken.GetType() != TokenType::END_OF_FILE &&
           currentToken.GetType() != TokenType::CLOSE_PARENTHESES)
    {
      consume();
    }
    errors.push_back(ParserErrorInfo(currentToken.GetLineNumber(),
                                     currentToken.GetColumnNumber(),
                                     currentToken.GetType(), "factor"));
    // Create a dummy constant expression for error recovery
    factorNode = std::make_unique<ConstantExpression>(0);
    break;
  }
  return factorNode;
}

ASTNodePtr Parser::parseExpression(int minPrecedence)
{
  ASTNodePtr left = parseFactor();
  // Handle postfix operators
  while (peek().GetType() == TokenType::INCREMENT_OPERATOR ||
         peek().GetType() == TokenType::DECREMENT_OPERATOR)
  {
    TokenType op = consume().GetType();
    left = std::make_unique<PostfixExpression>(std::move(left), op);
  }
  while ((peek().GetType() == TokenType::ASSIGNMENT ||
          peek().GetType() == TokenType::PLUS ||
          peek().GetType() == TokenType::HYPHEN ||
          peek().GetType() == TokenType::ASTERISK ||
          peek().GetType() == TokenType::FORWARD_SLASH ||
          peek().GetType() == TokenType::PERCENT_SIGN ||
          peek().GetType() == TokenType::LEFT_SHIFT ||
          peek().GetType() == TokenType::RIGHT_SHIFT ||
          peek().GetType() == TokenType::XOR ||
          peek().GetType() == TokenType::AAND ||
          peek().GetType() == TokenType::AOR ||
          peek().GetType() == TokenType::LOR ||
          peek().GetType() == TokenType::LAND ||
          peek().GetType() == TokenType::GREATERTHAN ||
          peek().GetType() == TokenType::LESSTHAN ||
          peek().GetType() == TokenType::GREATERTHANEQUAL ||
          peek().GetType() == TokenType::LESSTHANEQUAL ||
          peek().GetType() == TokenType::EQUAL ||
          peek().GetType() == TokenType::NOTEQUAL ||
          peek().GetType() == TokenType::COMPOUND_SUM ||
          peek().GetType() == TokenType::COMPOUND_DIFFERENCE ||
          peek().GetType() == TokenType::COMPOUND_PRODUCT ||
          peek().GetType() == TokenType::COMPOUND_DIVISION ||
          peek().GetType() == TokenType::COMPOUND_REMAINDER ||
          peek().GetType() == TokenType::COMPOUND_LEFTSHIFT ||
          peek().GetType() == TokenType::COMPOUND_RIGHTSHIFT ||
          peek().GetType() == TokenType::COMPOUND_AND ||
          peek().GetType() == TokenType::COMPOUND_XOR ||
          peek().GetType() == TokenType::COMPOUND_OR ||
          peek().GetType() == TokenType::QUESTION_MARK) &&
         Precedence[peek().GetType()] > minPrecedence)
  {
    TokenType op = consume().GetType();
    ASTNodePtr right;
    if (currentToken.GetType() == TokenType::QUESTION_MARK)
    {
      // Ternary conditional operator
      ASTNodePtr trueExpr = parseExpression(0);
      expect(consume().GetType(), TokenType::COLON);
      ASTNodePtr falseExpr = parseExpression(Precedence[op] - 1);
      left = std::make_unique<ConditionalExpression>(std::move(left), std::move(trueExpr), std::move(falseExpr));
      continue;
    }
    if (currentToken.GetType() == TokenType::ASSIGNMENT ||
        currentToken.GetType() == TokenType::COMPOUND_SUM ||
        currentToken.GetType() == TokenType::COMPOUND_DIFFERENCE ||
        currentToken.GetType() == TokenType::COMPOUND_PRODUCT ||
        currentToken.GetType() == TokenType::COMPOUND_DIVISION ||
        currentToken.GetType() == TokenType::COMPOUND_REMAINDER ||
        currentToken.GetType() == TokenType::COMPOUND_LEFTSHIFT ||
        currentToken.GetType() == TokenType::COMPOUND_RIGHTSHIFT ||
        currentToken.GetType() == TokenType::COMPOUND_AND ||
        currentToken.GetType() == TokenType::COMPOUND_XOR ||
        currentToken.GetType() == TokenType::COMPOUND_OR)
    {
      right = parseExpression(Precedence[op] - 1);
      // Create assignment expression
      left = std::make_unique<AssignmentExpression>(std::move(left),
                                                    std::move(right), op);
    }
    else
    {
      right = parseExpression(Precedence[op]);
      // Create binary expression
      left = std::make_unique<BinaryExpression>(op, std::move(left),
                                                std::move(right));
    }
  }
  return left;
}

ASTNodePtr Parser::parseDeclaration()
{
  expect(consume().GetType(), TokenType::INT);
  expect(consume().GetType(), TokenType::IDENTIFIER);
  std::string varName = currentToken.GetLexeme();
  ASTNodePtr init = nullptr;
  if (peek().GetType() == TokenType::ASSIGNMENT)
  {
    consume(); // consume '='
    init = parseExpression(0);
  }
  expect(consume().GetType(), TokenType::SEMICOLON);

  if (init)
  {
    return std::make_unique<DeclarationNode>(varName, std::move(init));
  }
  else
  {
    return std::make_unique<DeclarationNode>(varName);
  }
}

ASTNodePtr Parser::parseBlockItem()
{
  ASTNodePtr blockItemNode;
  if (peek().GetType() == TokenType::INT)
  {
    // Parse declaration
    blockItemNode = std::make_unique<BlockItemNode>(parseDeclaration());
  }
  else
  {
    // Parse statement
    blockItemNode = std::make_unique<BlockItemNode>(parseStatement());
  }
  return blockItemNode;
}

ASTNodePtr Parser::parseBlock()
{
  ASTNodePtr blockNode;
  expect(consume().GetType(), TokenType::OPEN_BRACE);
  std::vector<ASTNodePtr> items;
  while (peek().GetType() != TokenType::CLOSE_BRACE &&
         peek().GetType() != TokenType::END_OF_FILE)
  {
    items.push_back(parseBlockItem());
  }
  expect(consume().GetType(), TokenType::CLOSE_BRACE);
  blockNode = std::make_unique<BlockNode>(std::move(items));
  return blockNode;
}