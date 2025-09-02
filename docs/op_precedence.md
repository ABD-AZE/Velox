# Operator Precedence for Pratt Parser

This document defines the precedence levels and binding powers for all operators in the Velox C compiler's Pratt parser implementation.

## Pratt Parser Concepts

- **Precedence**: Higher numbers indicate higher precedence (tighter binding)
- **Left Binding Power (LBP)**: Controls how tightly an operator binds to its left operand
- **Right Binding Power (RBP)**: Controls how tightly an operator binds to its right operand
- **Associativity**: 
  - Left-associative: LBP = RBP + 1
  - Right-associative: LBP = RBP - 1
  - Non-associative: LBP = RBP

## Operator Precedence Table

| Precedence | Operator | Token | Associativity | LBP | RBP | Description |
|------------|----------|-------|---------------|-----|-----|-------------|
| 16 | `()` `[]` `->` `.` | OPEN_PARANTHESES, OPEN_BRACKET, ARROW_OPERATOR, DOT | Left | 159 | 160 | Postfix, function call, array subscript, member access |
| 15 | `++` `--` (postfix) | INCREMENT_OPERATOR, DECREMENT_OPERATOR | Left | 149 | 150 | Postfix increment/decrement |
| 14 | `++` `--` (prefix) | INCREMENT_OPERATOR, DECREMENT_OPERATOR | Right | 140 | 139 | Prefix increment/decrement |
| 14 | `+` `-` (unary) | PLUS, HYPHEN | Right | 140 | 139 | Unary plus/minus |
| 14 | `!` `~` | NOT, TILDE | Right | 140 | 139 | Logical NOT, bitwise complement |
| 14 | `*` (dereference) | ASTERISK | Right | 140 | 139 | Pointer dereference |
| 14 | `&` (address-of) | AAND | Right | 140 | 139 | Address-of operator |
| 14 | `sizeof` | SIZEOF | Right | 140 | 139 | Size of operator |
| 13 | `*` `/` `%` | ASTERISK, FORWARD_SLASH, PERCENT_SIGN | Left | 129 | 130 | Multiplication, division, modulo |
| 12 | `+` `-` (binary) | PLUS, HYPHEN | Left | 119 | 120 | Addition, subtraction |
| 11 | `<<` `>>` | LEFT_SHIFT, RIGHT_SHIFT | Left | 109 | 110 | Bitwise shift operators |
| 10 | `<` `<=` `>` `>=` | LESSTHAN, LESSTHANEQUAL, GREATERTHAN, GREATERTHANEQUAL | Left | 99 | 100 | Relational operators |
| 9 | `==` `!=` | EQUAL, NOTEQUAL | Left | 89 | 90 | Equality operators |
| 8 | `&` (bitwise) | AAND | Left | 79 | 80 | Bitwise AND |
| 7 | `^` | XOR | Left | 69 | 70 | Bitwise XOR |
| 6 | `\|` | AOR | Left | 59 | 60 | Bitwise OR |
| 5 | `&&` | LAND | Left | 49 | 50 | Logical AND |
| 4 | `\|\|` | LOR | Left | 39 | 40 | Logical OR |
| 3 | `?:` | QUESTION_MARK, COLON | Right | 30 | 29 | Conditional (ternary) operator |
| 2 | `=` | ASSIGNMENT | Right | 20 | 19 | Simple assignment |
| 2 | `+=` `-=` `*=` `/=` `%=` | COMPOUND_SUM, COMPOUND_DIFFERENCE, COMPOUND_PRODUCT, COMPOUND_DIVISION, COMPOUND_REMAINDER | Right | 20 | 19 | Compound assignment |
| 2 | `&=` `^=` `\|=` | COMPOUND_AND, COMPOUND_XOR, COMPOUND_OR | Right | 20 | 19 | Bitwise compound assignment |
| 2 | `<<=` `>>=` | COMPOUND_LEFTSHIFT, COMPOUND_RIGHTSHIFT | Right | 20 | 19 | Shift compound assignment |
| 1 | `,` | COMMA | Left | 9 | 10 | Comma operator |

## Special Cases

### Function Calls and Array Access
- Function calls `f()` and array access `a[i]` are handled as postfix operators
- The opening parenthesis or bracket acts as the operator token
- These have the highest precedence (16)

### Unary vs Binary Operators
Several tokens can represent both unary and binary operators:
- `PLUS`, `HYPHEN`: Unary (precedence 14) vs Binary (precedence 12)
- `ASTERISK`: Dereference (precedence 14) vs Multiplication (precedence 13)
- `AAND`: Address-of (precedence 14) vs Bitwise AND (precedence 8)

The parser context determines which interpretation to use.

### Ternary Operator
The conditional operator `?:` is special:
- `?` has precedence 3 with right associativity
- `:` is handled as part of the ternary expression parsing logic
- Right associativity allows chaining: `a ? b : c ? d : e`

### Assignment Operators
All assignment operators have the same precedence (2) and are right-associative:
- This allows chaining: `a = b = c`
- Compound assignments follow the same pattern

## Implementation Notes

1. **Precedence 0**: Reserved for end-of-expression or lowest precedence contexts
2. **Even/Odd Pattern**: 
   - Even numbers for LBP (left-associative operators)
   - Odd numbers for RBP to ensure proper precedence relationships
3. **Context Sensitivity**: The parser must distinguish between unary and binary uses of ambiguous operators based on parsing context
4. **Error Recovery**: Lower precedence values can be used for error recovery and synchronization

## Usage in Pratt Parser

```cpp
// Example precedence lookup
int getPrecedence(TokenType token) {
    switch (token) {
        case ASTERISK: return 130; // multiplication (binary context)
        case PLUS: return 120;     // addition (binary context)
        case ASSIGNMENT: return 20;
        case COMMA: return 10;
        default: return 0;
    }
}

// Example binding power calculation
std::pair<int, int> getBindingPower(TokenType token) {
    switch (token) {
        case ASTERISK: return {130, 129}; // left-associative
        case ASSIGNMENT: return {19, 20}; // right-associative
        case COMMA: return {10, 9};       // left-associative
        default: return {0, 0};
    }
}
```

This precedence table ensures correct parsing of C expressions according to the C language specification.