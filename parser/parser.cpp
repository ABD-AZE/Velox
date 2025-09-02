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
    functionDefNode->statement = parseStatement();
    expect(consume().GetType(), TokenType::CLOSE_BRACE);
    return functionDefNode;
}

ASTNodePtr Parser::parseStatement()
{
    StatementNodePtr statementNode = std::make_unique<StatementNode>();
    expect(consume().GetType(), TokenType::RETURN);
    statementNode->expression = parseExpression();
    expect(consume().GetType(), TokenType::SEMICOLON);
    return statementNode;
}

ASTNodePtr Parser::parseExpression()
{
    ExpressionNodePtr expressionNode = std::make_unique<ExpressionNode>();
    switch (peek().GetType())
    {
        case TokenType::INT_CONSTANT:
            consume();
            try
            {
                std::string lexeme = currentToken.GetLexeme();
                if (lexeme.empty())
                {
                    expressionNode->const_token = 0;
                }
                else
                {
                    expressionNode->const_token = std::stoi(lexeme);
                }
            }
            catch (const std::exception &e)
            {
                expressionNode->const_token = 0;
            }
            break;

        case TokenType::HYPHEN:
        case TokenType::TILDE:
            consume();
            expressionNode->op = currentToken.GetType();
            expressionNode->right = parseExpression();
            break;
        case TokenType::OPEN_PARENTHESES:
            consume();
            expressionNode = std::unique_ptr<ExpressionNode>(dynamic_cast<ExpressionNode*>(parseExpression().release()));
            expect(consume().GetType(), TokenType::CLOSE_PARENTHESES);
            break;
        default:
            success = 0;
            errors.push_back(ParserErrorInfo(currentToken.GetLineNumber(), currentToken.GetColumnNumber(),
                                        currentToken.GetType(),"Integer or unary expression expected"));
            break;
    }
    return expressionNode;
}