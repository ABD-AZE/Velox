#include "parser.hpp"
#include <format>

Token Parser::consume() {
  if (currentIndex < tokenSize) {
    currentToken = tokens[currentIndex--];
    tokens.pop_back();
    return currentToken;
  }
  return Token(); // return a default token if out of bounds
}

Token Parser::get() {
  if (currentIndex < tokenSize) {
    currentToken = tokens[currentIndex--];
    return currentToken;
  }
  return Token(); // return a default token if out of bounds
}

Token Parser::peek() {
  if (currentIndex < tokenSize) {
    currentToken = tokens[currentIndex];
  }
  return currentToken;
}

bool Parser::expect(TokenType actual, TokenType expected) {
  if (expected != actual) {
    success = false;
    errors.emplace_back(currentToken.GetLineNumber(),
                        currentToken.GetColumnNumber(), actual,
                        "expected " + TokenTypeToString(expected));
    return false;
  }
  return true;
}

void Parser::printErrors() const {
<<<<<<< Updated upstream
  using namespace termcolor;

  if (errors.empty()) {
    std::cout << green << "[ok] No syntax errors.\n" << reset;
    return;
  }

  std::cout << bold << red << "[errors] Syntax errors found: " << errors.size()
            << "\n"
            << reset;

  for (const auto &error : errors) {
    if (error.actualToken == TokenType::WS) {
      std::cout << red << "  " << error.lineNumber << ":" << error.columnNumber
                << ": expected " << error.expected << ", found whitespace\n"
                << reset;
=======
  for (const auto &error : errors) {
    if (error.actualToken == TokenType::WS) {
      std::cerr << "Error at line " << error.lineNumber << ", column "
                << error.columnNumber << " " << error.expected << std::endl;
>>>>>>> Stashed changes
      continue;
    }

    std::cout << red << "  " << error.lineNumber << ":" << error.columnNumber
              << ": expected " << error.expected << ", but got "
              << TokenTypeToString(error.actualToken) << "\n"
              << reset;
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
    : tokens(tokens), tokenSize(tokens.size() + 1) {
  this->tokens.push_back(Token());
  this->tokens.back().SetType(TokenType::END_OF_FILE);
  std::reverse(this->tokens.begin(), this->tokens.end());
  currentIndex = this->tokens.size() - 1;
}

const ASTNodePtr &Parser::parseProgram() {
  std::vector<ASTNodePtr>
      declarations; // contains both declarations and definitions
  while (peek().GetType() != TokenType::END_OF_FILE) {
    declarations.push_back(parseDeclaration());
  }
  ProgramNodePtr programNode =
      std::make_unique<ProgramNode>(std::move(declarations));
  ast = std::move(programNode);
  if (tokens.size() > 1) {
    success = false;
    errors.emplace_back(currentToken.GetLineNumber(),
                        currentToken.GetColumnNumber(), consume().GetType(),
                        " end of file expected");
  }
  return ast;
}

// parses both function declarations and definitions
ASTNodePtr Parser::parseFunctionDeclaration() {
  std::vector<TokenType> specifier_list;
  if (!isSpecifier(peek().GetType())) {
    success = 0;
    errors.push_back(ParserErrorInfo(
        currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
        peek().GetType(), "expected variable declaration"));
    while (peek().GetType() != TokenType::IDENTIFIER &&
           peek().GetType() != TokenType::SEMICOLON &&
           peek().GetType() != TokenType::END_OF_FILE) {
      consume();
    }
  }
  while (isSpecifier(peek().GetType())) {
    consume();
    specifier_list.push_back(currentToken.GetType());
  }
  auto [type_list, storage_class] = parseSpecifierList(specifier_list);
  auto declarator = parseDeclarator();
  auto [name, type, param_names] = processDeclarator(
      std::move(declarator), parseTypeSpecifierList(type_list));
  if (type.kind != TypeKind::FUNC) {
    success = 0;
    errors.push_back(ParserErrorInfo(
        currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
        peek().GetType(), "expected function declarator"));
    while (peek().GetType() != TokenType::SEMICOLON &&
           peek().GetType() != TokenType::END_OF_FILE) {
      consume();
    }
  }
  FunDeclNodePtr functionDeclNode = std::make_unique<FunDeclNode>();
  functionDeclNode->type = std::move(type);
  functionDeclNode->storage_class = storage_class;
  functionDeclNode->name = name;
  functionDeclNode->param_names = param_names;
  if (peek().GetType() == TokenType::SEMICOLON) {
    consume();               // consume ';'
    return functionDeclNode; // function declaration without body
  }
  // function definition with body
  functionDeclNode->body = parseBlock();
  return functionDeclNode;
}

ASTNodePtr Parser::parseVariableDeclaration() {
  DeclarationNodePtr declarationNode = std::make_unique<DeclarationNode>();

  std::vector<TokenType> specifier_list;
  if (!isSpecifier(peek().GetType())) {
    success = 0;
    errors.push_back(ParserErrorInfo(
        currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
        peek().GetType(), "expected variable declaration"));
    while (peek().GetType() != TokenType::IDENTIFIER &&
           peek().GetType() != TokenType::SEMICOLON &&
           peek().GetType() != TokenType::END_OF_FILE) {
      consume();
    }
  }
  while (isSpecifier(peek().GetType())) {
    consume();
    specifier_list.push_back(currentToken.GetType());
  }
  auto [type_list, storage_class] = parseSpecifierList(specifier_list);
  auto declarator = parseDeclarator();
  auto [name, type, param_names] = processDeclarator(
      std::move(declarator), parseTypeSpecifierList(type_list));
  if (type.kind == TypeKind::FUNC) {
    success = 0;
    errors.push_back(ParserErrorInfo(
        currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
        peek().GetType(), "function declaration not allowed here"));
    while (peek().GetType() != TokenType::SEMICOLON &&
           peek().GetType() != TokenType::END_OF_FILE) {
      consume();
    }
  }
  VarDeclNodePtr varDeclNode =
      std::make_unique<VarDeclNode>(name, std::move(type));
  varDeclNode->storage_class = storage_class;
  if (peek().GetType() == TokenType::ASSIGNMENT) {
    consume(); // consume '='
    varDeclNode->init = parseExpression(0);
  }
  expect(consume().GetType(), TokenType::SEMICOLON);
  return varDeclNode;
}

ASTNodePtr Parser::parseStatement() {
  ASTNodePtr statementNode;
  switch (peek().GetType()) {
  case TokenType::SEMICOLON:
    consume();
    statementNode = std::make_unique<NullStatement>();
    break;
  case TokenType::RETURN:
    expect(consume().GetType(), TokenType::RETURN);
    statementNode = std::make_unique<ReturnStatement>(parseExpression(0));
    expect(consume().GetType(), TokenType::SEMICOLON);
    break;
  case TokenType::IF: {
    expect(consume().GetType(), TokenType::IF);
    expect(consume().GetType(), TokenType::OPEN_PARENTHESES);
    ASTNodePtr condition = parseExpression(0);
    expect(consume().GetType(), TokenType::CLOSE_PARENTHESES);
    ASTNodePtr thenBranch = parseStatement();
    std::optional<std::unique_ptr<ASTNode>> elseBranch = std::nullopt;
    if (peek().GetType() == TokenType::ELSE) {
      expect(consume().GetType(), TokenType::ELSE);
      elseBranch = parseStatement();
    }
    statementNode = std::make_unique<IfStatement>(
        std::move(condition), std::move(thenBranch), std::move(elseBranch));
    break;
  }
  case TokenType::GOTO: {
    expect(consume().GetType(), TokenType::GOTO);
    expect(consume().GetType(), TokenType::IDENTIFIER);
    std::string label = currentToken.GetLexeme();
    statementNode = std::make_unique<GotoStatement>(label);
    expect(consume().GetType(), TokenType::SEMICOLON);
    break;
  }
  case TokenType::IDENTIFIER: {
    // Could be a label or an expression statement
    if (currentIndex - 1 >= 0) {
      Token nextToken = tokens[currentIndex - 1];
      if (nextToken.GetType() == TokenType::COLON) {
        // It's a label
        expect(consume().GetType(), TokenType::IDENTIFIER);
        std::string label = currentToken.GetLexeme(); // store label name
        expect(consume().GetType(), TokenType::COLON);
        statementNode = std::make_unique<LabelStatement>(label);
      } else {
        // It's an expression statement
        statementNode =
            std::make_unique<ExpressionStatement>(parseExpression(0));
        expect(consume().GetType(), TokenType::SEMICOLON);
      }
    } else {
      // It's an expression statement
      statementNode = std::make_unique<ExpressionStatement>(parseExpression(0));
      expect(consume().GetType(), TokenType::SEMICOLON);
    }
    break;
  }
  case TokenType::OPEN_BRACE: {
    statementNode = std::make_unique<CompoundStatement>(parseBlock());
    break;
  }
  case TokenType::BREAK: {
    expect(consume().GetType(), TokenType::BREAK);
    expect(consume().GetType(), TokenType::SEMICOLON);
    statementNode = std::make_unique<BreakNode>();
    break;
  }
  case TokenType::CONTINUE: {
    expect(consume().GetType(), TokenType::CONTINUE);
    expect(consume().GetType(), TokenType::SEMICOLON);
    statementNode = std::make_unique<ContinueNode>();
    break;
  }
  case TokenType::DO: {
    statementNode = parseDoWhile();
    break;
  }
  case TokenType::WHILE: {
    statementNode = parseWhile();
    break;
  }
  case TokenType::FOR: {
    statementNode = parseFor();
    break;
  }
  // Default case: treat as an expression statement
  // expression
  default: {
    statementNode = std::make_unique<ExpressionStatement>(parseExpression(0));
    expect(consume().GetType(), TokenType::SEMICOLON);
    break;
  }
  }
  return statementNode;
}

ASTNodePtr Parser::parseFactor() {
  ASTNodePtr factorNode;
  switch (peek().GetType()) {
  case TokenType::INT_CONSTANT:
    consume();
    try {
      std::string lexeme = currentToken.GetLexeme();
      int value = 0;
      if (!lexeme.empty()) {
        value = std::stoi(lexeme);
      }
      factorNode = std::make_unique<ConstantExpression>(value);
    } catch (const std::exception &e) {
      // If int parsing fails, try parsing as long
      try {
        std::string lexeme = currentToken.GetLexeme();
        long value = 0;
        if (!lexeme.empty()) {
          value = std::stol(lexeme);
        }
        factorNode = std::make_unique<ConstantExpression>(value);
      } catch (const std::exception &e2) {
        factorNode = std::make_unique<ConstantExpression>(0);
        success = 0;
        errors.push_back(ParserErrorInfo(
            currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
            currentToken.GetType(), "invalid integer constant"));
      }
    }
    break;

  case TokenType::LONG_CONSTANT:
    consume();
    try {
      std::string lexeme = currentToken.GetLexeme();
      long value = 0;
      if (!lexeme.empty()) {
        value = std::stol(lexeme);
      }
      factorNode = std::make_unique<ConstantExpression>(value);
    } catch (const std::exception &e) {
      factorNode = std::make_unique<ConstantExpression>(0L);
      success = 0;
      errors.push_back(ParserErrorInfo(
          currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
          currentToken.GetType(), "long integer constant"));
    }
    break;

  case TokenType::UINT_CONSTANT:
    consume();
    try {
      std::string lexeme = currentToken.GetLexeme();
      unsigned int value = 0;
      if (!lexeme.empty()) {
        value = (unsigned int)std::stoul(lexeme);
      }
      factorNode = std::make_unique<ConstantExpression>(value);
    } catch (const std::exception &e) {
      factorNode = std::make_unique<ConstantExpression>((unsigned int)0);
      success = 0;
      errors.push_back(ParserErrorInfo(
          currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
          currentToken.GetType(), "unsigned int constant"));
    }
    break;
  case TokenType::ULONG_CONSTANT:
    consume();
    try {
      std::string lexeme = currentToken.GetLexeme();
      unsigned long value = 0;
      if (!lexeme.empty()) {
        value = std::stoul(lexeme);
      }
      factorNode = std::make_unique<ConstantExpression>(value);
    } catch (const std::exception &e) {
      factorNode = std::make_unique<ConstantExpression>((unsigned int)0);
      success = 0;
      errors.push_back(ParserErrorInfo(
          currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
          currentToken.GetType(), "unsigned long integer constant"));
    }
    break;
  case TokenType::FLOAT_CONSTANT:
    consume();
    try {
      std::string lexeme = currentToken.GetLexeme();
      double value = 0;
      if (!lexeme.empty()) {
        value = std::stod(lexeme);
      }
      factorNode = std::make_unique<ConstantExpression>(value);
    } catch (const std::exception &e) {
      factorNode = std::make_unique<ConstantExpression>((unsigned int)0);
      success = 0;
      errors.push_back(ParserErrorInfo(
          currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
          currentToken.GetType(), "double constant"));
    }
    break;
  case TokenType::HYPHEN:
  case TokenType::TILDE:
  case TokenType::NOT:
  case TokenType::ASTERISK:
  case TokenType::AAND:
    consume();
    {
      TokenType op = currentToken.GetType();
      ASTNodePtr operand = parseFactor();
      factorNode = std::make_unique<UnaryExpression>(op, std::move(operand));
    }
    break;
  case TokenType::OPEN_PARENTHESES:
    // cast expression or parenthesized expression
    consume();
    if (isTypeSpecifier(peek().GetType())) {
      // cast expression
      std::vector<TokenType> type_list;
      while (isTypeSpecifier(peek().GetType()))
        type_list.push_back(consume().GetType());
      // abstract declarator
      Type baseType = parseTypeSpecifierList(type_list);
      auto abstractDeclarator = parseAbstractDeclarator();
      Type targetType = processAbstractDeclarator(std::move(abstractDeclarator),
                                                  std::move(baseType));
      expect(consume().GetType(), TokenType::CLOSE_PARENTHESES);
      ASTNodePtr expr = parseFactor();
      factorNode = std::make_unique<CastExpression>(std::move(targetType),
                                                    std::move(expr));
    } else {
      // parenthesized expression
      factorNode = parseExpression(0);
      expect(consume().GetType(), TokenType::CLOSE_PARENTHESES);
    }
    break;
  case TokenType::IDENTIFIER: {
    std::string identifier = consume().GetLexeme();
    if (peek().GetType() == TokenType::OPEN_PARENTHESES) {
      // function call
      consume(); // consume '('
      std::vector<ASTNodePtr> args;
      if (peek().GetType() != TokenType::CLOSE_PARENTHESES) {
        while (true) {
          ASTNodePtr arg = parseExpression(0);
          args.push_back(std::move(arg));
          if (peek().GetType() == TokenType::COMMA) {
            consume(); // consume ','
            continue;  // parse next argument
          } else {
            break; // end of argument list
          }
        }
      }
      expect(consume().GetType(), TokenType::CLOSE_PARENTHESES);
      factorNode =
          std::make_unique<FunctionCallNode>(identifier, std::move(args));
    } else {
      factorNode = std::make_unique<VariableExpression>(identifier);
    }
    break;
  }
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
           currentToken.GetType() != TokenType::CLOSE_PARENTHESES) {
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

ASTNodePtr Parser::parseExpression(int minPrecedence) {
  ASTNodePtr left = parseFactor();
  // Handle postfix operators
  while (peek().GetType() == TokenType::INCREMENT_OPERATOR ||
         peek().GetType() == TokenType::DECREMENT_OPERATOR) {
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
         Precedence[peek().GetType()] > minPrecedence) {
    TokenType op = consume().GetType();
    ASTNodePtr right;
    if (currentToken.GetType() == TokenType::QUESTION_MARK) {
      // Ternary conditional operator
      ASTNodePtr trueExpr = parseExpression(0);
      expect(consume().GetType(), TokenType::COLON);
      ASTNodePtr falseExpr = parseExpression(Precedence[op] - 1);
      left = std::make_unique<ConditionalExpression>(
          std::move(left), std::move(trueExpr), std::move(falseExpr));
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
        currentToken.GetType() == TokenType::COMPOUND_OR) {
      right = parseExpression(Precedence[op] - 1);
      // Create assignment expression
      left = std::make_unique<AssignmentExpression>(std::move(left),
                                                    std::move(right), op);
    } else {
      right = parseExpression(Precedence[op]);
      // Create binary expression
      left = std::make_unique<BinaryExpression>(op, std::move(left),
                                                std::move(right));
    }
  }
  return left;
}

// intermediate declaration node not required in ast
ASTNodePtr Parser::parseDeclaration() {
  DeclarationNodePtr declarationNode = std::make_unique<DeclarationNode>();

  std::vector<TokenType> specifier_list;
  if (!isSpecifier(peek().GetType())) {
    success = 0;
    errors.push_back(ParserErrorInfo(
        currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
        peek().GetType(), "expected variable declaration"));
    while (peek().GetType() != TokenType::IDENTIFIER &&
           peek().GetType() != TokenType::SEMICOLON &&
           peek().GetType() != TokenType::END_OF_FILE) {
      consume();
    }
  }
  while (isSpecifier(peek().GetType())) {
    consume();
    specifier_list.push_back(currentToken.GetType());
  }
  auto [type_list, storage_class] = parseSpecifierList(specifier_list);
  auto declarator = parseDeclarator();
  auto [name, type, param_names] = processDeclarator(
      std::move(declarator), parseTypeSpecifierList(type_list));
  if (type.kind == TypeKind::FUNC) {
    FunDeclNodePtr functionDeclNode = std::make_unique<FunDeclNode>();
    functionDeclNode->type = std::move(type);
    functionDeclNode->storage_class = storage_class;
    functionDeclNode->name = name;
    functionDeclNode->param_names = param_names;
    if (peek().GetType() == TokenType::SEMICOLON) {
      consume();               // consume ';'
      return functionDeclNode; // function declaration without body
    }
    // function definition with body
    functionDeclNode->body = parseBlock();
    return functionDeclNode;
  } else {
    VarDeclNodePtr varDeclNode = std::make_unique<VarDeclNode>();
    varDeclNode->name = name;
    varDeclNode->type = std::move(type);
    varDeclNode->storage_class = storage_class;
    if (peek().GetType() == TokenType::ASSIGNMENT) {
      consume(); // consume '='
      varDeclNode->init = parseExpression(0);
    }
    expect(consume().GetType(), TokenType::SEMICOLON);
    return varDeclNode;
  }
}

ASTNodePtr Parser::parseDeclarator() {
  DeclaratorNodePtr declaratorNode;
  switch (peek().GetType()) {
  case TokenType::IDENTIFIER:
    // simple declarator
    declaratorNode = std::make_unique<Ident>(consume().GetLexeme());
    if (peek().GetType() == TokenType::OPEN_PARENTHESES) {
      // function declarator
      auto params = parseParams();
      auto funDecl = std::make_unique<FunDeclarator>(std::move(params),
                                                     std::move(declaratorNode));
      declaratorNode = std::move(funDecl);
    }
    break;
  case TokenType::OPEN_PARENTHESES: {
    // simple declarator
    consume();
    auto declaratorNode = parseDeclarator();
    expect(consume().GetType(), TokenType::CLOSE_PARENTHESES);
    if (peek().GetType() == TokenType::OPEN_PARENTHESES) {
      // function declarator
      auto params = parseParams();
      auto funDecl = std::make_unique<FunDeclarator>(std::move(params),
                                                     std::move(declaratorNode));
      declaratorNode = std::move(funDecl);
    }
    return declaratorNode;
  }
  case TokenType::ASTERISK:
    // pointer declarator
    consume();
    declaratorNode = std::make_unique<PointerDeclarator>(parseDeclarator());
    break;
  }
  return declaratorNode;
}

ASTNodePtr Parser::parseAbstractDeclarator() {
  AbstractDeclaratorPtr declaratorNode;
  switch (peek().GetType()) {
  case TokenType::ASTERISK: {
    consume(); // consume '*'
    declaratorNode =
        std::make_unique<AbstractPointer>(parseAbstractDeclarator());
    return declaratorNode;
  }
  case TokenType::OPEN_PARENTHESES: {
    // direct abstract declarator
    consume(); // consume '('
    auto inner = parseAbstractDeclarator();
    expect(consume().GetType(), TokenType::CLOSE_PARENTHESES);
    return inner;
  }
  default:
    return std::make_unique<AbstractBase>();
  }
}

std::tuple<std::string, Type, std::vector<std::string>>
Parser::processDeclarator(ASTNodePtr declaratorNode, Type baseType) {
  std::string name;
  std::vector<std::string> param_names;
  Type type = std::move(baseType);
  if (auto ident = dynamic_cast<Ident *>(declaratorNode.get())) {
    name = ident->identifier;
    return std::make_tuple(name, type, param_names);
  } else if (auto pointerDecl =
                 dynamic_cast<PointerDeclarator *>(declaratorNode.get())) {
    type = Type(TypeKind::POINTER, PointerType(std::make_unique<Type>(type)));
    return processDeclarator(std::move(pointerDecl->declarator),
                             std::move(type));
  } else if (auto funDecl =
                 dynamic_cast<FunDeclarator *>(declaratorNode.get())) {
    std::vector<Type> paramtypes;
    for (auto &param : funDecl->params) {

      auto [param_name, param_type, _] =
          processDeclarator(std::move(param.declarator), std::move(param.type));
      if (param_type.kind == TypeKind::FUNC) {
        success = 0;
        errors.push_back(ParserErrorInfo(
            currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
            TokenType::WS, "function pointers in parameter list not allowed"));
      }
      param_names.push_back(param_name);
      paramtypes.push_back(param_type);
    }
    type = Type::Function(std::move(paramtypes), type);
    Ident *ident = dynamic_cast<Ident *>(funDecl->declarator.get());
    if (ident) {
      return std::make_tuple(ident->identifier, type, param_names);
    } else {
      success = 0;
      errors.push_back(ParserErrorInfo(
          currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
          TokenType::WS, "function declarator must have an identifier"));
      // error: function declarator must have an identifier
      return std::make_tuple("", type, param_names);
    }
  }
  success = 0;
  errors.push_back(ParserErrorInfo(currentToken.GetLineNumber(),
                                   currentToken.GetColumnNumber(),
                                   TokenType::WS, "unknown declarator type"));
  return std::make_tuple("", Type::Int(), param_names);
}

Type Parser::processAbstractDeclarator(ASTNodePtr abstractDeclaratorNode,
                                       Type baseType) {
  if (dynamic_cast<AbstractPointer *>(abstractDeclaratorNode.get())) {
    auto pointerDecl =
        dynamic_cast<AbstractPointer *>(abstractDeclaratorNode.get());
    Type type =
        Type(TypeKind::POINTER, PointerType(std::make_unique<Type>(baseType)));
    return processAbstractDeclarator(std::move(pointerDecl->base),
                                     std::move(type));
  } else if (dynamic_cast<AbstractBase *>(abstractDeclaratorNode.get())) {
    return baseType;
  }
  success = 0;
  errors.push_back(ParserErrorInfo(
      currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
      TokenType::WS, "unknown abstract declarator type"));
  return Type::Int();
}

std::vector<paraminfo> Parser::parseParams() {
  // {type-specifier}+ <declarator>
  expect(consume().GetType(), TokenType::OPEN_PARENTHESES);
  std::vector<paraminfo> params;
  if (peek().GetType() == TokenType::VOID) {
    consume(); // consume 'void'
    expect(consume().GetType(), TokenType::CLOSE_PARENTHESES);
    return params; // no parameters
  }
  while (peek().GetType() != TokenType::CLOSE_PARENTHESES &&
         peek().GetType() != TokenType::END_OF_FILE) {
    std::vector<TokenType> specifier_list;
    if (!isSpecifier(peek().GetType())) {
      success = 0;
      errors.push_back(ParserErrorInfo(
          currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
          peek().GetType(), "expected parameter declaration"));
      while (peek().GetType() != TokenType::IDENTIFIER &&
             peek().GetType() != TokenType::COMMA &&
             peek().GetType() != TokenType::CLOSE_PARENTHESES &&
             peek().GetType() != TokenType::END_OF_FILE) {
        consume();
      }
    }
    while (isSpecifier(peek().GetType())) {
      consume();
      specifier_list.push_back(currentToken.GetType());
    }
    auto [type_list, storage_class] = parseSpecifierList(specifier_list);
    Type param_type = parseTypeSpecifierList(type_list);
    if (storage_class.has_value()) {
      success = 0;
      errors.push_back(ParserErrorInfo(
          currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
          peek().GetType(),
          "storage class specifier not allowed in parameter declaration"));
    }
    auto declarator = parseDeclarator();
    paraminfo param(std::move(param_type), std::move(declarator));
    params.push_back(std::move(param));
    if (peek().GetType() == TokenType::COMMA) {
      consume(); // consume ','
      if (peek().GetType() == TokenType::CLOSE_PARENTHESES) {
        success = 0;
        errors.push_back(ParserErrorInfo(
            currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
            peek().GetType(), "trailing comma in parameter list"));
      }
      continue; // parse next parameter
    } else {
      expect(peek().GetType(), TokenType::CLOSE_PARENTHESES);
    }
  }
  expect(consume().GetType(), TokenType::CLOSE_PARENTHESES);
  return params;
}

ASTNodePtr Parser::parseBlockItem() {
  ASTNodePtr blockItemNode;
  if (isSpecifier(peek().GetType())) {
    // Parse declaration
    blockItemNode = std::make_unique<BlockItemNode>(parseDeclaration());
  } else {
    // Parse statement
    blockItemNode = std::make_unique<BlockItemNode>(parseStatement());
  }
  return blockItemNode;
}

ASTNodePtr Parser::parseBlock() {
  ASTNodePtr blockNode;
  expect(consume().GetType(), TokenType::OPEN_BRACE);
  std::vector<ASTNodePtr> items;
  while (peek().GetType() != TokenType::CLOSE_BRACE &&
         peek().GetType() != TokenType::END_OF_FILE) {
    items.push_back(parseBlockItem());
  }
  expect(consume().GetType(), TokenType::CLOSE_BRACE);
  blockNode = std::make_unique<BlockNode>(std::move(items));
  return blockNode;
}

ASTNodePtr Parser::parseDoWhile() {
  expect(consume().GetType(), TokenType::DO);
  ASTNodePtr body = parseStatement();
  expect(consume().GetType(), TokenType::WHILE);
  expect(consume().GetType(), TokenType::OPEN_PARENTHESES);
  ASTNodePtr condition = parseExpression(0);
  expect(consume().GetType(), TokenType::CLOSE_PARENTHESES);
  expect(consume().GetType(), TokenType::SEMICOLON);
  return std::make_unique<DoWhileNode>(std::move(condition), std::move(body));
}

ASTNodePtr Parser::parseWhile() {
  expect(consume().GetType(), TokenType::WHILE);
  expect(consume().GetType(), TokenType::OPEN_PARENTHESES);
  ASTNodePtr condition = parseExpression(0);
  expect(consume().GetType(), TokenType::CLOSE_PARENTHESES);
  ASTNodePtr body = parseStatement();
  return std::make_unique<WhileNode>(std::move(condition), std::move(body));
}

ASTNodePtr Parser::parseFor() {
  expect(consume().GetType(), TokenType::FOR);
  expect(consume().GetType(), TokenType::OPEN_PARENTHESES);
  ASTNodePtr init = nullptr;
  if (peek().GetType() != TokenType::SEMICOLON) {
    // could be declaration or expression
    if (isSpecifier(peek().GetType())) {
      // semicolon included in parseVariableDeclaration
      init = parseVariableDeclaration();
    } else {
      init = parseExpression(0);
      expect(consume().GetType(), TokenType::SEMICOLON);
    }
  } else {
    consume(); // consume the semicolon
  }
  std::optional<ASTNodePtr> condition = std::nullopt;
  if (peek().GetType() != TokenType::SEMICOLON) {
    condition = parseExpression(0);
  }
  expect(consume().GetType(), TokenType::SEMICOLON);
  std::optional<ASTNodePtr> post = std::nullopt;
  if (peek().GetType() != TokenType::CLOSE_PARENTHESES) {
    post = parseExpression(0);
  }
  expect(consume().GetType(), TokenType::CLOSE_PARENTHESES);
  ASTNodePtr body = parseStatement();
  return std::make_unique<ForNode>(std::move(init), std::move(condition),
                                   std::move(post), std::move(body));
}

std::pair<std::vector<TokenType>, std::optional<TokenType>>
Parser::parseSpecifierList(std::vector<TokenType> specifier_list) {
  std::vector<TokenType> storage_classes, type;
  for (auto specifier : specifier_list) {
    if (isStorageSpecifier(specifier)) {
      storage_classes.push_back(specifier);
    } else {
      type.push_back(specifier);
    }
  }
  // type checking would be done in parse type specifier list
  if (storage_classes.size() > 1) {
    success = 0;
    errors.push_back(ParserErrorInfo(
        currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
        TokenType::WS, "multiple storage class specifiers"));
    // using WS as error type
    return {{TokenType::WS}, TokenType::WS};
  }
  std::optional<TokenType> storage_class = std::nullopt;
  if (!storage_classes.empty()) {
    storage_class = storage_classes.back();
  }
  return {type, storage_class};
}

Type Parser::parseTypeSpecifierList(std::vector<TokenType> list) {
  if (list.size() == 0 || list.size() > 3) {
    success = 0;
    errors.push_back(ParserErrorInfo(
        currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
        TokenType::WS, "multiple or zero type specifiers"));
    return Type::Error();
  }
  if (list.size() == 1) {
    switch (list[0]) {
    case TokenType::INT:
    case TokenType::SIGNED:
      return Type::Int();
    case TokenType::LONG:
      return Type::Long();
    case TokenType::UNSIGNED:
      return Type::UInt();
    case TokenType::DOUBLE:
      return Type::Double();
    default:
      success = 0;
      errors.push_back(ParserErrorInfo(
          currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
          TokenType::WS, "invalid type specifier"));
      return Type::Error();
    }
  }
  if (list.size() == 2) {
    // long int or unsigned int or signed int or ul or signeld long
    for (int i = 0; i < 2; i++) {
      if (list[i % 2] == TokenType::INT &&
          list[(i + 1) % 2] == TokenType::LONG) {
        return Type::Long();
      } else if (list[i % 2] == TokenType::UNSIGNED &&
                 list[(i + 1) % 2] == TokenType::INT) {
        return Type::UInt();
      } else if (list[i % 2] == TokenType::SIGNED &&
                 list[(i + 1) % 2] == TokenType::INT) {
        return Type::Int();
      } else if (list[i % 2] == TokenType::UNSIGNED &&
                 list[(i + 1) % 2] == TokenType::LONG) {
        return Type::ULong();
      } else if (list[i % 2] == TokenType::SIGNED &&
                 list[(i + 1) % 2] == TokenType::LONG) {
        return Type::Long();
      }
    }
  }
  if (list.size() == 3) {
    // all valid possible combinations of (signed/unsigned) (long,int)
    bool hasSigned = false, hasUnsigned = false, hasLong = false,
         hasInt = false;
    for (auto token : list) {
      if (token == TokenType::SIGNED)
        hasSigned = true;
      else if (token == TokenType::UNSIGNED)
        hasUnsigned = true;
      else if (token == TokenType::LONG)
        hasLong = true;
      else if (token == TokenType::INT)
        hasInt = true;
    }

    if (hasSigned && hasLong && hasInt && !hasUnsigned) {
      return Type::Long();
    } else if (hasUnsigned && hasLong && hasInt && !hasSigned) {
      return Type::ULong();
    } else {
      success = 0;
      errors.push_back(ParserErrorInfo(
          currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
          TokenType::WS, "invalid type specifier"));
      return Type::Error();
    }
  }
  success = 0;
  errors.push_back(ParserErrorInfo(currentToken.GetLineNumber(),
                                   currentToken.GetColumnNumber(),
                                   TokenType::WS, "invalid type specifier"));
  return Type::Error();
}