#include "parser.hpp"

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

std::map<std::string, std::string> variable_map;

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
      continue;
    }

    std::cout << red << "  " << error.lineNumber << ":" << error.columnNumber
              << ": expected " << error.expected << ", but got "
              << TokenTypeToString(error.actualToken) << "\n"
              << reset;
  }
}

Parser::Parser(const std::vector<Token> &tokens)
    : tokens(tokens), tokenSize(tokens.size() + 1) {
  this->tokens.push_back(Token());
  this->tokens.back().SetType(TokenType::END_OF_FILE);
  std::reverse(this->tokens.begin(), this->tokens.end());
  currentIndex = this->tokens.size() - 1;
}

ASTNodePtr &Parser::parseProgram() {
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
// redundant function
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
  bool isStruct = false;
  std::string structName;
  while (isSpecifier(peek().GetType())) {
    consume();
    if (currentToken.GetType() == TokenType::STRUCT) {
      expect(consume().GetType(), TokenType::IDENTIFIER);
      specifier_list.push_back(TokenType::STRUCT);
      isStruct = 1;
      structName = currentToken.GetLexeme();
      continue;
    }
    specifier_list.push_back(currentToken.GetType());
  }
  auto [type_list, storage_class] = parseSpecifierList(specifier_list);
  Type type_spec;
  if (isStruct) {
    type_spec = Type(TypeKind::STRUCT, StructType(structName));
    if (type_list.size() > 1) {
      success = 0;
      errors.push_back(ParserErrorInfo(
          currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
          peek().GetType(), "invalid struct type specifier"));
      while (peek().GetType() != TokenType::SEMICOLON &&
             peek().GetType() != TokenType::END_OF_FILE) {
        consume();
      }
    }
  } else {
    type_spec = parseTypeSpecifierList(type_list);
  }
  auto declarator = parseDeclarator();
  auto [name, type, param_names, param_types, isVariadic] =
      processDeclarator((declarator), (type_spec));
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
  functionDeclNode->param_types = param_types;
  functionDeclNode->isVariadic = isVariadic;
  if (peek().GetType() == TokenType::SEMICOLON) {
    consume();               // consume ';'
    return functionDeclNode; // function declaration without body
  }
  // function definition with body
  functionDeclNode->body = parseBlock();
  return functionDeclNode;
}

ASTNodePtr Parser::parseVariableDeclaration() {
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

  bool isStruct = false;
  std::string structName;

  while (isSpecifier(peek().GetType())) {
    consume();
    if (currentToken.GetType() == TokenType::STRUCT) {
      expect(consume().GetType(), TokenType::IDENTIFIER);
      specifier_list.push_back(TokenType::STRUCT);
      isStruct = 1;
      structName = currentToken.GetLexeme();
      continue;
    }
    specifier_list.push_back(currentToken.GetType());
  }
  auto [type_list, storage_class] = parseSpecifierList(specifier_list);
  Type type_spec;
  if (isStruct) {
    type_spec = Type(TypeKind::STRUCT, StructType(structName));
    if (type_list.size() > 1) {
      success = 0;
      errors.push_back(ParserErrorInfo(
          currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
          peek().GetType(), "invalid struct type specifier"));
      while (peek().GetType() != TokenType::SEMICOLON &&
             peek().GetType() != TokenType::END_OF_FILE) {
        consume();
      }
    }
  } else {
    type_spec = parseTypeSpecifierList(type_list);
  }
  auto declarator = parseDeclarator();
  auto [name, type, param_names, param_types,_] =
      processDeclarator((declarator), (type_spec));
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
    varDeclNode->init = parseInitializer();
  }
  expect(consume().GetType(), TokenType::SEMICOLON);
  // resolve_declaration(varDeclNode, variable_map);
  return varDeclNode;
}

ASTNodePtr Parser::parseMemberDeclaration() {
  // type-specifier+ declarator ';'
  std::vector<TokenType> specifier_list;
  if (!isSpecifier(peek().GetType())) {
    success = 0;
    errors.push_back(ParserErrorInfo(
        currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
        peek().GetType(), "expected member declaration"));
    while (peek().GetType() != TokenType::IDENTIFIER &&
           peek().GetType() != TokenType::SEMICOLON &&
           peek().GetType() != TokenType::END_OF_FILE) {
      consume();
    }
  }

  bool isStruct = false;
  std::string structName;

  while (isSpecifier(peek().GetType())) {
    consume();
    if (currentToken.GetType() == TokenType::STRUCT) {
      expect(consume().GetType(), TokenType::IDENTIFIER);
      specifier_list.push_back(TokenType::STRUCT);
      isStruct = 1;
      structName = currentToken.GetLexeme();
      continue;
    }
    specifier_list.push_back(currentToken.GetType());
  }
  auto [type_list, storage_class] = parseSpecifierList(specifier_list);
  Type type_spec;
  if (isStruct) {
    type_spec = Type(TypeKind::STRUCT, StructType(structName));
    if (type_list.size() > 1) {
      success = 0;
      errors.push_back(ParserErrorInfo(
          currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
          peek().GetType(), "invalid struct type specifier"));
      while (peek().GetType() != TokenType::SEMICOLON &&
             peek().GetType() != TokenType::END_OF_FILE) {
        consume();
      }
    }
  } else {
    type_spec = parseTypeSpecifierList(type_list);
  }

  if (storage_class.has_value()) {
    success = 0;
    errors.push_back(ParserErrorInfo(
        currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
        peek().GetType(),
        "storage class specifier not allowed in member declaration"));
    while (peek().GetType() != TokenType::SEMICOLON &&
           peek().GetType() != TokenType::END_OF_FILE) {
      consume();
    }
  }

  auto declarator = parseDeclarator();
  auto [name, type, param_names, param_types, _] =
      processDeclarator((declarator), (type_spec));
  if (type.kind == TypeKind::FUNC) {
    success = 0;
    errors.push_back(ParserErrorInfo(
        currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
        peek().GetType(), "function declarator not allowed here"));
    while (peek().GetType() != TokenType::SEMICOLON &&
           peek().GetType() != TokenType::END_OF_FILE) {
      consume();
    }
  }

  expect(consume().GetType(), TokenType::SEMICOLON);
  std::unique_ptr<MemberDeclarationNode> memberDeclNode =
      std::make_unique<MemberDeclarationNode>(
          std::move(std::make_unique<Type>(type)), name);

  return memberDeclNode;
}

// redundant function
ASTNodePtr Parser::parseStructDeclaration() {
  expect(consume().GetType(), TokenType::STRUCT);
  expect(consume().GetType(), TokenType::IDENTIFIER);
  std::string struct_name = currentToken.GetLexeme();

  if (peek().GetType() == TokenType::SEMICOLON) {
    consume(); // consume ';'
    std::unique_ptr<StructDeclarationNode> structDeclNode =
        std::make_unique<StructDeclarationNode>(
            struct_name, std::vector<std::unique_ptr<MemberDeclarationNode>>{});
    return structDeclNode; // forward declaration
  }

  expect(consume().GetType(), TokenType::OPEN_BRACE);
  std::vector<ASTNodePtr> members;

  if (peek().GetType() == TokenType::CLOSE_BRACE) {
    // empty struct not allowed
    success = 0;
    errors.push_back(ParserErrorInfo(
        currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
        peek().GetType(), "struct must have at least one member"));
    expect(consume().GetType(), TokenType::CLOSE_BRACE);
    std::unique_ptr<StructDeclarationNode> structDeclNode =
        std::make_unique<StructDeclarationNode>(
            struct_name, std::vector<std::unique_ptr<MemberDeclarationNode>>{});
    return structDeclNode;
  }

  while (peek().GetType() != TokenType::CLOSE_BRACE &&
         peek().GetType() != TokenType::END_OF_FILE) {
    members.push_back(parseMemberDeclaration());
  }
  expect(consume().GetType(), TokenType::CLOSE_BRACE);

  std::vector<std::unique_ptr<MemberDeclarationNode>> member_nodes;
  for (auto &member : members) {
    member_nodes.push_back(std::unique_ptr<MemberDeclarationNode>(
        dynamic_cast<MemberDeclarationNode *>(member.release())));
  }

  return std::make_unique<StructDeclarationNode>(struct_name,
                                                 std::move(member_nodes));
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
    if (peek().GetType() == TokenType::SEMICOLON) {
      statementNode = std::make_unique<ReturnStatement>();
      expect(consume().GetType(), TokenType::SEMICOLON);
      break;
    }
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
    if (currentIndex >= 1) {
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
  // resolve_statement(statementNode, variable_map);
  return statementNode;
}

ASTNodePtr Parser::parsePostfixExp() {
  ASTNodePtr postfixExpNode = parsePrimaryExp();
  // (subscript | increment | decrement | dot | arrow)*
  while (true) {
    if (peek().GetType() == TokenType::OPEN_BRACKET) {
      consume(); // consume '['
      ASTNodePtr indexExp = parseExpression(0);
      expect(consume().GetType(), TokenType::CLOSE_BRACKET);
      postfixExpNode = std::make_unique<SubscriptExpression>(
          std::move(postfixExpNode), std::move(indexExp));
    } else if (peek().GetType() == TokenType::INCREMENT_OPERATOR) {
      consume(); // consume '++'
      postfixExpNode = std::make_unique<PostfixExpression>(
          std::move(postfixExpNode), TokenType::INCREMENT_OPERATOR);
    } else if (peek().GetType() == TokenType::DECREMENT_OPERATOR) {
      consume(); // consume '--'
      postfixExpNode = std::make_unique<PostfixExpression>(
          std::move(postfixExpNode), TokenType::DECREMENT_OPERATOR);
    } else if (peek().GetType() == TokenType::DOT) {
      consume(); // consume '.'
      expect(consume().GetType(), TokenType::IDENTIFIER);
      std::string memberName = currentToken.GetLexeme();
      postfixExpNode = std::make_unique<DotExpression>(
          std::move(postfixExpNode), memberName);
    } else if (peek().GetType() == TokenType::ARROW_OPERATOR) {
      consume(); // consume '->'
      expect(consume().GetType(), TokenType::IDENTIFIER);
      std::string memberName = currentToken.GetLexeme();
      // a->b is equivalent to (*(a)).b
      ASTNodePtr derefNode = std::make_unique<UnaryExpression>(
          TokenType::ASTERISK, std::move(postfixExpNode));
      postfixExpNode =
          std::make_unique<DotExpression>(std::move(derefNode), memberName);
    } else {
      break;
    }
  }
  return postfixExpNode;
}

ASTNodePtr Parser::parsePrimaryExp() {
  ASTNodePtr primaryExpNode;
  switch (peek().GetType()) {
  case TokenType::INT_CONSTANT:
    consume();
    try {
      // Try parsing as int first
      std::string lexeme = currentToken.GetLexeme();
      int value = 0;
      if (!lexeme.empty()) {
        value = std::stoi(lexeme);
      }
      primaryExpNode = std::make_unique<ConstantExpression>(value);
    } catch (const std::exception &e) {
      // If int parsing fails, try parsing as long
      try {
        std::string lexeme = currentToken.GetLexeme();
        long value = 0;
        if (!lexeme.empty()) {
          value = std::stol(lexeme);
        }
        primaryExpNode = std::make_unique<ConstantExpression>(value);
      } catch (const std::exception &e1) {
        // If long parsing fails, try as unsigned int
        try {
          std::string lexeme = currentToken.GetLexeme();
          unsigned long value = 0;
          if (!lexeme.empty()) {
            value = std::stoul(lexeme);
          }
          primaryExpNode = std::make_unique<ConstantExpression>(value);
        } catch (const std::exception &e3) {
            // All parsing attempts failed
            primaryExpNode = std::make_unique<ConstantExpression>(0);
            success = 0;
            errors.push_back(ParserErrorInfo(
                currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
                currentToken.GetType(), "invalid integer constant"));
        }
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
      primaryExpNode = std::make_unique<ConstantExpression>(value);
    } catch (const std::exception &e) {
      try {
        std::string lexeme = currentToken.GetLexeme();
        unsigned long value = 0;
        if (!lexeme.empty()) {
          value = std::stoul(lexeme);
        }
        primaryExpNode = std::make_unique<ConstantExpression>(value);
      } catch (const std::exception &e1) {

        primaryExpNode = std::make_unique<ConstantExpression>(0L);
        success = 0;
        errors.push_back(ParserErrorInfo(
            currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
            currentToken.GetType(), "long integer constant"));
      }
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
      primaryExpNode = std::make_unique<ConstantExpression>(value);
    } catch (const std::exception &e) {
      try {
        std::string lexeme = currentToken.GetLexeme();
        unsigned long value = 0;
        if (!lexeme.empty()) {
          value = std::stoul(lexeme);
        }
        primaryExpNode = std::make_unique<ConstantExpression>(value);
      } catch (const std::exception &e1) {

        primaryExpNode = std::make_unique<ConstantExpression>((unsigned int)0);
        success = 0;
        errors.push_back(ParserErrorInfo(
            currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
            currentToken.GetType(), "unsigned int constant"));
      }
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
      primaryExpNode = std::make_unique<ConstantExpression>(value);
    } catch (const std::exception &e) {
      primaryExpNode = std::make_unique<ConstantExpression>((unsigned int)0);
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
      primaryExpNode = std::make_unique<ConstantExpression>(value);
    } catch (const std::exception &e) {
      primaryExpNode = std::make_unique<ConstantExpression>((unsigned int)0);
      success = 0;
      errors.push_back(ParserErrorInfo(
          currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
          currentToken.GetType(), "double constant"));
    }
    break;
  case TokenType::CHARACTER: {
    consume();
    std::string lexeme = currentToken.GetLexeme();
    char value = lexeme[0];
    primaryExpNode = std::make_unique<ConstantExpression>((int)value);
    break;
  }
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
      primaryExpNode =
          std::make_unique<FunctionCallNode>(identifier, std::move(args));
    } else {
      primaryExpNode = std::make_unique<VariableExpression>(identifier);
    }
    break;
  }
  case TokenType::STRING: {
    std::string str_value = consume().GetLexeme();
    while (peek().GetType() == TokenType::STRING) {
      str_value += consume().GetLexeme();
    }
    primaryExpNode = std::make_unique<StringLiteralExpression>(str_value);
    break;
  }
  case TokenType::OPEN_PARENTHESES: {
    consume(); // consume '('
    primaryExpNode = parseExpression(0);
    expect(consume().GetType(), TokenType::CLOSE_PARENTHESES);
    break;
  }
  default:
    success = 0;
    while (currentToken.GetType() != TokenType::SEMICOLON &&
           currentToken.GetType() != TokenType::END_OF_FILE &&
           currentToken.GetType() != TokenType::CLOSE_PARENTHESES) {
      consume();
    }
    errors.push_back(ParserErrorInfo(
        currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
        currentToken.GetType(), "primary expression"));
    // Create a dummy constant expression for error recovery
    primaryExpNode = std::make_unique<ConstantExpression>(0);
    break;
  }
  return primaryExpNode;
}

ASTNodePtr Parser::parseCastExp() {
  ASTNodePtr castExpNode;
  switch (peek().GetType()) {
  case TokenType::OPEN_PARENTHESES:
    if (isTypeSpecifier(tokens[currentIndex - 1].GetType())) {
      // cast expression
      consume();

      bool isStruct = false;
      std::string structName;

      std::vector<TokenType> type_list;
      while (isTypeSpecifier(peek().GetType())) {
        if (peek().GetType() == TokenType::STRUCT) {
          consume();
          expect(consume().GetType(), TokenType::IDENTIFIER);
          type_list.push_back(TokenType::STRUCT);
          isStruct = 1;
          structName = currentToken.GetLexeme();
          continue;
        }
        type_list.push_back(consume().GetType());
      }
      Type baseType;
      if (isStruct) {
        baseType = Type(TypeKind::STRUCT, StructType(structName));
        if (type_list.size() > 1) {
          success = 0;
          errors.push_back(ParserErrorInfo(
              currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
              peek().GetType(), "invalid struct type specifier"));
          while (peek().GetType() != TokenType::CLOSE_PARENTHESES &&
                 peek().GetType() != TokenType::END_OF_FILE) {
            consume();
          }
        }
      } else {
        baseType = parseTypeSpecifierList(type_list);
      }
      Type targetType;
      // abstract declarator
      if (peek().GetType() != TokenType::CLOSE_PARENTHESES) {
        auto abstractDeclarator = parseAbstractDeclarator();
        targetType = processAbstractDeclarator(std::move(abstractDeclarator),
                                               std::move(baseType));
      } else {
        targetType = std::move(baseType);
      }
      expect(consume().GetType(), TokenType::CLOSE_PARENTHESES);
      ASTNodePtr expr = parseCastExp();
      castExpNode = std::make_unique<CastExpression>(std::move(targetType),
                                                     std::move(expr));
      break;
    }
    castExpNode = parseUnaryExp();
    break;
  default: {
    castExpNode = parseUnaryExp();
    break;
  }
  }
  return castExpNode;
}

ASTNodePtr Parser::parseUnaryExp() {
  ASTNodePtr unaryExpNode;
  switch (peek().GetType()) {
  case TokenType::HYPHEN:
  case TokenType::TILDE:
  case TokenType::NOT:
  case TokenType::AAND:
  case TokenType::ASTERISK:
  case TokenType::INCREMENT_OPERATOR:
  case TokenType::DECREMENT_OPERATOR:
    consume();
    {
      TokenType op = currentToken.GetType();
      ASTNodePtr operand = parseCastExp();
      if (op == TokenType::ASTERISK) {
        unaryExpNode =
            std::make_unique<DereferenceExpression>(std::move(operand));
      } else if (op == TokenType::AAND) {
        unaryExpNode =
            std::make_unique<AddressOfExpression>(std::move(operand));
      } else {
        unaryExpNode =
            std::make_unique<UnaryExpression>(op, std::move(operand));
      }
    }
    break;
  case TokenType::SIZEOF: {
    consume();
    if (peek().GetType() == TokenType::OPEN_PARENTHESES &&
        isTypeSpecifier(tokens[currentIndex - 1].GetType())) {
      // sizeof(type)
      consume(); // consume '('
      std::vector<TokenType> type_list;

      bool isStruct = false;
      std::string structName;

      while (isTypeSpecifier(peek().GetType())) {
        if (peek().GetType() == TokenType::STRUCT) {
          consume();
          expect(consume().GetType(), TokenType::IDENTIFIER);
          type_list.push_back(TokenType::STRUCT);
          isStruct = 1;
          structName = currentToken.GetLexeme();
          continue;
        }
        type_list.push_back(consume().GetType());
      }
      Type baseType;
      if (isStruct) {
        baseType = Type(TypeKind::STRUCT, StructType(structName));
        if (type_list.size() > 1) {
          success = 0;
          errors.push_back(ParserErrorInfo(
              currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
              peek().GetType(), "invalid struct type specifier"));
          while (peek().GetType() != TokenType::CLOSE_PARENTHESES &&
                 peek().GetType() != TokenType::END_OF_FILE) {
            consume();
          }
        }
      } else {
        baseType = parseTypeSpecifierList(type_list);
      }
      Type targetType;
      if (peek().GetType() != TokenType::CLOSE_PARENTHESES) {
        auto abstractDeclarator = parseAbstractDeclarator();
        targetType = processAbstractDeclarator(std::move(abstractDeclarator),
                                               std::move(baseType));
      } else {
        targetType = std::move(baseType);
      }
      expect(consume().GetType(), TokenType::CLOSE_PARENTHESES);
      unaryExpNode = std::make_unique<SizeofTypeExpression>(
          std::move(std::make_unique<Type>(targetType)));
    } else {
      // sizeof expression
      ASTNodePtr expr = parseUnaryExp();
      unaryExpNode = std::make_unique<SizeofExpression>(std::move(expr));
    }
    break;
  }
  default:
    unaryExpNode = parsePostfixExp();
    break;
  }
  return unaryExpNode;
}

ASTNodePtr Parser::parseExpression(int minPrecedence) {
  ASTNodePtr left = parseCastExp();
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
  // DeclarationNodePtr declarationNode = std::make_unique<DeclarationNode>();

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

  bool isStruct = false;
  std::string structName;

  while (isSpecifier(peek().GetType())) {
    consume();
    if (currentToken.GetType() == TokenType::STRUCT) {
      expect(consume().GetType(), TokenType::IDENTIFIER);
      specifier_list.push_back(TokenType::STRUCT);
      isStruct = 1;
      structName = currentToken.GetLexeme();
      continue;
    }
    specifier_list.push_back(currentToken.GetType());
  }
  auto [type_list, storage_class] = parseSpecifierList(specifier_list);
  Type type_spec;
  if (isStruct) {
    type_spec = Type(TypeKind::STRUCT, StructType(structName));
    if (type_list.size() > 1) {
      success = 0;
      errors.push_back(ParserErrorInfo(
          currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
          peek().GetType(), "invalid struct type specifier"));
      while (peek().GetType() != TokenType::SEMICOLON &&
             peek().GetType() != TokenType::END_OF_FILE) {
        consume();
      }
    }
  } else {
    type_spec = parseTypeSpecifierList(type_list);
  }
  if (peek().GetType() == TokenType::SEMICOLON && isStruct) {
    // struct declaration without members
    consume(); // consume ';'
    std::unique_ptr<StructDeclarationNode> structDeclNode =
        std::make_unique<StructDeclarationNode>(
            structName, std::vector<std::unique_ptr<MemberDeclarationNode>>{});

    return structDeclNode;
  } else if (peek().GetType() == TokenType::OPEN_BRACE && isStruct) {
    // struct declaration with members
    consume(); // consume '{'
    std::vector<ASTNodePtr> members;

    if (peek().GetType() == TokenType::CLOSE_BRACE) {
      // empty struct not allowed
      success = 0;
      errors.push_back(ParserErrorInfo(
          currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
          peek().GetType(), "struct must have at least one member"));
      expect(consume().GetType(), TokenType::CLOSE_BRACE);
      std::unique_ptr<StructDeclarationNode> structDeclNode =
          std::make_unique<StructDeclarationNode>(
              structName,
              std::vector<std::unique_ptr<MemberDeclarationNode>>{});
    }

    while (peek().GetType() != TokenType::CLOSE_BRACE &&
           peek().GetType() != TokenType::END_OF_FILE) {
      members.push_back(parseMemberDeclaration());
    }
    expect(consume().GetType(), TokenType::CLOSE_BRACE);
    expect(consume().GetType(), TokenType::SEMICOLON);
    std::vector<std::unique_ptr<MemberDeclarationNode>> member_nodes;
    for (auto &member : members) {
      member_nodes.push_back(std::unique_ptr<MemberDeclarationNode>(
          dynamic_cast<MemberDeclarationNode *>(member.release())));
    }
    std::unique_ptr<StructDeclarationNode> structDeclNode =
        std::make_unique<StructDeclarationNode>(structName,
                                                std::move(member_nodes));
    return structDeclNode;
  }
  auto declarator = parseDeclarator();
  auto [name, type, param_names, param_types, isVariadic] =
      processDeclarator((declarator), (type_spec));
  if (type.kind == TypeKind::FUNC) {
    FunDeclNodePtr functionDeclNode = std::make_unique<FunDeclNode>();
    functionDeclNode->type = std::move(type);
    functionDeclNode->storage_class = storage_class;
    functionDeclNode->name = name;
    functionDeclNode->param_names = param_names;
    functionDeclNode->param_types = param_types;
    functionDeclNode->isVariadic = isVariadic;
    if (peek().GetType() == TokenType::SEMICOLON) {
      consume();               // consume ';'
      return functionDeclNode; // function declaration without body
    }
    // function definition with body
    functionDeclNode->body = parseBlock();
    return functionDeclNode;
  } else if (type.kind == TypeKind::ARRAY) {
    VarDeclNodePtr varDeclNode = std::make_unique<VarDeclNode>();
    varDeclNode->name = name;
    varDeclNode->type = std::move(type);
    varDeclNode->storage_class = storage_class;
    if (peek().GetType() == TokenType::ASSIGNMENT) {
      consume(); // consume '='
      varDeclNode->init = parseInitializer();
    }
    expect(consume().GetType(), TokenType::SEMICOLON);
    return varDeclNode;
  } else {
    VarDeclNodePtr varDeclNode = std::make_unique<VarDeclNode>();
    varDeclNode->name = name;
    varDeclNode->type = std::move(type);
    varDeclNode->storage_class = storage_class;
    if (peek().GetType() == TokenType::ASSIGNMENT) {
      consume(); // consume '='
      varDeclNode->init = parseInitializer();
    }
    expect(consume().GetType(), TokenType::SEMICOLON);
    // resolve_declaration(varDeclNode, variable_map);
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
    } else {
      // zero or more array declarators
      while (peek().GetType() == TokenType::OPEN_BRACKET) {
        consume(); // consume '['
        ASTNodePtr sizeNode = parsePrimaryExp();
        if (!dynamic_cast<ConstantExpression *>(sizeNode.get())) {
          success = 0;
          errors.push_back(ParserErrorInfo(
              currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
              currentToken.GetType(),
              "array size must be a constant expression"));
        }
        int size;
        ConstantExpression *constExpr =
            dynamic_cast<ConstantExpression *>(sizeNode.get());
        if (int *ptr = std::get_if<int>(&(constExpr->value))) {
          size = *ptr;
        } else if (unsigned int *ptr =
                       std::get_if<unsigned int>(&(constExpr->value))) {
          size = static_cast<int>(*ptr);
        } else if (long *ptr = std::get_if<long>(&(constExpr->value))) {
          size = static_cast<int>(*ptr);
        } else if (unsigned long *ptr =
                       std::get_if<unsigned long>(&(constExpr->value))) {
          size = static_cast<int>(*ptr);
        } else if (char *ptr = std::get_if<char>(&(constExpr->value))) {
          size = static_cast<int>(*ptr);
        } else if (unsigned char *ptr =
                       std::get_if<unsigned char>(&(constExpr->value))) {
          size = static_cast<int>(*ptr);
        } else {
          success = 0;
          errors.push_back(ParserErrorInfo(
              currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
              TokenType::WS, "invalid array size"));
        }
        expect(consume().GetType(), TokenType::CLOSE_BRACKET);
        declaratorNode =
            std::make_unique<ArrayDeclarator>(std::move(declaratorNode), size);
      }
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
    } else {
      // one or more array declarators
      while (peek().GetType() == TokenType::OPEN_BRACKET) {
        consume(); // consume '['
        ASTNodePtr sizeNode = parsePrimaryExp();
        if (!dynamic_cast<ConstantExpression *>(sizeNode.get())) {
          success = 0;
          errors.push_back(ParserErrorInfo(
              currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
              currentToken.GetType(),
              "array size must be a constant expression"));
        }
        int size;
        ConstantExpression *constExpr =
            dynamic_cast<ConstantExpression *>(sizeNode.get());
        if (int *ptr = std::get_if<int>(&(constExpr->value))) {
          size = *ptr;
        } else if (unsigned int *ptr =
                       std::get_if<unsigned int>(&(constExpr->value))) {
          size = static_cast<int>(*ptr);
        } else if (long *ptr = std::get_if<long>(&(constExpr->value))) {
          size = static_cast<int>(*ptr);
        } else if (unsigned long *ptr =
                       std::get_if<unsigned long>(&(constExpr->value))) {
          size = static_cast<int>(*ptr);
        } else if (char *ptr = std::get_if<char>(&(constExpr->value))) {
          size = static_cast<int>(*ptr);
        } else if (unsigned char *ptr =
                       std::get_if<unsigned char>(&(constExpr->value))) {
          size = static_cast<int>(*ptr);
        } else {
          success = 0;
          errors.push_back(ParserErrorInfo(
              currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
              TokenType::WS, "invalid array size"));
        }
        expect(consume().GetType(), TokenType::CLOSE_BRACKET);
        declaratorNode =
            std::make_unique<ArrayDeclarator>(std::move(declaratorNode), size);
      }
    }
    return declaratorNode;
  }
  case TokenType::ASTERISK:
    // pointer declarator
    consume();
    declaratorNode = std::make_unique<PointerDeclarator>(parseDeclarator());
    break;
  default:
    success = 0;
    errors.push_back(ParserErrorInfo(currentToken.GetLineNumber(),
                                     currentToken.GetColumnNumber(),
                                     peek().GetType(), "expected declarator"));
    // Create a dummy identifier for error recovery
    declaratorNode = std::make_unique<Ident>("error_identifier");
    break;
  }
  return declaratorNode;
}

ASTNodePtr Parser::parseDirectAbstractDeclarator() {
  ASTNodePtr declaratorNode;
  switch (peek().GetType()) {
  case TokenType::OPEN_PARENTHESES: {
    consume(); // consume '('
    auto inner = parseAbstractDeclarator();
    expect(consume().GetType(), TokenType::CLOSE_PARENTHESES);
    // zero or more array declarators
    while (peek().GetType() == TokenType::OPEN_BRACKET) {
      consume(); // consume '['
      ASTNodePtr sizeNode = parsePrimaryExp();
      if (!dynamic_cast<ConstantExpression *>(sizeNode.get())) {
        success = 0;
        errors.push_back(ParserErrorInfo(
            currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
            currentToken.GetType(),
            "array size must be a constant expression"));
      }
      int size;
      ConstantExpression *constExpr =
          dynamic_cast<ConstantExpression *>(sizeNode.get());
      if (int *ptr = std::get_if<int>(&(constExpr->value))) {
        size = *ptr;
      } else if (unsigned int *ptr =
                     std::get_if<unsigned int>(&(constExpr->value))) {
        size = static_cast<int>(*ptr);
      } else if (long *ptr = std::get_if<long>(&(constExpr->value))) {
        size = static_cast<int>(*ptr);
      } else if (unsigned long *ptr =
                     std::get_if<unsigned long>(&(constExpr->value))) {
        size = static_cast<int>(*ptr);
      } else if (char *ptr = std::get_if<char>(&(constExpr->value))) {
        size = static_cast<int>(*ptr);
      } else if (unsigned char *ptr =
                     std::get_if<unsigned char>(&(constExpr->value))) {
        size = static_cast<int>(*ptr);
      } else {
        success = 0;
        errors.push_back(ParserErrorInfo(currentToken.GetLineNumber(),
                                         currentToken.GetColumnNumber(),
                                         TokenType::WS, "invalid array size"));
      }
      expect(consume().GetType(), TokenType::CLOSE_BRACKET);
      inner = std::make_unique<AbstractArray>(std::move(inner), size);
    }
    declaratorNode = std::move(inner);
    break;
  }
  case TokenType::OPEN_BRACKET: {
    // one or more array declarators
    ASTNodePtr inner = std::make_unique<AbstractBase>();
    while (peek().GetType() == TokenType::OPEN_BRACKET) {
      consume(); // consume '['
      ASTNodePtr sizeNode = parsePrimaryExp();
      if (!dynamic_cast<ConstantExpression *>(sizeNode.get())) {
        success = 0;
        errors.push_back(ParserErrorInfo(
            currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
            currentToken.GetType(),
            "array size must be a constant expression"));
      }
      int size;
      ConstantExpression *constExpr =
          dynamic_cast<ConstantExpression *>(sizeNode.get());
      if (int *ptr = std::get_if<int>(&(constExpr->value))) {
        size = *ptr;
      } else if (unsigned int *ptr =
                     std::get_if<unsigned int>(&(constExpr->value))) {
        size = static_cast<int>(*ptr);
      } else if (long *ptr = std::get_if<long>(&(constExpr->value))) {
        size = static_cast<int>(*ptr);
      } else if (unsigned long *ptr =
                     std::get_if<unsigned long>(&(constExpr->value))) {
        size = static_cast<int>(*ptr);
      } else if (char *ptr = std::get_if<char>(&(constExpr->value))) {
        size = static_cast<int>(*ptr);
      } else if (unsigned char *ptr =
                     std::get_if<unsigned char>(&(constExpr->value))) {
        size = static_cast<int>(*ptr);
      } else {
        success = 0;
        errors.push_back(ParserErrorInfo(currentToken.GetLineNumber(),
                                         currentToken.GetColumnNumber(),
                                         TokenType::WS, "invalid array size"));
      }
      expect(consume().GetType(), TokenType::CLOSE_BRACKET);
      inner = std::make_unique<AbstractArray>(std::move(inner), size);
    }
    declaratorNode = std::move(inner);
    break;
  }
  default:
    declaratorNode = std::make_unique<AbstractBase>();
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
  default:
    return parseDirectAbstractDeclarator();
  }
}

std::tuple<std::string, Type, std::vector<std::string>, std::vector<Type>, bool >
Parser::processDeclarator(ASTNodePtr &declaratorNode, Type &baseType) {
  std::string name;
  std::vector<std::string> param_names;
  std::vector<Type> param_types;
  Type type = std::move(baseType);
  if (auto ident = dynamic_cast<Ident *>(declaratorNode.get())) {
    name = ident->identifier;
    return std::make_tuple(name, type, param_names, param_types,false);
  } else if (auto pointerDecl =
                 dynamic_cast<PointerDeclarator *>(declaratorNode.get())) {
    type = Type(TypeKind::POINTER, PointerType(std::make_unique<Type>(type)));
    return processDeclarator((pointerDecl->declarator), (type));
  } else if (auto funDecl =
                 dynamic_cast<FunDeclarator *>(declaratorNode.get())) {
    std::vector<Type> paramtypes;
    bool isVariadic = false;
    if(funDecl->params.size()!=0 && funDecl->params.back().type.kind  == TypeKind::ERROR) {
      // variadic function
      funDecl->params.pop_back();
      isVariadic = true;
    }
    for (auto &param : funDecl->params) {
  
      auto [param_name, param_type, _, __,___] =
          processDeclarator((param.declarator), (param.type));
      if (param_type.kind == TypeKind::FUNC) {
        success = 0;
        errors.push_back(ParserErrorInfo(
            currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
            TokenType::WS, "function pointers in parameter list not allowed"));
      }
      param_names.push_back(param_name);
      paramtypes.push_back(param_type);
      param_types.push_back(param_type);
    }
    type = Type::Function(paramtypes, type);
    Ident *ident = dynamic_cast<Ident *>(funDecl->declarator.get());
    if (ident) {
      return std::make_tuple(ident->identifier, type, param_names, param_types,isVariadic);
    } else {
      success = 0;
      errors.push_back(ParserErrorInfo(
          currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
          TokenType::WS, "function declarator must have an identifier"));
      // error: function declarator must have an identifier
      return std::make_tuple("", type, param_names, param_types,false);
    }
  } else if (auto arrayDecl =
                 dynamic_cast<ArrayDeclarator *>(declaratorNode.get())) {
    type = Type(TypeKind::ARRAY,
                ArrayType(std::make_shared<Type>(type), arrayDecl->size));
    return processDeclarator(arrayDecl->declarator, type);
  }
  success = 0;
  errors.push_back(ParserErrorInfo(currentToken.GetLineNumber(),
                                   currentToken.GetColumnNumber(),
                                   TokenType::WS, "unknown declarator type"));
  return std::make_tuple("", Type::Int(), param_names, param_types,false);
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
  } else if (dynamic_cast<AbstractArray *>(abstractDeclaratorNode.get())) {
    auto arrayDecl =
        dynamic_cast<AbstractArray *>(abstractDeclaratorNode.get());
    Type type =
        Type(TypeKind::ARRAY,
             ArrayType(std::make_unique<Type>(baseType), arrayDecl->size));
    return processAbstractDeclarator(std::move(arrayDecl->base),
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
  if (peek().GetType() == TokenType::VOID && currentIndex >= 1 &&
      tokens[currentIndex - 1].GetType() == TokenType::CLOSE_PARENTHESES) {
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

    bool isStruct = false;
    std::string structName;

    while (isSpecifier(peek().GetType())) {
      consume();
      if (currentToken.GetType() == TokenType::STRUCT) {
        expect(consume().GetType(), TokenType::IDENTIFIER);
        specifier_list.push_back(TokenType::STRUCT);
        isStruct = 1;
        structName = currentToken.GetLexeme();
        continue;
      }
      specifier_list.push_back(currentToken.GetType());
    }
    auto [type_list, storage_class] = parseSpecifierList(specifier_list);
    Type param_type;
    if (isStruct) {
      param_type = Type(TypeKind::STRUCT, StructType(structName));
      if (type_list.size() > 1) {
        success = 0;
        errors.push_back(ParserErrorInfo(
            currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
            peek().GetType(), "invalid struct type specifier"));
        while (peek().GetType() != TokenType::COMMA &&
               peek().GetType() != TokenType::CLOSE_PARENTHESES &&
               peek().GetType() != TokenType::END_OF_FILE) {
          consume();
        }
      }
    } else {
      param_type = parseTypeSpecifierList(type_list);
    }
    if (storage_class.has_value()) {
      success = 0;
      errors.push_back(ParserErrorInfo(
          currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
          peek().GetType(),
          "storage class specifier not allowed in parameter declaration"));
    }
    auto declarator = parseDeclarator();
    paraminfo param(param_type, std::move(declarator));
    params.push_back(std::move(param));
    if (peek().GetType() == TokenType::COMMA) {
      consume(); // consume ','
      if (peek().GetType() == TokenType::CLOSE_PARENTHESES) {
        success = 0;
        errors.push_back(ParserErrorInfo(
            currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
            peek().GetType(), "trailing comma in parameter list"));
      }
      if( peek().GetType() == TokenType::ELLIPSIS) {
        consume(); // consume '...'
        params.push_back(paraminfo(Type::Error(), nullptr));
        expect(peek().GetType(), TokenType::CLOSE_PARENTHESES);
        break;
      }
      continue; // parse next parameter
    } else {
      expect(peek().GetType(), TokenType::CLOSE_PARENTHESES);
    }
  }
  expect(consume().GetType(), TokenType::CLOSE_PARENTHESES);
  return params;
}

std::unique_ptr<InitializerNode> Parser::parseInitializer() {
  std::unique_ptr<InitializerNode> initializerNode;
  if (peek().GetType() == TokenType::OPEN_BRACE) {
    consume(); // consume '{'

    if (peek().GetType() == TokenType::CLOSE_BRACE) {
      // empty initializer list is invalid
      success = 0;
      errors.push_back(ParserErrorInfo(
          currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
          TokenType::WS, "non empty initializer list"));
    }

    std::vector<InitializerNode> initializers;
    while (peek().GetType() != TokenType::CLOSE_BRACE &&
           peek().GetType() != TokenType::END_OF_FILE) {
      initializers.push_back(std::move(*parseInitializer()));
      if (peek().GetType() == TokenType::COMMA) {
        consume(); // consume ','
        continue;  // parse next initializer
      } else {
        break; // end of initializer list
      }
    }
    expect(consume().GetType(), TokenType::CLOSE_BRACE);
    initializerNode =
        std::make_unique<InitializerNode>(std::move(initializers));
  } else {
    ASTNodePtr expr = parseExpression(0);
    initializerNode = std::make_unique<InitializerNode>(std::move(expr));
  }
  return initializerNode;
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
    case TokenType::CHAR:
      return Type::Char();
    case TokenType::VOID:
      return Type::Void();
    default:
      success = 0;
      errors.push_back(ParserErrorInfo(
          currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
          TokenType::WS, "invalid type specifier"));
      return Type::Error();
    }
  }
  if (find(list.begin(), list.end(), TokenType::VOID) != list.end()) {
    success = 0;
    errors.push_back(ParserErrorInfo(
        currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
        TokenType::WS,
        "void type specifier cannot be combined with other type specifiers"));
    return Type::Error();
  }
  if (list.size() == 2) {
    // long int or unsigned int or signed int or ul or signeld long
    // or unsigned char or signed char
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
      } else if (list[i % 2] == TokenType::UNSIGNED &&
                 list[(i + 1) % 2] == TokenType::CHAR) {
        return Type::UChar();
      } else if (list[i % 2] == TokenType::SIGNED &&
                 list[(i + 1) % 2] == TokenType::CHAR) {
        return Type::SChar();
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