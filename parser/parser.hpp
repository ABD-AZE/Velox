#include<string>
#include<vector>
#include "token/token.hpp"

// just a temporary astnode type
struct ASTNode {
    TokenType type;
    std::string value;
    std::vector<ASTNode> children;
};

// unary operators to be handled separately while parsing
int GetPrecedence(TokenType token);
std::pair<int, int> GetBindingPower(TokenType token);
int GetUnaryPrecedence(TokenType token);
std::pair<int, int> GetUnaryBindingPower(TokenType token);

class Parser {
  public:
    Parser(const std::vector<Token>& tokens);
    // advances the index
    Token get();
    // token at current index
    Token peek();
    bool expect(TokenType expected, Token actual);
    
  private:
    std::vector<Token> tokens;
    size_t tokenSize;
    size_t currentIndex = 0; // Add this for parsing position
};