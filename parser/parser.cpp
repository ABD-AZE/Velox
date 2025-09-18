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

Token Parser::peek(){
    if(currentIndex < tokenSize){
        currentToken = tokens[currentIndex];
    }
    return currentToken;
}

bool Parser::expect(TokenType actual, TokenType expected)
{
    if (expected != actual)
    {
        success = false;
        errors.emplace_back(currentToken.GetLineNumber(), currentToken.GetColumnNumber(), actual, "expected " + TokenTypeToString(expected));
        return false;
    }
    return true;
}

void Parser::printErrors() const
{
    for (const auto &error : errors)
    {
        std::cerr << "Error at line " << error.lineNumber << ", column " << error.columnNumber
                  << error.expected
                  << ", but got " << TokenTypeToString(error.actualToken) << std::endl;
    }
}

std::map<TokenType, int> Precedence = {
    {TokenType::END_OF_FILE, 0},
    {TokenType::ASSIGNMENT, 2},
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
};

Parser::Parser(const std::vector<Token> &tokens) : tokens(tokens), tokenSize(tokens.size() + 1)
{
    this->tokens.push_back(Token());
    this->tokens.back().SetType(TokenType::END_OF_FILE);
    std::reverse(this->tokens.begin(), this->tokens.end());
    currentIndex = this->tokens.size() - 1;
}

const ASTNodePtr &Parser::parseProgram()
{
    ProgramNodePtr programNode = std::make_unique<ProgramNode>(parseFunctionDefinition());
    ast = std::move(programNode);
    if (tokens.size() > 1)
    {
        success = false;
        errors.emplace_back(currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
                           consume().GetType()," end of file expected");
    }
    return ast;
}

ASTNodePtr Parser::parseFunctionDefinition()
{
    FunctionDefinitionNodePtr functionDefNode = std::make_unique<FunctionDefinitionNode>();
    expect(consume().GetType(), TokenType::INT);
    expect(consume().GetType(), TokenType::IDENTIFIER);
    functionDefNode->name = currentToken.GetLexeme();
    expect(consume().GetType(), TokenType::OPEN_PARENTHESES);
    expect(consume().GetType(), TokenType::VOID);
    expect(consume().GetType(), TokenType::CLOSE_PARENTHESES);
    expect(consume().GetType(), TokenType::OPEN_BRACE);
    while (peek().GetType() != TokenType::CLOSE_BRACE && peek().GetType() != TokenType::END_OF_FILE) {
        functionDefNode->block_items.push_back(parseBlockItem());
    }
    expect(consume().GetType(), TokenType::CLOSE_BRACE);
    return functionDefNode;
}

ASTNodePtr Parser::parseStatement()
{
    StatementNodePtr statementNode;
    switch (peek().GetType())
    {
        case TokenType::SEMICOLON:
            consume();
            statementNode = std::make_unique<StatementNode>();
            break;
        case TokenType::RETURN:
            expect(consume().GetType(), TokenType::RETURN);
            statementNode = std::make_unique<StatementNode>(1, parseExpression(0));
            expect(consume().GetType(), TokenType::SEMICOLON);
            break;
        // expression
        default:
            {
                statementNode = std::make_unique<StatementNode>(parseExpression(0));
                expect(consume().GetType(), TokenType::SEMICOLON);
                break;
            }
    }
    return statementNode;
}

ASTNodePtr Parser::parseFactor()
{
    ExpressionNodePtr factorNode = std::make_unique<ExpressionNode>();
    switch (peek().GetType())
    {
        case TokenType::INT_CONSTANT:
            consume();
            try
            {
                std::string lexeme = currentToken.GetLexeme();
                if (lexeme.empty())
                {
                    factorNode->const_value = 0;
                }
                else
                {
                    factorNode->const_value = std::stoi(lexeme);
                }
            }
            catch (const std::exception &e)
            {
                factorNode->const_value = 0;
                success = 0;
                errors.push_back(ParserErrorInfo(currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
                                        currentToken.GetType(),"integer constant"));
            }
            break;

        case TokenType::HYPHEN:
        case TokenType::TILDE:
        case TokenType::NOT:
            consume();
            factorNode->op = currentToken.GetType();
            factorNode->right = parseFactor();
            break;
        case TokenType::OPEN_PARENTHESES:
            consume();
            factorNode = std::unique_ptr<ExpressionNode>(dynamic_cast<ExpressionNode*>(parseExpression(0).release()));
            expect(consume().GetType(), TokenType::CLOSE_PARENTHESES);
            break;
        case TokenType::IDENTIFIER:
            consume();
            factorNode = std::make_unique<ExpressionNode>(currentToken.GetLexeme());
            break;
        default:
            success = 0;
            while(currentToken.GetType() != TokenType::SEMICOLON && currentToken.GetType() != TokenType::END_OF_FILE && currentToken.GetType() != TokenType::CLOSE_PARENTHESES)
            {       
                consume();
            }
            errors.push_back(ParserErrorInfo(currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
                                        currentToken.GetType(),"factor"));
            break;
    }
    return factorNode;
}

ASTNodePtr Parser::parseExpression(int minPrecedence){
    ASTNodePtr left = parseFactor();
    while((peek().GetType() == TokenType::ASSIGNMENT || peek().GetType() == TokenType::PLUS || peek().GetType() == TokenType::HYPHEN || peek().GetType() == TokenType::ASTERISK || peek().GetType() == TokenType::FORWARD_SLASH || peek().GetType() == TokenType::PERCENT_SIGN || peek().GetType() == TokenType::LEFT_SHIFT || peek().GetType() == TokenType::RIGHT_SHIFT || peek().GetType() == TokenType::XOR || peek().GetType() == TokenType::AAND || peek().GetType() == TokenType::AOR || peek().GetType() == TokenType::LOR || peek().GetType() == TokenType::LAND || peek().GetType() == TokenType::GREATERTHAN || peek().GetType() == TokenType::LESSTHAN || peek().GetType() == TokenType::GREATERTHANEQUAL || peek().GetType() == TokenType::LESSTHANEQUAL || peek().GetType() == TokenType::EQUAL || peek().GetType() == TokenType::NOTEQUAL) && Precedence[peek().GetType()] > minPrecedence){
        TokenType op = consume().GetType();
        ASTNodePtr right;
        if (currentToken.GetType() == TokenType::ASSIGNMENT){
            right = parseExpression(Precedence[op] - 1);
        }
        else{
            right = parseExpression(Precedence[op]);
        }
        ExpressionNodePtr newLeft = std::make_unique<ExpressionNode>(op, std::move(left), std::move(right));
        left = std::move(newLeft);
    }
    return left;
}

ASTNodePtr Parser::parseDeclaration() {
    expect(consume().GetType(), TokenType::INT);
    expect(consume().GetType(), TokenType::IDENTIFIER);
    std::string varName = currentToken.GetLexeme();
    ASTNodePtr init = nullptr;
    if (peek().GetType() == TokenType::ASSIGNMENT) {
        consume(); // consume '='
        init = parseExpression(0);
    }
    expect(consume().GetType(), TokenType::SEMICOLON);
    ExpressionNodePtr varNode = std::make_unique<ExpressionNode>(varName);
    if (init) {
        return std::make_unique<ExpressionNode>(std::move(varNode), std::move(init));
    } else {
        return varNode; // Just the variable declaration without initialization
    }
}

ASTNodePtr Parser::parseBlockItem() {
    BlockItemNodePtr blockItemNode;
    if (peek().GetType() == TokenType::INT) {
        // Parse declaration
        blockItemNode = std::make_unique<BlockItemNode>(parseDeclaration());
    } else {
        // Parse statement
        blockItemNode = std::make_unique<BlockItemNode>(parseStatement());
    }
    return blockItemNode;
}