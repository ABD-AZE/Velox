#include "parser.hpp"
#include <assert.h>
#include <iostream>


void Parser::PrintErrors(){
    for(auto& error : errors){
        std::cout << "Error at line " << error.lineNumber << ", column " << error.columnNumber << ": " << error.expectedToken << " expected, " << error.actualToken << " found" << std::endl;
    }
}

int GetPrecedence(TokenType token)
{
    switch (token)
    {
    // Precedence 16: Postfix operators
    case OPEN_PARENTHESES: // Function calls
    case OPEN_BRACKET:     // Array subscript
    case ARROW_OPERATOR:   // Member access through pointer
    case DOT:              // Member access
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
    case NOT:    // Logical NOT
    case TILDE:  // Bitwise complement
    case SIZEOF: // Size of operator
        return 140;

    // Precedence 13: Multiplicative operators
    case ASTERISK:      // Multiplication (binary context)
    case FORWARD_SLASH: // Division
    case PERCENT_SIGN:  // Modulo
        return 130;

    // Precedence 12: Additive operators
    case PLUS:   // Addition (binary context)
    case HYPHEN: // Subtraction (binary context)
        return 120;

    // Precedence 11: Shift operators
    case LEFT_SHIFT:  // <<
    case RIGHT_SHIFT: // >>
        return 110;

    // Precedence 10: Relational operators
    case LESSTHAN:         // <
    case LESSTHANEQUAL:    // <=
    case GREATERTHAN:      // >
    case GREATERTHANEQUAL: // >=
        return 100;

    // Precedence 9: Equality operators
    case EQUAL:    // ==
    case NOTEQUAL: // !=
        return 90;

    // Precedence 8: Bitwise AND
    case AAND: // & (binary context)
        return 80;

    // Precedence 7: Bitwise XOR
    case XOR: // ^
        return 70;

    // Precedence 6: Bitwise OR
    case AOR: // |
        return 60;

    // Precedence 5: Logical AND
    case LAND: // &&
        return 50;

    // Precedence 4: Logical OR
    case LOR: // ||
        return 40;

    // Precedence 3: Conditional (ternary)
    case QUESTION_MARK: // ?
        return 30;

    // Precedence 2: Assignment operators
    case ASSIGNMENT:          // =
    case COMPOUND_SUM:        // +=
    case COMPOUND_DIFFERENCE: // -=
    case COMPOUND_PRODUCT:    // *=
    case COMPOUND_DIVISION:   // /=
    case COMPOUND_REMAINDER:  // %=
    case COMPOUND_AND:        // &=
    case COMPOUND_XOR:        // ^=
    case COMPOUND_OR:         // |=
    case COMPOUND_LEFTSHIFT:  // <<=
    case COMPOUND_RIGHTSHIFT: // >>=
        return 20;

    // Precedence 1: Comma operator
    case COMMA: // ,
        return 10;

    // Default: No precedence (not an operator or end of expression)
    default:
        return 0;
    }
}

// Returns {left_binding_power, right_binding_power} for binary operators
std::pair<int, int> GetBindingPower(TokenType token)
{
    switch (token)
    {
    // Precedence 16: Postfix operators (left-associative)
    case OPEN_PARENTHESES: // Function calls
    case OPEN_BRACKET:     // Array subscript
    case ARROW_OPERATOR:   // Member access through pointer
    case DOT:              // Member access
        return {159, 160};

    // Precedence 15: Postfix increment/decrement (left-associative)
    case INCREMENT_OPERATOR: // ++ (postfix)
    case DECREMENT_OPERATOR: // -- (postfix)
        return {149, 150};

    // Precedence 13: Multiplicative operators (left-associative)
    case ASTERISK:      // Multiplication
    case FORWARD_SLASH: // Division
    case PERCENT_SIGN:  // Modulo
        return {129, 130};

    // Precedence 12: Additive operators (left-associative)
    case PLUS:   // Addition
    case HYPHEN: // Subtraction
        return {119, 120};

    // Precedence 11: Shift operators (left-associative)
    case LEFT_SHIFT:  // <<
    case RIGHT_SHIFT: // >>
        return {109, 110};

    // Precedence 10: Relational operators (left-associative)
    case LESSTHAN:         // <
    case LESSTHANEQUAL:    // <=
    case GREATERTHAN:      // >
    case GREATERTHANEQUAL: // >=
        return {99, 100};

    // Precedence 9: Equality operators (left-associative)
    case EQUAL:    // ==
    case NOTEQUAL: // !=
        return {89, 90};

    // Precedence 8: Bitwise AND (left-associative)
    case AAND: // &
        return {79, 80};

    // Precedence 7: Bitwise XOR (left-associative)
    case XOR: // ^
        return {69, 70};

    // Precedence 6: Bitwise OR (left-associative)
    case AOR: // |
        return {59, 60};

    // Precedence 5: Logical AND (left-associative)
    case LAND: // &&
        return {49, 50};

    // Precedence 4: Logical OR (left-associative)
    case LOR: // ||
        return {39, 40};

    // Precedence 3: Conditional (ternary) - right-associative
    case QUESTION_MARK: // ?
        return {30, 29};

    // Precedence 2: Assignment operators (right-associative)
    case ASSIGNMENT:          // =
    case COMPOUND_SUM:        // +=
    case COMPOUND_DIFFERENCE: // -=
    case COMPOUND_PRODUCT:    // *=
    case COMPOUND_DIVISION:   // /=
    case COMPOUND_REMAINDER:  // %=
    case COMPOUND_AND:        // &=
    case COMPOUND_XOR:        // ^=
    case COMPOUND_OR:         // |=
    case COMPOUND_LEFTSHIFT:  // <<=
    case COMPOUND_RIGHTSHIFT: // >>=
        return {20, 19};

    // Precedence 1: Comma operator (left-associative)
    case COMMA: // ,
        return {9,10};

    // Special case: Colon (used in ternary conditional) - very low precedence
    case COLON: // :
        return {1, 1};

    // Default: No binding power (not a binary operator)
    default:
        return {0, 0};
    }
}

// Helper function to get unary operator precedence
int GetUnaryPrecedence(TokenType token)
{
    switch (token)
    {
    // Precedence 14: Unary operators (right-associative)
    case INCREMENT_OPERATOR: // ++ (prefix)
    case DECREMENT_OPERATOR: // -- (prefix)
    case PLUS:               // Unary +
    case HYPHEN:             // Unary -
    case NOT:                // Logical NOT !
    case TILDE:              // Bitwise complement ~
    case ASTERISK:           // Dereference *
    case AAND:               // Address-of &
    case SIZEOF:             // Size of operator
        return 140;

    default:
        return 0;
    }
}

// Helper function to get unary binding power (for right-associative unary ops)
std::pair<int, int> GetUnaryBindingPower(TokenType token)
{
    switch (token)
    {
    // Precedence 14: Unary operators (right-associative)
    case INCREMENT_OPERATOR: // ++ (prefix)
    case DECREMENT_OPERATOR: // -- (prefix)
    case PLUS:               // Unary +
    case HYPHEN:             // Unary -
    case NOT:                // Logical NOT !
    case TILDE:              // Bitwise complement ~
    case ASTERISK:           // Dereference *
    case AAND:               // Address-of &
    case SIZEOF:             // Size of operator
        return {140, 139};

    default:
        return {0, 0};
    }
}

Parser::Parser(const std::vector<Token> &inputtokens) : tokens(inputtokens)
{
    Token eof;
    eof.SetType(END_OF_FILE);
    tokens.push_back(eof); // Add EOF token at the end
    std::reverse(tokens.begin(), tokens.end());
    tokenSize = tokens.size();
    
    // Start from the last valid index (tokens are reversed, so we start from the end)
    if (tokenSize > 0) {
        currentIndex = tokenSize - 1;
    } else {
        currentIndex = 0;
    }
    
    // Parser initialized successfully
}

Token Parser::get()
{
    if(currentIndex >= 0 && currentIndex < tokens.size())
        return tokens[currentIndex--];
    return Token(); // Return a default token if out of bounds
}

Token Parser::peek()
{
    if(currentIndex >= 0 && currentIndex < tokens.size())
        return tokens[currentIndex];
    return Token(); // Return a default token if out of bounds
}

Token Parser::peekNext()
{
    if(currentIndex - 1 >= 0 && currentIndex - 1 < tokens.size())
        return tokens[currentIndex - 1];
    return Token(); // Return a default token if out of bounds
}

/// consumes the current token and advances the index
Token Parser::consume()
{
    if(!tokens.empty()) {
        Token token = tokens.back(); // Get the last element
        tokens.pop_back(); // Remove the last element
        tokenSize = tokens.size(); // Update token size
        currentIndex = tokenSize - 1; // Reset to last index
        return token;
    }
    return Token(); // Return a default token if empty
}

void Parser::reset()
{
    currentIndex = tokenSize - 1;
}

bool Parser::expect(TokenType expected, TokenType actual)
{
    if (expected != actual)
    {
        // Set success to false to indicate parsing error
        success = false;
        
        // Get current token position for error reporting
        Token current_token = peek();
        int line = current_token.GetLineNumber();
        int column = current_token.GetColumnNumber();
        
        // Record the error for later reporting
        errors.emplace_back(line, column, expected, actual);
        
        // Continue parsing as if the expected token was present (error recovery)
        return false; // Indicates mismatch but parsing continues
    }
    return true; // Token matched as expected
}

bool Parser::expectToken(TokenType expected)
{
    Token current = peek();
    bool matched = expect(expected, current.GetType());
    
    // Always consume the token (or advance past it) for error recovery
    // If token matched, consume it normally
    // If token didn't match, still advance to continue parsing
    if (currentIndex >= 0) {
        get(); // Consume the token regardless of match for error recovery
    }
    
    return matched;
}

void Parser::printErrors() const
{
    if (errors.empty()) {
        std::cout << "No parsing errors found." << std::endl;
        return;
    }
    
    std::cout << "=== PARSING ERRORS ===" << std::endl;
    for (const auto& error : errors) {
        std::cout << "Error at line " << error.lineNumber 
                  << ", column " << error.columnNumber 
                  << ": Expected token type " << static_cast<int>(error.expectedToken)
                  << ", but found token type " << static_cast<int>(error.actualToken) 
                  << std::endl;
    }
    std::cout << "Total errors: " << errors.size() << std::endl;
}



bool isTypeSpecifier(TokenType type)
{
    switch (type)
    {
    case TokenType::CHAR:
    case TokenType::INT:
    case TokenType::LONG:
    case TokenType::DOUBLE:
    case TokenType::VOID:
    case TokenType::UNSIGNED:
    case TokenType::SIGNED:
    case TokenType::CONST:
    case TokenType::STATIC:
    case TokenType::EXTERN:
    case TokenType::STRUCT:
    case TokenType::UNION:
    case TokenType::ENUM:
    case TokenType::CLASS:
    case TokenType::TYPEDEF:
    case TokenType::VA_LIST:
    // case TokenType::TYPEDEF-NAME: TODO 
        return true;
    default:
        return false;
    }
}

// move not necessary because RVO handles it
AST_Node_declarationPtr Parser::parseDeclaration()
{
    Token token = peek();
    
    // Examining token for declaration type
    
    // Handle specific declaration types first
    switch (token.GetType()) {
    case TokenType::STRUCT: {
        auto declaration = std::make_unique<AST_Node_declaration>();
        declaration->decl_type = DeclType::DeclStruct;
        declaration->info = parseStructDeclaration();
        return declaration;
    }
    case TokenType::CLASS: {
        auto declaration = std::make_unique<AST_Node_declaration>();
        declaration->decl_type = DeclType::DeclClass;
        declaration->info = parseClassDeclaration();
        return declaration;
    }
    case TokenType::ENUM: {
        auto declaration = std::make_unique<AST_Node_declaration>();
        declaration->decl_type = DeclType::DeclEnum;
        declaration->info = parseEnumDeclaration();
        return declaration;
    }
    case TokenType::TYPEDEF: {
        auto declaration = std::make_unique<AST_Node_declaration>();
        declaration->decl_type = DeclType::DeclTypedef;
        declaration->info = parseTypedefDeclaration();
        return declaration;
    }
    case TokenType::UNION: {
        auto declaration = std::make_unique<AST_Node_declaration>();
        declaration->decl_type = DeclType::DeclUnion;
        declaration->info = parseUnionDeclaration();
        return declaration;
    }
    }
    
    // Handle type specifiers (int, char, double, void, etc.)
    bool is_type = isTypeSpecifier(token.GetType());
    
    if (!is_type) {
        // Not a type specifier, cannot parse as declaration
        return nullptr; // Not a valid declaration
    }
    
    // Parse type
    TypeType type_enum = TypeType::TypeInt; // default
    switch (token.GetType()) {
        case TokenType::INT:
            type_enum = TypeType::TypeInt;
            break;
        case TokenType::CHAR:
            type_enum = TypeType::TypeChar;
            break;
        case TokenType::DOUBLE:
            type_enum = TypeType::TypeDouble;
            break;
        case TokenType::VOID:
            type_enum = TypeType::TypeVoid;
            break;
    }
    get(); // consume type token
    
    // Must have identifier
    if (peek().GetType() != TokenType::IDENTIFIER) {
        return nullptr;
    }
    std::string identifier = get().GetLexeme();
    
    // Check if it's a function (has parentheses) or variable
    if (peek().GetType() == TokenType::OPEN_PARENTHESES) {
        // Function declaration
        auto declaration = std::make_unique<AST_Node_declaration>();
        auto func_decl = std::make_unique<AST_Node_function_declaration>();
        
        auto type = std::make_unique<AST_Node_type>();
        type->type = type_enum;
        func_decl->fun_type = std::move(type);
        func_decl->identifier = identifier;
        
        get(); // consume '('
        // Skip parameters for now
        int param_count = 0;
        while (peek().GetType() != TokenType::CLOSE_PARENTHESES && 
               peek().GetType() != TokenType::END_OF_FILE) {
            param_count++;
            if (param_count > 20) {
                break;
            }
            get();
        }
        expectToken(TokenType::CLOSE_PARENTHESES); // Required closing parenthesis with error recovery
        
        // Parse function body or semicolon
                // Checking next token
        if (peek().GetType() == TokenType::OPEN_BRACE) {
            func_decl->body = parseCompoundStatement();
        } else if (peek().GetType() == TokenType::SEMICOLON) {
            get(); // consume ';'
        } else {
        }
        
        declaration->decl_type = DeclType::DeclFunction;
        declaration->info = std::move(func_decl);
        return declaration;
    } else {
        // Variable declaration
        auto declaration = std::make_unique<AST_Node_declaration>();
        auto var_decl = std::make_unique<AST_Node_variable_declaration>();
        
        auto type = std::make_unique<AST_Node_type>();
        type->type = type_enum;
        var_decl->var_type = std::move(type);
        var_decl->identifier = identifier;
        
        // Parse initializer if present
        if (peek().GetType() == TokenType::EQUAL) {
            get(); // consume '='
            auto init = std::make_unique<AST_Node_initializer>();
            init->init_type = InitializerExp;
            init->info = parseExpression(0);
            var_decl->initializer = std::move(init);
        }
        
        if (peek().GetType() == TokenType::SEMICOLON) {
            get(); // consume ';'
        }
        
        declaration->decl_type = DeclType::DeclVariable;
        declaration->info = std::move(var_decl);
        return declaration;
    }
}

AST_Node_struct_declarationPtr Parser::parseStructDeclaration()
{
    auto structDecl = std::make_unique<AST_Node_struct_declaration>();
    Token token = get();
    expect(TokenType::STRUCT, token.GetType());
    token = get();
    expect(TokenType::IDENTIFIER, token.GetType());
    structDecl->tag = token.GetLexeme();
    if (peek().GetType() == TokenType::OPEN_BRACE)
    {
        get(); // consume {
        while (peek().GetType() != TokenType::CLOSE_BRACE && peek().GetType() != TokenType::END_OF_FILE)
        {
            if (isTypeSpecifier(peek().GetType()))
            {
                structDecl->members.push_back(parseMemberDeclaration());
            }
            else
            {
                // Skip unrecognized token to avoid infinite loop
                success = 0;
                errors.emplace_back(peek().GetLineNumber(), peek().GetColumnNumber(), TokenType::IDENTIFIER, peek().GetType());
                get(); // consume the problematic token
            }
        }
        expect(TokenType::CLOSE_BRACE, get().GetType());
        expect(TokenType::SEMICOLON, get().GetType());
    }
    else{
        expect(TokenType::SEMICOLON, get().GetType());

    }
    return structDecl;
}

AST_Node_member_declarationPtr Parser::parseMemberDeclaration()
{
    auto memberDecl = std::make_unique<AST_Node_member_declaration>();
    
    // Parse type (int, char, etc.)
    if (!isTypeSpecifier(peek().GetType())) {
        return nullptr;
    }
    
    auto type = std::make_unique<AST_Node_type>();
    switch (peek().GetType()) {
        case TokenType::INT:
            type->type = TypeType::TypeInt;
            break;
        case TokenType::CHAR:
            type->type = TypeType::TypeChar;
            break;
        case TokenType::DOUBLE:
            type->type = TypeType::TypeDouble;
            break;
        case TokenType::VOID:
            type->type = TypeType::TypeVoid;
            break;
        default:
            type->type = TypeType::TypeInt;
            break;
    }
    get(); // consume type token
    
    // Parse identifier
    if (peek().GetType() != TokenType::IDENTIFIER) {
        return nullptr;
    }
    std::string identifier = get().GetLexeme();
    
    // Consume semicolon if present
    if (peek().GetType() == TokenType::SEMICOLON) {
        get();
    }
    
    memberDecl->member_type = std::move(type);
    memberDecl->identifier = identifier;
    
    return memberDecl;
}

/// Pratt parsing for expressions

AST_Node_expPtr Parser::parseExpression(int min_bp){
    // Parse the left-hand side (primary expression or prefix operator)
    auto left = parsePrimary();
    if (!left) {
        return nullptr;
    }
    
    // Continue parsing while we have operators with sufficient binding power
    while (currentIndex >= 0) {
        Token op_token = peek();
        
        // Stop if we hit EOF (which is at index 0 after reversal)
        if (op_token.GetType() == TokenType::END_OF_FILE) {
            break;
        }
        
        auto [left_bp, right_bp] = GetBindingPower(op_token.GetType());
        
        // If the left binding power is 0 (not an operator) or less than minimum, we're done
        if (left_bp == 0 || left_bp < min_bp) {
            break;
        }
        
        // Get the operator (advance index without removing token)
        get();
        
        // Handle different types of operators
        switch (op_token.GetType()) {
            case TokenType::QUESTION_MARK: {
                // Ternary conditional operator: condition ? true_expr : false_expr
                // Parse true expression with binding power higher than colon (1)
                auto true_expr = parseExpression(2);
                if (!true_expr) {
                    return nullptr;
                }
                
                // Expect and get the colon
                if (peek().GetType() != TokenType::COLON) {
                    success = 0;
                    errors.emplace_back(peek().GetLineNumber(), peek().GetColumnNumber(), 
                                      TokenType::COLON, peek().GetType());
                    return nullptr;
                }
                get(); // get ':' (non-destructive)
                
                // Parse false expression with right associativity
                auto false_expr = parseExpression(right_bp);
                if (!false_expr) {
                    return nullptr;
                }
                
                auto conditional = std::make_unique<AST_Node_conditional>();
                conditional->condition = std::move(left);
                conditional->true_expr = std::move(true_expr);
                conditional->false_expr = std::move(false_expr);
                
                auto exp = std::make_unique<AST_Node_exp>();
                exp->expr_type = ExprType::ExprConditional;
                exp->info = std::move(conditional);
                left = std::move(exp);
                break;
            }
            
            case TokenType::OPEN_PARENTHESES: {
                // Function call: func(args...)
                auto func_call = std::make_unique<AST_Node_function_call>();
                
                // Extract function name from left expression (should be a variable)
                if (left->expr_type == ExprType::ExprVar) {
                    func_call->identifier = std::get<std::unique_ptr<AST_Node_var>>(left->info)->identifier;
                } else {
                    success = 0;
                    errors.emplace_back(op_token.GetLineNumber(), op_token.GetColumnNumber(), 
                                      TokenType::IDENTIFIER, op_token.GetType());
                    return nullptr;
                }
                
                // Parse arguments
                if (peek().GetType() != TokenType::CLOSE_PARENTHESES) {
                    do {
                        auto arg = parseExpression(0);
                        if (arg) {
                            func_call->args.push_back(std::move(arg));
                        }
                        
                        if (peek().GetType() == TokenType::COMMA) {
                            get(); // get ','
                        } else {
                            break;
                        }
                    } while (currentIndex < tokenSize);
                }
                
                expect(TokenType::CLOSE_PARENTHESES, peek().GetType());
                get(); // get ')'
                
                auto exp = std::make_unique<AST_Node_exp>();
                exp->expr_type = ExprType::ExprFunctionCall;
                exp->info = std::move(func_call);
                left = std::move(exp);
                break;
            }
            
            case TokenType::OPEN_BRACKET: {
                // Array subscript: array[index]
                get(); // consume '['
                auto right = parseExpression(0);
                if (peek().GetType() != TokenType::CLOSE_BRACKET) {
                    expect(TokenType::CLOSE_BRACKET, peek().GetType()); // Error recovery
                    break; // Exit on error
                }
                get(); // consume ']'
                
                auto subscript = std::make_unique<AST_Node_subscript>();
                subscript->array = std::move(left);
                subscript->index = std::move(right);
                
                auto exp = std::make_unique<AST_Node_exp>();
                exp->expr_type = ExprType::ExprSubscript;
                exp->info = std::move(subscript);
                left = std::move(exp);
                break;
            }
            
            case TokenType::DOT: {
                // Member access: struct.member
                get(); // consume '.'
                if (peek().GetType() != TokenType::IDENTIFIER) {
                    expect(TokenType::IDENTIFIER, peek().GetType()); // Error recovery
                    break; // Exit on error
                }
                Token member_token = get(); // Get the identifier
                
                auto dot = std::make_unique<AST_Node_dot>();
                dot->structure = std::move(left);
                dot->member = member_token.GetLexeme();
                
                auto exp = std::make_unique<AST_Node_exp>();
                exp->expr_type = ExprType::ExprDot;
                exp->info = std::move(dot);
                left = std::move(exp);
                break;
            }
            
            case TokenType::ARROW_OPERATOR: {
                // Pointer member access: ptr->member
                expect(TokenType::IDENTIFIER, peek().GetType());
                Token member_token = get();
                
                auto arrow = std::make_unique<AST_Node_arrow>();
                arrow->pointer = std::move(left);
                arrow->member = member_token.GetLexeme();
                
                auto exp = std::make_unique<AST_Node_exp>();
                exp->expr_type = ExprType::ExprArrow;
                exp->info = std::move(arrow);
                left = std::move(exp);
                break;
            }
            
            case TokenType::INCREMENT_OPERATOR:
            case TokenType::DECREMENT_OPERATOR: {
                // Postfix increment/decrement
                auto exp = std::make_unique<AST_Node_exp>();
                exp->expr_type = (op_token.GetType() == TokenType::INCREMENT_OPERATOR) ? 
                                ExprType::ExprPostInc : ExprType::ExprPostDec;
                
                exp->info = std::move(left);
                left = std::move(exp);
                break;
            }
            
            // Assignment operators
            case TokenType::ASSIGNMENT:
            case TokenType::COMPOUND_SUM:
            case TokenType::COMPOUND_DIFFERENCE:
            case TokenType::COMPOUND_PRODUCT:
            case TokenType::COMPOUND_DIVISION:
            case TokenType::COMPOUND_REMAINDER:
            case TokenType::COMPOUND_AND:
            case TokenType::COMPOUND_XOR:
            case TokenType::COMPOUND_OR:
            case TokenType::COMPOUND_LEFTSHIFT:
            case TokenType::COMPOUND_RIGHTSHIFT: {
                // Assignment operators (right-associative)
                auto right = parseExpression(right_bp);
                
                auto assignment = std::make_unique<AST_Node_assignment>();
                assignment->left = std::move(left);
                assignment->right = std::move(right);
                
                // Map token type to assignment operator type
                switch (op_token.GetType()) {
                    case TokenType::ASSIGNMENT: assignment->op = AssignOpType::Assign; break;
                    case TokenType::COMPOUND_SUM: assignment->op = AssignOpType::AddAssign; break;
                    case TokenType::COMPOUND_DIFFERENCE: assignment->op = AssignOpType::SubAssign; break;
                    case TokenType::COMPOUND_PRODUCT: assignment->op = AssignOpType::MulAssign; break;
                    case TokenType::COMPOUND_DIVISION: assignment->op = AssignOpType::DivAssign; break;
                    case TokenType::COMPOUND_REMAINDER: assignment->op = AssignOpType::ModAssign; break;
                    case TokenType::COMPOUND_AND: assignment->op = AssignOpType::AndAssign; break;
                    case TokenType::COMPOUND_XOR: assignment->op = AssignOpType::XorAssign; break;
                    case TokenType::COMPOUND_OR: assignment->op = AssignOpType::OrAssign; break;
                    case TokenType::COMPOUND_LEFTSHIFT: assignment->op = AssignOpType::ShlAssign; break;
                    case TokenType::COMPOUND_RIGHTSHIFT: assignment->op = AssignOpType::ShrAssign; break;
                    default: assignment->op = AssignOpType::Assign; break; // Default to simple assignment
                }
                
                auto exp = std::make_unique<AST_Node_exp>();
                exp->expr_type = ExprType::ExprAssignment;
                exp->info = std::move(assignment);
                left = std::move(exp);
                break;
            }
            
            // Binary operators
            default: {
                // Parse right-hand side with appropriate binding power
                auto right = parseExpression(right_bp);
                
                auto binary_op = std::make_unique<AST_Node_binary_operator>();
                
                // Map token type to binary operator type
                switch (op_token.GetType()) {
                    case TokenType::PLUS: binary_op->op = BinOpType::BinOpAdd; break;
                    case TokenType::HYPHEN: binary_op->op = BinOpType::BinOpSub; break;
                    case TokenType::ASTERISK: binary_op->op = BinOpType::BinOpMul; break;
                    case TokenType::FORWARD_SLASH: binary_op->op = BinOpType::BinOpDiv; break;
                    case TokenType::PERCENT_SIGN: binary_op->op = BinOpType::BinOpMod; break;
                    case TokenType::AAND: binary_op->op = BinOpType::BinOpAnd; break;
                    case TokenType::AOR: binary_op->op = BinOpType::BinOpOr; break;
                    case TokenType::XOR: binary_op->op = BinOpType::BinOpXor; break;
                    case TokenType::LEFT_SHIFT: binary_op->op = BinOpType::BinOpShl; break;
                    case TokenType::RIGHT_SHIFT: binary_op->op = BinOpType::BinOpShr; break;
                    case TokenType::LAND: binary_op->op = BinOpType::LogicalAnd; break;
                    case TokenType::LOR: binary_op->op = BinOpType::LogicalOr; break;
                    case TokenType::EQUAL: binary_op->op = BinOpType::Equal; break;
                    case TokenType::NOTEQUAL: binary_op->op = BinOpType::NotEqual; break;
                    case TokenType::LESSTHAN: binary_op->op = BinOpType::LessThan; break;
                    case TokenType::GREATERTHAN: binary_op->op = BinOpType::GreaterThan; break;
                    case TokenType::LESSTHANEQUAL: binary_op->op = BinOpType::LessEqual; break;
                    case TokenType::GREATERTHANEQUAL: binary_op->op = BinOpType::GreaterEqual; break;
                    default:
                        success = 0;
                        errors.emplace_back(op_token.GetLineNumber(), op_token.GetColumnNumber(), 
                                          TokenType::IDENTIFIER, op_token.GetType());
                        return nullptr;
                }
                
                auto binary_exp = std::make_unique<AST_Node_binary_exp>();
                binary_exp->op = std::move(binary_op);
                binary_exp->left = std::move(left);
                binary_exp->right = std::move(right);
                
                auto exp = std::make_unique<AST_Node_exp>();
                exp->expr_type = ExprType::ExprBinary;
                exp->info = std::move(binary_exp);
                left = std::move(exp);
                break;
            }
        }
    }
    
    return left;
}

/// Parse primary expressions and prefix operators
AST_Node_expPtr Parser::parsePrimary() {
    Token token = peek();
    
    // Handle prefix unary operators
    auto [unary_left_bp, unary_right_bp] = GetUnaryBindingPower(token.GetType());
    if (unary_left_bp > 0) {
        get(); // get the unary operator (advance index without removing token)
        
        auto operand = parseExpression(unary_right_bp);
        if (!operand) {
            return nullptr;
        }
        
        // Handle different unary operators
        switch (token.GetType()) {
            case TokenType::INCREMENT_OPERATOR:
            case TokenType::DECREMENT_OPERATOR: {
                // Prefix increment/decrement
                auto exp = std::make_unique<AST_Node_exp>();
                exp->expr_type = (token.GetType() == TokenType::INCREMENT_OPERATOR) ? 
                                ExprType::ExprPreInc : ExprType::ExprPreDec;
                
                exp->info = std::move(operand);
                return exp;
            }
            
            case TokenType::ASTERISK: {
                // Dereference operator
                auto exp = std::make_unique<AST_Node_exp>();
                exp->expr_type = ExprType::ExprDereference;
                exp->info = std::move(operand);
                return exp;
            }
            
            case TokenType::AAND: {
                // Address-of operator
                auto exp = std::make_unique<AST_Node_exp>();
                exp->expr_type = ExprType::ExprAddrof;
                exp->info = std::move(operand);
                return exp;
            }
            
            case TokenType::SIZEOF: {
                // Sizeof operator
                auto exp = std::make_unique<AST_Node_exp>();
                exp->expr_type = ExprType::ExprSizeof;
                exp->info = std::move(operand);
                return exp;
            }
            
            default: {
                // Other unary operators (-, +, !, ~)
                auto unary_op = std::make_unique<AST_Node_unary_operator>();
                
                switch (token.GetType()) {
                    case TokenType::HYPHEN: unary_op->op = UnOpType::UnOpNeg; break;
                    case TokenType::PLUS: 
                        // Unary plus - just return the operand
                        return operand;
                    case TokenType::NOT: unary_op->op = UnOpType::UnOpNot; break;
                    case TokenType::TILDE: unary_op->op = UnOpType::UnOpCompl; break;
                    default:
                        success = 0;
                        errors.emplace_back(token.GetLineNumber(), token.GetColumnNumber(), 
                                          TokenType::IDENTIFIER, token.GetType());
                        return nullptr;
                }
                
                auto unary_exp = std::make_unique<AST_Node_unary_exp>();
                unary_exp->op = std::move(unary_op);
                unary_exp->expr = std::move(operand);
                
                auto exp = std::make_unique<AST_Node_exp>();
                exp->expr_type = ExprType::ExprUnary;
                exp->info = std::move(unary_exp);
                return exp;
            }
        }
    }
    
    // Handle primary expressions
    switch (token.GetType()) {
        case TokenType::IDENTIFIER: {
            // Variable reference
            Token token_to_use = get();
            auto var = std::make_unique<AST_Node_var>();
            var->identifier = token_to_use.GetLexeme();
            
            auto exp = std::make_unique<AST_Node_exp>();
            exp->expr_type = ExprType::ExprVar;
            exp->info = std::move(var);
            return exp;
        }
        
        case TokenType::INT_CONSTANT: {
            // Integer constant
            Token token_to_use = get();
            auto constant = std::make_unique<AST_Node_const>();
            constant->type = ConstType::ConstInt;
            constant->value.intValue = std::stoi(token_to_use.GetLexeme());
            
            auto exp = std::make_unique<AST_Node_exp>();
            exp->expr_type = ExprType::ExprConstant;
            exp->info = std::move(constant);
            return exp;
        }
        
        case TokenType::LONG_CONSTANT: {
            // Long constant
            get();
            auto constant = std::make_unique<AST_Node_const>();
            constant->type = ConstType::ConstLong;
            constant->value.longValue = std::stol(token.GetLexeme());
            
            auto exp = std::make_unique<AST_Node_exp>();
            exp->expr_type = ExprType::ExprConstant;
            exp->info = std::move(constant);
            return exp;
        }
        
        case TokenType::UINT_CONSTANT: {
            // Unsigned constant
            get();
            auto constant = std::make_unique<AST_Node_const>();
            constant->type = ConstType::ConstUInt;
            constant->value.uintValue = static_cast<unsigned int>(std::stoul(token.GetLexeme()));
            
            auto exp = std::make_unique<AST_Node_exp>();
            exp->expr_type = ExprType::ExprConstant;
            exp->info = std::move(constant);
            return exp;
        }
        
        case TokenType::ULONG_CONSTANT: {
            // Unsigned long constant
            get();
            auto constant = std::make_unique<AST_Node_const>();
            constant->type = ConstType::ConstULong;
            constant->value.ulongValue = std::stoul(token.GetLexeme());
            
            auto exp = std::make_unique<AST_Node_exp>();
            exp->expr_type = ExprType::ExprConstant;
            exp->info = std::move(constant);
            return exp;
        }
        
        case TokenType::FLOAT_CONSTANT: {
            // Double constant
            get();
            auto constant = std::make_unique<AST_Node_const>();
            constant->type = ConstType::ConstDouble;
            constant->value.doubleValue = std::stod(token.GetLexeme());
            
            auto exp = std::make_unique<AST_Node_exp>();
            exp->expr_type = ExprType::ExprConstant;
            exp->info = std::move(constant);
            return exp;
        }
        
        case TokenType::CHARACTER: {
            // Character constant
            get();
            auto constant = std::make_unique<AST_Node_const>();
            constant->type = ConstType::ConstChar;
            std::string lexeme = token.GetLexeme();
            // Handle escape sequences and extract character value
            if (lexeme.length() >= 3 && lexeme[0] == '\'' && lexeme.back() == '\'') {
                if (lexeme[1] == '\\' && lexeme.length() >= 4) {
                    // Handle escape sequences
                    switch (lexeme[2]) {
                        case 'n': constant->value.charValue = '\n'; break;
                        case 't': constant->value.charValue = '\t'; break;
                        case 'r': constant->value.charValue = '\r'; break;
                        case '\\': constant->value.charValue = '\\'; break;
                        case '\'': constant->value.charValue = '\''; break;
                        case '\"': constant->value.charValue = '\"'; break;
                        case '0': constant->value.charValue = '\0'; break;
                        default: constant->value.charValue = lexeme[2]; break;
                    }
                } else {
                    constant->value.charValue = lexeme[1];
                }
            } else {
                constant->value.charValue = 0; // Error case
            }
            
            auto exp = std::make_unique<AST_Node_exp>();
            exp->expr_type = ExprType::ExprConstant;
            exp->info = std::move(constant);
            return exp;
        }
        
        case TokenType::STRING: {
            // String literal
            get();
            auto string_node = std::make_unique<AST_Node_string>();
            string_node->value = token.GetLexeme();
            
            auto exp = std::make_unique<AST_Node_exp>();
            exp->expr_type = ExprType::ExprString;
            exp->info = std::move(string_node);
            return exp;
        }
        
        case TokenType::OPEN_PARENTHESES: {
            // Parenthesized expression or cast
            get(); // get '('
            
            // Look ahead to see if this is a cast or parenthesized expression
            // If the next tokens form a type, it's a cast
            if (isTypeSpecifier(peek().GetType())) {
                // Parse cast: (type)expression
                auto target_type = parseType();
                expect(TokenType::CLOSE_PARENTHESES, peek().GetType());
                get(); // get ')'
                
                auto expr = parseExpression(GetUnaryPrecedence(TokenType::ASTERISK)); // Cast has high precedence
                
                auto cast = std::make_unique<AST_Node_cast>();
                cast->target_type = std::move(target_type);
                cast->expr = std::move(expr);
                
                auto exp = std::make_unique<AST_Node_exp>();
                exp->expr_type = ExprType::ExprCast;
                exp->info = std::move(cast);
                return exp;
            } else {
                // Parenthesized expression
                auto expr = parseExpression(0);
                expect(TokenType::CLOSE_PARENTHESES, peek().GetType());
                get(); // get ')'
                return expr;
            }
        }
        
        default: {
            // Error: unexpected token
            success = 0;
            errors.emplace_back(token.GetLineNumber(), token.GetColumnNumber(), 
                              TokenType::IDENTIFIER, token.GetType());
            return nullptr;
        }
    }
}

/// Parse compound statement (block)
std::unique_ptr<AST_Node_block> Parser::parseCompoundStatement() {
    auto block = std::make_unique<AST_Node_block>();
    
    if (peek().GetType() != TokenType::OPEN_BRACE) {
        return nullptr;
    }
    get(); // consume '{'
    
    // Process all tokens within the braces sequentially (in reversed token order)
    // We need to track brace depth to find the correct matching closing brace
    int brace_depth = 1; // We've consumed the opening brace
    while (brace_depth > 0 && peek().GetType() != TokenType::END_OF_FILE) {
        
        // Track nested braces
        if (peek().GetType() == TokenType::OPEN_BRACE) {
            brace_depth++;
            get(); // consume nested '{'
            continue;
        } else if (peek().GetType() == TokenType::CLOSE_BRACE) {
            brace_depth--;
            if (brace_depth == 0) {
                break; // Found our matching closing brace
            }
            get(); // consume nested '}'
            continue;
        }
        
        auto block_item = std::make_unique<AST_Node_block_item>();
        
        // Try to parse as variable declaration statement (avoid recursion)
        if (isTypeSpecifier(peek().GetType())) {
            // Parse simple variable declaration directly
            auto decl = std::make_unique<AST_Node_declaration>();
            decl->decl_type = DeclType::DeclVariable;
            
            auto var_decl = std::make_unique<AST_Node_variable_declaration>();
            
            // Parse type
            auto type = std::make_unique<AST_Node_type>();
            switch (peek().GetType()) {
                case TokenType::INT:
                    get(); // consume 'int'
                    type->type = TypeType::TypeInt;
                    break;
                case TokenType::CHAR:
                    get(); // consume 'char'
                    type->type = TypeType::TypeChar;
                    break;
                case TokenType::DOUBLE:
                    get(); // consume 'double'
                    type->type = TypeType::TypeDouble;
                    break;
                default:
                    get(); // consume token
                    type->type = TypeType::TypeInt; // default
                    break;
            }
            var_decl->var_type = std::move(type);
            
            // Parse identifier
            if (peek().GetType() == TokenType::IDENTIFIER) {
                var_decl->identifier = get().GetLexeme();
                
                // Parse initializer if present
                if (peek().GetType() == TokenType::ASSIGNMENT) {
                    get(); // consume '='
                    auto init = std::make_unique<AST_Node_initializer>();
                    init->init_type = InitializerExp;
                    init->info = parseExpression(0);
                    var_decl->initializer = std::move(init);
                }
                
                // Consume semicolon
                if (peek().GetType() == TokenType::SEMICOLON) {
                    get(); // consume ';'
                }
                
                decl->info = std::move(var_decl);
                block_item->item_type = BlockItemDeclaration;
                block_item->info = std::move(decl);
                block->items.push_back(std::move(block_item));
                continue;
            }
        }
        
        // Parse as statement
        auto stmt = parseStatement();
        if (stmt) {
            block_item->item_type = BlockItemStatement;
            block_item->info = std::move(stmt);
            block->items.push_back(std::move(block_item));
        } else {
            // Skip unrecognized token to avoid infinite loop
            get();
        }
    }
    
    // At this point, brace_depth should be 0 and we should be at the matching closing brace
    if (peek().GetType() == TokenType::CLOSE_BRACE && brace_depth == 0) {
        get(); // consume the matching '}'
    }
    return block;
}

/// Parse statements
AST_Node_statementPtr Parser::parseStatement() {
    auto stmt = std::make_unique<AST_Node_statement>();
    
    Token token = peek();
    switch (token.GetType()) {
        case TokenType::IF:
            stmt->stmt_type = StmtType::StmtIf;
            stmt->info = parseIfStatement();
            break;
        case TokenType::WHILE:
            stmt->stmt_type = StmtType::StmtWhile;
            stmt->info = parseWhileStatement();
            break;
        case TokenType::DO:
            stmt->stmt_type = StmtType::StmtDoWhile;
            stmt->info = parseDoWhileStatement();
            break;
        case TokenType::FOR:
            stmt->stmt_type = StmtType::StmtFor;
            stmt->info = parseForStatement();
            break;
        case TokenType::RETURN:
            stmt->stmt_type = StmtType::StmtReturn;
            stmt->info = parseReturnStatement();
            break;
        case TokenType::BREAK:
            stmt->stmt_type = StmtType::StmtBreak;
            get(); // consume 'break'
            if (peek().GetType() == TokenType::SEMICOLON) {
                get();
            }
            break;
        case TokenType::CONTINUE:
            stmt->stmt_type = StmtType::StmtContinue;
            get(); // consume 'continue'
            if (peek().GetType() == TokenType::SEMICOLON) {
                get();
            }
            break;
        case TokenType::SWITCH:
            stmt->stmt_type = StmtType::StmtSwitch;
            stmt->info = parseSwitchStatement();
            break;
        case TokenType::GOTO:
            stmt->stmt_type = StmtType::StmtGoto;
            get(); // consume 'goto'
            if (peek().GetType() == TokenType::IDENTIFIER) {
                stmt->info = get().GetLexeme(); // store label name
            }
            if (peek().GetType() == TokenType::SEMICOLON) {
                get();
            }
            break;
        case TokenType::OPEN_BRACE:
            stmt->stmt_type = StmtType::StmtBlock;
            stmt->info = parseBlock();
            break;
        case TokenType::SEMICOLON:
            // Empty statement
            stmt->stmt_type = StmtType::StmtNull;
            get();
            break;
        default:
            // Check if it's a labeled statement
            if (token.GetType() == TokenType::IDENTIFIER) {
                size_t saved_index = currentIndex;
                get(); // consume identifier
                if (peek().GetType() == TokenType::COLON) {
                    // Labeled statement
                    currentIndex = saved_index; // restore
                    stmt->stmt_type = StmtType::StmtLabeled;
                    stmt->info = parseLabeledStatement();
                } else {
                    // Expression statement
                    currentIndex = saved_index; // restore
                    stmt->stmt_type = StmtType::StmtExpression;
                    stmt->info = parseExpressionStatement();
                }
            } else {
                // Expression statement
                stmt->stmt_type = StmtType::StmtExpression;
                stmt->info = parseExpressionStatement();
            }
            break;
    }
    
    return stmt;
}

/// Parse if statements
AST_Node_if_statementPtr Parser::parseIfStatement() {
    auto if_stmt = std::make_unique<AST_Node_if_statement>();
    
    // Consume 'if'
    if (peek().GetType() == TokenType::IF) {
        get();
    }
    
    // Parse condition
    if (peek().GetType() == TokenType::OPEN_PARENTHESES) {
        get(); // consume '('
        if_stmt->condition = parseExpression();
        expectToken(TokenType::CLOSE_PARENTHESES); // Required closing parenthesis with error recovery
    }
    
    // Parse then statement
    if_stmt->true_statement = parseStatement();
    
    // Parse optional else statement
    if (peek().GetType() == TokenType::ELSE) {
        get(); // consume 'else'
        if_stmt->false_statement = parseStatement();
    }
    
    return if_stmt;
}

/// Parse while statements
AST_Node_while_statementPtr Parser::parseWhileStatement() {
    auto while_stmt = std::make_unique<AST_Node_while_statement>();
    
    // Consume 'while'
    if (peek().GetType() == TokenType::WHILE) {
        get();
    }
    
    // Parse condition
    if (peek().GetType() == TokenType::OPEN_PARENTHESES) {
        get(); // consume '('
        while_stmt->condition = parseExpression();
        if (peek().GetType() == TokenType::CLOSE_PARENTHESES) {
            get(); // consume ')'
        }
    }
    
    // Parse body
    while_stmt->body = parseStatement();
    
    return while_stmt;
}

/// Parse do-while statements
AST_Node_do_while_statementPtr Parser::parseDoWhileStatement() {
    auto do_while_stmt = std::make_unique<AST_Node_do_while_statement>();
    
    // Consume 'do'
    if (peek().GetType() == TokenType::DO) {
        get();
    }
    
    // Parse body
    do_while_stmt->body = parseStatement();
    
    // Consume 'while'
    if (peek().GetType() == TokenType::WHILE) {
        get();
    }
    
    // Parse condition
    if (peek().GetType() == TokenType::OPEN_PARENTHESES) {
        get(); // consume '('
        do_while_stmt->condition = parseExpression();
        if (peek().GetType() == TokenType::CLOSE_PARENTHESES) {
            get(); // consume ')'
        }
    }
    
    // Expect semicolon
    if (peek().GetType() == TokenType::SEMICOLON) {
        get();
    }
    
    return do_while_stmt;
}

/// Parse for statements
AST_Node_for_statementPtr Parser::parseForStatement() {
    auto for_stmt = std::make_unique<AST_Node_for_statement>();
    
    // Consume 'for'
    if (peek().GetType() == TokenType::FOR) {
        get();
    }
    
    // Parse for header
    if (peek().GetType() == TokenType::OPEN_PARENTHESES) {
        get(); // consume '('
        
        // Parse init
        for_stmt->init = parseForInit();
        
        // Parse condition
        if (peek().GetType() != TokenType::SEMICOLON) {
            auto cond_stmt = std::make_unique<AST_Node_expression_statement>();
            cond_stmt->expr = parseExpression();
            for_stmt->condition = std::move(cond_stmt);
        }
        if (peek().GetType() == TokenType::SEMICOLON) {
            get();
        }
        
        // Parse increment
        if (peek().GetType() != TokenType::CLOSE_PARENTHESES) {
            auto incr_stmt = std::make_unique<AST_Node_expression_statement>();
            incr_stmt->expr = parseExpression();
            for_stmt->increment = std::move(incr_stmt);
        }
        
        if (peek().GetType() == TokenType::CLOSE_PARENTHESES) {
            get(); // consume ')'
        }
    }
    
    // Parse body
    for_stmt->body = parseStatement();
    
    return for_stmt;
}

/// Parse for init (can be declaration or expression)
AST_Node_for_initPtr Parser::parseForInit() {
    auto for_init = std::make_unique<AST_Node_for_init>();
    
    // Check if it's a declaration or expression
    if (isTypeSpecifier(peek().GetType())) {
        // Variable declaration
        for_init->init_type = InitDeclaration;
        auto var_decl = parseVariableDeclaration();
        // Create declaration node with variable declaration
        auto decl = std::make_unique<AST_Node_declaration>();
        decl->decl_type = DeclVariable;
        decl->info = std::move(var_decl);
        for_init->info = std::move(decl);
    } else {
        // Expression or empty
        for_init->init_type = InitExpression;
        if (peek().GetType() != TokenType::SEMICOLON) {
            auto expr_stmt = std::make_unique<AST_Node_expression_statement>();
            expr_stmt->expr = parseExpression();
            for_init->info = std::move(expr_stmt);
        }
        if (peek().GetType() == TokenType::SEMICOLON) {
            get();
        }
    }
    
    return for_init;
}

/// Parse return statements
AST_Node_return_statementPtr Parser::parseReturnStatement() {
    auto return_stmt = std::make_unique<AST_Node_return_statement>();
    
    // Consume 'return'
    if (peek().GetType() == TokenType::RETURN) {
        get();
    }
    
    // Parse optional return value
    if (peek().GetType() != TokenType::SEMICOLON) {
        return_stmt->expr = parseExpression(0);
    }
    
    // Expect semicolon
    if (peek().GetType() == TokenType::SEMICOLON) {
        get();
    }
    
    return return_stmt;
}

/// Parse expression statements
AST_Node_expression_statementPtr Parser::parseExpressionStatement() {
    auto expr_stmt = std::make_unique<AST_Node_expression_statement>();
    
    // Parse expression
    if (peek().GetType() != TokenType::SEMICOLON) {
        expr_stmt->expr = parseExpression();
    }
    
    // Expect semicolon
    if (peek().GetType() == TokenType::SEMICOLON) {
        get();
    }
    
    return expr_stmt;
}

/// Parse blocks
AST_Node_blockPtr Parser::parseBlock() {
    auto block = std::make_unique<AST_Node_block>();
    
    // Consume '{' with error recovery
    expectToken(TokenType::OPEN_BRACE);
    
    // Parse block items
    while (peek().GetType() != TokenType::CLOSE_BRACE && 
           peek().GetType() != TokenType::END_OF_FILE) {
        size_t start_index = currentIndex;
        auto item = parseBlockItem();
        if (item) {
            block->items.push_back(std::move(item));
        }
        
        // Prevent infinite loop: if no progress was made, skip the current token
        if (currentIndex == start_index) {
            get(); // Skip unrecognized token to avoid infinite loop
        }
    }
    
    // Consume '}' with error recovery
    expectToken(TokenType::CLOSE_BRACE);
    
    return block;
}

/// Parse block items (declarations or statements)
AST_Node_block_itemPtr Parser::parseBlockItem() {
    auto item = std::make_unique<AST_Node_block_item>();
    
    // Check if it's a declaration or statement
    if (isTypeSpecifier(peek().GetType()) || 
        peek().GetType() == TokenType::STATIC ||
        peek().GetType() == TokenType::EXTERN) {
        // Declaration
        item->item_type = BlockItemDeclaration;
        item->info = parseDeclaration();
    } else {
        // Statement
        item->item_type = BlockItemStatement;
        item->info = parseStatement();
    }
    
    return item;
}

/// Parse labeled statements
AST_Node_labeled_statementPtr Parser::parseLabeledStatement() {
    auto labeled_stmt = std::make_unique<AST_Node_labeled_statement>();
    
    // Parse label
    if (peek().GetType() == TokenType::IDENTIFIER) {
        labeled_stmt->label.push_back(get().GetLexeme());
    }
    
    // Consume ':'
    if (peek().GetType() == TokenType::COLON) {
        get();
    }
    
    // Parse statement
    labeled_stmt->statement = parseStatement();
    
    return labeled_stmt;
}

/// Parse switch statements
AST_Node_switch_statementPtr Parser::parseSwitchStatement() {
    auto switch_stmt = std::make_unique<AST_Node_switch_statement>();
    
    // Consume 'switch'
    if (peek().GetType() == TokenType::SWITCH) {
        get();
    }
    
    // Parse condition
    if (peek().GetType() == TokenType::OPEN_PARENTHESES) {
        get(); // consume '('
        switch_stmt->selector = parseExpression();
        if (peek().GetType() == TokenType::CLOSE_PARENTHESES) {
            get(); // consume ')'
        }
    }
    
    // Parse body (should be a block with case/default labels)
    switch_stmt->body = parseStatement();
    
    return switch_stmt;
}

/// Parse array size
AST_Node_array_sizePtr Parser::parseArraySize() {
    auto array_size = std::make_unique<AST_Node_array_size>();
    
    if (peek().GetType() == TokenType::CLOSE_BRACKET) {
        // Empty brackets []
        array_size->size_type = ArraySizeType::ArraySizeUnspecified;
    } else {
        // Has size expression
        auto size_expr = parseExpression();
        if (size_expr) {
            if (size_expr->expr_type == ExprType::ExprConstant) {
                // Constant size
                array_size->size_type = ArraySizeType::ArraySizeConst;
                if (std::holds_alternative<std::unique_ptr<AST_Node_const>>(size_expr->info)) {
                    auto& const_node = std::get<std::unique_ptr<AST_Node_const>>(size_expr->info);
                    array_size->info = std::move(const_node);
                }
            } else {
                // Variable length array
                array_size->size_type = ArraySizeType::ArraySizeVar;
                array_size->info = std::move(size_expr);
            }
        }
    }
    
    return array_size;
}

/// Parse program (top-level)
AST_Node_programPtr Parser::parseProgram() {
    auto program = std::make_unique<AST_Node_program>();
    
    int iteration = 0;
    while (currentIndex < tokens.size() && peek().GetType() != TokenType::END_OF_FILE) {
        iteration++;
        size_t start_index = currentIndex;
        
        auto decl = parseDeclaration();
        if (decl) {
            program->declarations.push_back(std::move(decl));
        } else {
            // Skip unrecognized token to avoid infinite loop
            if (currentIndex >= 0) {
                get();
            }
            if (currentIndex < 0) break;
        }
        
        // Safety check for infinite loop
        if (iteration > 50) {
            break;
        }
    }
    
    // Parsing completed
    return program;
}

/// Parse type specifiers
AST_Node_typePtr Parser::parseType() {
    auto type = std::make_unique<AST_Node_type>();
    
    Token token = peek();
    switch (token.GetType()) {
        case TokenType::CHAR:
            get();
            type->type = TypeType::TypeChar;
            break;
        case TokenType::INT:
            get();
            type->type = TypeType::TypeInt;
            break;
        case TokenType::LONG:
            get();
            type->type = TypeType::TypeLong;
            break;
        case TokenType::DOUBLE:
            get();
            type->type = TypeType::TypeDouble;
            break;
        case TokenType::VOID:
            get();
            type->type = TypeType::TypeVoid;
            break;
        case TokenType::STRUCT:
            get();
            type->type = TypeType::TypeStructure;
            if (peek().GetType() == TokenType::IDENTIFIER) {
                type->info = get().GetLexeme();
            }
            break;
        case TokenType::UNION:
            get();
            type->type = TypeType::TypeUnion;
            if (peek().GetType() == TokenType::IDENTIFIER) {
                type->info = get().GetLexeme();
            }
            break;
        case TokenType::ENUM:
            get();
            type->type = TypeType::TypeEnum;
            if (peek().GetType() == TokenType::IDENTIFIER) {
                type->info = get().GetLexeme();
            }
            break;
        case TokenType::IDENTIFIER:
            type->type = TypeType::TypeNamedtype;
            type->info = get().GetLexeme();
            break;
        default:
            return nullptr;
    }
    return type;
}

/// Parse variable declarations
AST_Node_variable_declarationPtr Parser::parseVariableDeclaration() {
    auto var_decl = std::make_unique<AST_Node_variable_declaration>();
    
    var_decl->var_type = parseType();
    if (!var_decl->var_type) {
        return nullptr;
    }
    
    if (peek().GetType() == TokenType::IDENTIFIER) {
        var_decl->identifier = get().GetLexeme();
    } else {
        return nullptr;
    }
    
    if (peek().GetType() == TokenType::EQUAL) {
        get();
        var_decl->initializer = parseInitializer();
    }
    
    expectToken(TokenType::SEMICOLON); // Required semicolon with error recovery
    
    return var_decl;
}

/// Parse function declarations
AST_Node_function_declarationPtr Parser::parseFunctionDeclaration() {
    auto func_decl = std::make_unique<AST_Node_function_declaration>();
    
    func_decl->fun_type = parseType();
    if (!func_decl->fun_type) {
        return nullptr;
    }
    
    if (peek().GetType() == TokenType::IDENTIFIER) {
        func_decl->identifier = get().GetLexeme();
    } else {
        return nullptr;
    }
    
    if (peek().GetType() == TokenType::OPEN_PARENTHESES) {
        get();
        while (peek().GetType() != TokenType::CLOSE_PARENTHESES &&
               peek().GetType() != TokenType::END_OF_FILE) {
            if (peek().GetType() == TokenType::IDENTIFIER) {
                func_decl->params.push_back(get().GetLexeme());
            }
            if (peek().GetType() == TokenType::COMMA) {
                get();
            } else {
                break;
            }
        }
        expectToken(TokenType::CLOSE_PARENTHESES); // Required closing parenthesis with error recovery
    }
    
    if (peek().GetType() == TokenType::OPEN_BRACE) {
        func_decl->body = parseBlock();
    } else if (peek().GetType() == TokenType::SEMICOLON) {
        get();
    }
    
    return func_decl;
}

/// Parse union declarations
AST_Node_union_declarationPtr Parser::parseUnionDeclaration() {
    auto union_decl = std::make_unique<AST_Node_union_declaration>();
    
    if (peek().GetType() == TokenType::UNION) {
        get();
    }
    
    if (peek().GetType() == TokenType::IDENTIFIER) {
        union_decl->tag = get().GetLexeme();
    }
    
    if (peek().GetType() == TokenType::OPEN_BRACE) {
        get();
        while (peek().GetType() != TokenType::CLOSE_BRACE &&
               peek().GetType() != TokenType::END_OF_FILE) {
            auto member = parseMemberDeclaration();
            if (member) {
                union_decl->members.push_back(std::move(member));
            }
        }
        if (peek().GetType() == TokenType::CLOSE_BRACE) {
            get();
        }
    }
    
    if (peek().GetType() == TokenType::SEMICOLON) {
        get();
    }
    
    return union_decl;
}

/// Parse enum declarations
AST_Node_enum_declarationPtr Parser::parseEnumDeclaration() {
    auto enum_decl = std::make_unique<AST_Node_enum_declaration>();
    
    if (peek().GetType() == TokenType::ENUM) {
        get();
    }
    
    if (peek().GetType() == TokenType::IDENTIFIER) {
        enum_decl->tag = get().GetLexeme();
    }
    
    if (peek().GetType() == TokenType::OPEN_BRACE) {
        get();
        while (peek().GetType() != TokenType::CLOSE_BRACE &&
               peek().GetType() != TokenType::END_OF_FILE) {
            auto enumerator = parseEnumerator();
            if (enumerator) {
                enum_decl->enumerators.push_back(std::move(enumerator));
            }
            if (peek().GetType() == TokenType::COMMA) {
                get();
            } else {
                break;
            }
        }
        if (peek().GetType() == TokenType::CLOSE_BRACE) {
            get();
        }
    }
    
    if (peek().GetType() == TokenType::SEMICOLON) {
        get();
    }
    
    return enum_decl;
}

/// Parse typedef declarations
AST_Node_typedef_declarationPtr Parser::parseTypedefDeclaration() {
    auto typedef_decl = std::make_unique<AST_Node_typedef_declaration>();
    
    if (peek().GetType() == TokenType::TYPEDEF) {
        get();
    }
    
    // For now, create a simple typedef entry
    auto entry = std::make_unique<AST_Node_typedef_entry>();
    entry->alias_type = parseType();
    if (!entry->alias_type) {
        return nullptr;
    }
    
    if (peek().GetType() == TokenType::IDENTIFIER) {
        entry->name = get().GetLexeme();
    } else {
        return nullptr;
    }
    
    typedef_decl->entries.push_back(std::move(entry));
    
    if (peek().GetType() == TokenType::SEMICOLON) {
        get();
    }
    
    return typedef_decl;
}

/// Parse class declarations
AST_Node_class_declarationPtr Parser::parseClassDeclaration() {
    auto class_decl = std::make_unique<AST_Node_class_declaration>();
    
    if (peek().GetType() == TokenType::CLASS) {
        get();
    }
    
    if (peek().GetType() == TokenType::IDENTIFIER) {
        class_decl->name = get().GetLexeme();
    } else {
        return nullptr;
    }
    
    if (peek().GetType() == TokenType::OPEN_BRACE) {
        get();
        while (peek().GetType() != TokenType::CLOSE_BRACE &&
               peek().GetType() != TokenType::END_OF_FILE) {
            auto member = parseClassMember();
            if (member) {
                class_decl->members.push_back(std::move(member));
            }
        }
        if (peek().GetType() == TokenType::CLOSE_BRACE) {
            get();
        }
    }
    
    if (peek().GetType() == TokenType::SEMICOLON) {
        get();
    }
    
    return class_decl;
}

/// Parse initializers
AST_Node_initializerPtr Parser::parseInitializer() {
    auto initializer = std::make_unique<AST_Node_initializer>();
    
    // For now, just parse a simple expression initializer
    auto expr = parseExpression(0);
    if (expr) {
        initializer->init_type = InitializerExp;
        initializer->info = std::move(expr);
        return initializer;
    }
    
    return nullptr;
}

/// Parse enumerators
AST_Node_enumeratorPtr Parser::parseEnumerator() {
    auto enumerator = std::make_unique<AST_Node_enumerator>();
    
    if (peek().GetType() == TokenType::IDENTIFIER) {
        enumerator->name = get().GetLexeme();
    } else {
        return nullptr;
    }
    
    // Check for optional value assignment
    if (peek().GetType() == TokenType::EQUAL) {
        get();
        // Parse constant expression for enum value
        Token token = peek();
        if (token.GetType() == TokenType::INT_CONSTANT) {
            auto const_node = std::make_unique<AST_Node_const>();
            const_node->type = ConstType::ConstInt;
            const_node->value.intValue = std::stoi(get().GetLexeme());
            enumerator->value = std::move(const_node);
        }
    }
    
    return enumerator;
}

/// Parse class members
AST_Node_class_memberPtr Parser::parseClassMember() {
    auto member = std::make_unique<AST_Node_class_member>();
    
    // For now, just handle field members (variable declarations)
    auto var_decl = parseVariableDeclaration();
    if (var_decl) {
        auto field_member = std::make_unique<AST_Node_field_member>();
        field_member->var = std::move(var_decl);
        field_member->access = Public; // Default access
        
        member->member_type = ClassMemberField;
        member->info = std::move(field_member);
        return member;
    }
    
    return nullptr;
}
