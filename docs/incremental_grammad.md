<program> ::= <function>

<function> ::= "int" <identifier> "(" "void" ")" <block>

<block> ::= "{" { <block-item> } "}"

<block-item> ::= <statement> | <declaration>
<declaration> ::= "int" <identifier> [ "=" <exp> ] ";"

<for-init> ::= <declaration> | [ <exp> ] ";"



<statement> ::= "return" <exp> ";" 
| <exp> ";" 
| "if" "(" <exp> ")" <statement> [ "else" <statement> ] 
| <block> 
| ";" 
| "goto:" <identifier> 
| <identifier> ":" 
| "break" ";"
| "continue" ";"
| "while" "(" <exp> ")" <statement>
| "do" <statement> "while" "(" <exp> ")" ";"
| "for" "(" <for-init> [ <exp> ] ";" [ <exp> ] ")" <statement>

<exp> ::= <factor> | <exp> <binop> <exp> | <exp> "?" <exp> ":" <exp>

<factor> ::= <int> | <identifier> |<unop> <factor> | "(" <exp> ")" // unary operator handled in this rule

<unop> ::= "-" | "~"

<binop> ::= "-" | "+" | "*" | "/" | "%" | "&" | "|" | "^" | "<<" | ">>" | "&&" | "||"
| "==" | "!=" | "<" | "<=" | ">" | ">="

<identifier> ::= ? An identifier token ?

<int> ::= ? A constant token ?



Reminders:
parse_constant(token):
v = integer value of token
if v > 2^63 - 1:
fail("Constant is too large to represent as an int or long")
if token is an int token and v <= 2^31 - 1:
return ConstInt(v)
return ConstLong(v)
