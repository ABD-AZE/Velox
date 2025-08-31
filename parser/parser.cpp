#include "parser.hpp"
#include "token/token.hpp"

int GetPrecedence(TokenType token) {
  switch (token) {
      // Precedence 16: Postfix operators
      case OPEN_PARENTHESES:  // Function calls
      case OPEN_BRACKET:      // Array subscript
      case ARROW_OPERATOR:    // Member access through pointer
      case DOT:               // Member access
          return 160;
          
      // Precedence 15: Postfix increment/decrement
      // Note: Context determines if these are prefix (14) or postfix (15)
      // This returns postfix precedence - prefix handled separately
      case INCREMENT_OPERATOR:
      case DECREMENT_OPERATOR:
          return 150;
          
      // Precedence 14: Unary operators (prefix)
      // Note: PLUS, HYPHEN, ASTERISK, AAND can be unary or binary
      // This returns unary precedence - binary handled below
      case NOT:               // Logical NOT
      case TILDE:             // Bitwise complement
      case SIZEOF:            // Size of operator
          return 140;
          
      // Precedence 13: Multiplicative operators
      case ASTERISK:          // Multiplication (binary context)
      case FORWARD_SLASH:     // Division
      case PERCENT_SIGN:      // Modulo
          return 130;
          
      // Precedence 12: Additive operators
      case PLUS:              // Addition (binary context)
      case HYPHEN:            // Subtraction (binary context)
          return 120;
          
      // Precedence 11: Shift operators
      case LEFT_SHIFT:        // <<
      case RIGHT_SHIFT:       // >>
          return 110;
          
      // Precedence 10: Relational operators
      case LESSTHAN:          // <
      case LESSTHANEQUAL:     // <=
      case GREATERTHAN:       // >
      case GREATERTHANEQUAL:  // >=
          return 100;
          
      // Precedence 9: Equality operators
      case EQUAL:             // ==
      case NOTEQUAL:          // !=
          return 90;
          
      // Precedence 8: Bitwise AND
      case AAND:              // & (binary context)
          return 80;
          
      // Precedence 7: Bitwise XOR
      case XOR:               // ^
          return 70;
          
      // Precedence 6: Bitwise OR
      case AOR:               // |
          return 60;
          
      // Precedence 5: Logical AND
      case LAND:              // &&
          return 50;
          
      // Precedence 4: Logical OR
      case LOR:               // ||
          return 40;
          
      // Precedence 3: Conditional (ternary)
      case QUESTION_MARK:     // ?
          return 30;
          
      // Precedence 2: Assignment operators
      case ASSIGNMENT:        // =
      case COMPOUND_SUM:      // +=
      case COMPOUND_DIFFERENCE: // -=
      case COMPOUND_PRODUCT:  // *=
      case COMPOUND_DIVISION: // /=
      case COMPOUND_REMAINDER: // %=
      case COMPOUND_AND:      // &=
      case COMPOUND_XOR:      // ^=
      case COMPOUND_OR:       // |=
      case COMPOUND_LEFTSHIFT: // <<=
      case COMPOUND_RIGHTSHIFT: // >>=
          return 20;
          
      // Precedence 1: Comma operator
      case COMMA:             // ,
          return 10;
          
      // Default: No precedence (not an operator or end of expression)
      default:
          return 0;
  }
}

// Returns {left_binding_power, right_binding_power} for binary operators
std::pair<int, int> GetBindingPower(TokenType token) {
  switch (token) {
      // Precedence 16: Postfix operators (left-associative)
      case OPEN_PARENTHESES:  // Function calls
      case OPEN_BRACKET:      // Array subscript
      case ARROW_OPERATOR:    // Member access through pointer
      case DOT:               // Member access
          return {160, 159};
          
      // Precedence 15: Postfix increment/decrement (left-associative)
      case INCREMENT_OPERATOR: // ++ (postfix)
      case DECREMENT_OPERATOR: // -- (postfix)
          return {150, 149};
          
      // Precedence 13: Multiplicative operators (left-associative)
      case ASTERISK:          // Multiplication
      case FORWARD_SLASH:     // Division
      case PERCENT_SIGN:      // Modulo
          return {130, 129};
          
      // Precedence 12: Additive operators (left-associative)
      case PLUS:              // Addition
      case HYPHEN:            // Subtraction
          return {120, 119};
          
      // Precedence 11: Shift operators (left-associative)
      case LEFT_SHIFT:        // <<
      case RIGHT_SHIFT:       // >>
          return {110, 109};
          
      // Precedence 10: Relational operators (left-associative)
      case LESSTHAN:          // <
      case LESSTHANEQUAL:     // <=
      case GREATERTHAN:       // >
      case GREATERTHANEQUAL:  // >=
          return {100, 99};
          
      // Precedence 9: Equality operators (left-associative)
      case EQUAL:             // ==
      case NOTEQUAL:          // !=
          return {90, 89};
          
      // Precedence 8: Bitwise AND (left-associative)
      case AAND:              // &
          return {80, 79};
          
      // Precedence 7: Bitwise XOR (left-associative)
      case XOR:               // ^
          return {70, 69};
          
      // Precedence 6: Bitwise OR (left-associative)
      case AOR:               // |
          return {60, 59};
          
      // Precedence 5: Logical AND (left-associative)
      case LAND:              // &&
          return {50, 49};
          
      // Precedence 4: Logical OR (left-associative)
      case LOR:               // ||
          return {40, 39};
          
      // Precedence 3: Conditional (ternary) - right-associative
      case QUESTION_MARK:     // ?
          return {29, 30};
          
      // Precedence 2: Assignment operators (right-associative)
      case ASSIGNMENT:        // =
      case COMPOUND_SUM:      // +=
      case COMPOUND_DIFFERENCE: // -=
      case COMPOUND_PRODUCT:  // *=
      case COMPOUND_DIVISION: // /=
      case COMPOUND_REMAINDER: // %=
      case COMPOUND_AND:      // &=
      case COMPOUND_XOR:      // ^=
      case COMPOUND_OR:       // |=
      case COMPOUND_LEFTSHIFT: // <<=
      case COMPOUND_RIGHTSHIFT: // >>=
          return {19, 20};
          
      // Precedence 1: Comma operator (left-associative)
      case COMMA:             // ,
          return {10, 9};
          
      // Default: No binding power (not a binary operator)
      default:
          return {0, 0};
  }
}

// Helper function to get unary operator precedence
int GetUnaryPrecedence(TokenType token) {
  switch (token) {
      // Precedence 14: Unary operators (right-associative)
      case INCREMENT_OPERATOR: // ++ (prefix)
      case DECREMENT_OPERATOR: // -- (prefix)
      case PLUS:              // Unary +
      case HYPHEN:            // Unary -
      case NOT:               // Logical NOT !
      case TILDE:             // Bitwise complement ~
      case ASTERISK:          // Dereference *
      case AAND:              // Address-of &
      case SIZEOF:            // Size of operator
          return 140;
          
      default:
          return 0;
  }
}

// Helper function to get unary binding power (for right-associative unary ops)
std::pair<int, int> GetUnaryBindingPower(TokenType token) {
  switch (token) {
      // Precedence 14: Unary operators (right-associative)
      case INCREMENT_OPERATOR: // ++ (prefix)
      case DECREMENT_OPERATOR: // -- (prefix)
      case PLUS:              // Unary +
      case HYPHEN:            // Unary -
      case NOT:               // Logical NOT !
      case TILDE:             // Bitwise complement ~
      case ASTERISK:          // Dereference *
      case AAND:              // Address-of &
      case SIZEOF:            // Size of operator
          return {139, 140};
          
      default:
          return {0, 0};
  }
}

Parser::Parser(const std::vector<Token>& inputtokens) : tokens(inputtokens), currentIndex(0), program(std::make_unique<AST_Node_program>()), tokenSize(tokens.size()+1) {
    Token eof;
    eof.SetType(END_OF_FILE);
    tokens.push_back(eof); // Add EOF token at the end   
}

Token Parser::get(){
    if(currentIndex < tokens.size()){
        return tokens[currentIndex++];
    }
    return Token(); // Return a default token if out of bounds
}

Token Parser::peek(){
    if(currentIndex < tokens.size()){
        return tokens[currentIndex];
    }
    return Token(); // Return a default token if out of bounds
}

bool Parser::expect(TokenType expected, TokenType actual){
    if(expected != actual) {
        // handle error here
    }
    return true;
}

void Parser::parseProgram(){
    while(currentIndex < tokenSize) {
        auto declaration = parseDeclaration();
        if (declaration) {
            program->AddDeclaration(std::move(declaration));
        }
    }
}
