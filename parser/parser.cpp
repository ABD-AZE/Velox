#include "parser.hpp"
#include <assert.h>

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

Parser::Parser(const std::vector<Token> &inputtokens) : tokens(inputtokens), program(std::make_unique<AST_Node_program>())
{
    Token eof;
    eof.SetType(END_OF_FILE);
    tokens.push_back(eof); // Add EOF token at the end
    std::reverse(tokens.begin(), tokens.end());
    tokenSize = tokens.size();
    currentIndex = tokenSize - 1; // Start from the last token (which is the first after reversing)
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
        // handle error here
    }
    return true;
}

void Parser::parseProgram()
{
    while (currentIndex < tokenSize)
    {
        program->AddDeclaration(std::move(parseDeclaration()));
    }
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
    auto declaration = std::make_unique<AST_Node_declaration>();
    Token token = peek();

    // directly return from within switch
    switch (token.GetType())
    {
    case TokenType::STRUCT:
        {
            std::get<AST_Node_struct_declarationPtr>(declaration->info) = std::move(parseStructDeclaration());
        }
        break;
    case TokenType::CLASS:
        {
            std::get<AST_Node_class_declarationPtr>(declaration->info) = std::move(parseClassDeclaration());
        }
        break;
    case TokenType::ENUM:
        {
            std::get<AST_Node_enum_declarationPtr>(declaration->info) = std::move(parseEnumDeclaration());
        }
        break;
    case TokenType::TYPEDEF:
        // Parse typedef declaration
        {
            std::get<AST_Node_typedef_declarationPtr>(declaration->info) = std::move(parseTypedefDeclaration());
        }
        break;
    case TokenType::UNION:
        // Parse union declaration
        {
            std::get<AST_Node_union_declarationPtr>(declaration->info) = std::move(parseUnionDeclaration());
        }
        break;
    default:
        // Assume variable or function declaration
        if (!isTypeSpecifier(token.GetType())){
            success = 0;
            errors.emplace_back(token.GetLineNumber(), token.GetColumnNumber(), TokenType::IDENTIFIER, token.GetType());
            // errors.emplace_back(std::string("Expected type specifier found '") + token.GetLexeme() + "' at " + std::to_string(token.GetLineNumber()) + ":" + std::to_string(token.GetColumnNumber()));
        }
        while (isTypeSpecifier(token.GetType()))
        {
            token = get();
        }
        expect(TokenType::IDENTIFIER, token.GetType());
        if (peek().GetType() == OPEN_PARENTHESES)
        {
            // Function declaration
            declaration->decl_type = DeclType::DeclFunction;
            // reset tokens to initial pos
            reset();
            std::get<AST_Node_function_declarationPtr>(declaration->info) = std::move(parseFunctionDeclaration());
        }
        else
        {
            // Variable declaration
            declaration->decl_type = DeclType::DeclVariable;
            // reset tokens to initial pos
            reset();
            std::get<AST_Node_variable_declarationPtr>(declaration->info) = std::move(parseVariableDeclaration());
        }
        break;
    } 
    return declaration;
}

AST_Node_struct_declarationPtr Parser::parseStructDeclaration()
{
    auto structDecl = std::make_unique<AST_Node_struct_declaration>();
    Token token = consume();
    expect(TokenType::STRUCT, token.GetType());
    token = consume();
    expect(TokenType::IDENTIFIER, token.GetType());
    structDecl->tag = token.GetLexeme();
    if (peek().GetType() == TokenType::OPEN_BRACE)
    {
        consume(); // consume {
        while (peek().GetType() != TokenType::CLOSE_BRACE && peek().GetType() != TokenType::END_OF_FILE)
        {
            if(!isTypeSpecifier(peek().GetType()))
            {
                success = 0;
                errors.emplace_back(peek().GetLineNumber(), peek().GetColumnNumber(), TokenType::IDENTIFIER, peek().GetType());
            }
            if (isTypeSpecifier(peek().GetType()))
            {
                structDecl->members.push_back(parseMemberDeclaration());
            }
        }
        expect(TokenType::CLOSE_BRACE, consume().GetType());
        expect(TokenType::SEMICOLON, consume().GetType());
    }
    else{
        expect(TokenType::SEMICOLON, consume().GetType());

    }
    return structDecl;
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
        
        // If the left binding power is less than minimum, we're done
        if (left_bp < min_bp) {
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
                auto right = parseExpression(0);
                expect(TokenType::CLOSE_BRACKET, peek().GetType());
                get(); // get ']'
                
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
                expect(TokenType::IDENTIFIER, peek().GetType());
                Token member_token = get();
                
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
