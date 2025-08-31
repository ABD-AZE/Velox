# Velox C Compiler Grammar

This document defines the grammar rules for the Velox C compiler using Extended Backus-Naur Form (EBNF) notation.

## Program Structure

### `<program>`
```ebnf
<program> ::= { <declaration> }
```
**Example:** `int x; void main() { return 0; }`

### `<declaration>`
```ebnf
<declaration> ::=   <variable-declaration> 
                    | <function-declaration>  
                    | <struct-declaration>  
                    | <typedef-declaration> 
                    | <enum-declaration> 
                    | <union-declaration>
                    | <class-declaration>
```
**Examples:**
- Variable declaration: `int x = 5;`
- Function declaration: `int add(int a, int b);`

## Declarations

### `<variable-declaration>`
```ebnf
<variable-declaration> ::= { <specifier> }+ <init-declarator-list> ";"

<init-declarator-list> ::= <init-declarator> { "," <init-declarator> }

<init-declarator> ::= <declarator> [ "=" <initializer> ]

```
**Examples:**
- `int x;`
- `static int count = 0;`
- `unsigned long *ptr = NULL;`

### `<function-declaration>`
```ebnf
<function-declaration> ::= { <specifier> }+ <declarator> ( <block> | ";")
```
**Examples:**
- Function prototype: `int factorial(int n);`
- Function definition: `int factorial(int n) { return n <= 1 ? 1 : n * factorial(n-1); }`

### `<struct-declaration>`
```ebnf
<struct-declaration> ::= "struct" <identifier> ";"
                       | "struct" <identifier> "{" { <member-declaration> } "}" ";"
```

### `enum-declaration`
```ebnf
<enum-declaration> ::= "enum" <identifier> ";"
                     | <enum-specifier> ";"
```

### `union-declaration`
```ebnf
<union-declaration>  ::= "union"  <identifier> ";"
                       | "union"  <identifier> "{" { <member-declaration> } "}" ";"
```

### `typedef-declaration`
```ebnf
<typedef-declaration> ::= "typedef" { <type-specifier> }+ <typedef-declarator-list> ";"

<typename-declarator-list> ::= <typedef-declarator> { "," <typedef-declarator> }

<typename-declarator> ::= <declarator> 
```

### `class-declaration`
```ebnf
<class-declaration> ::= "class" <identifier> [ ":" <base-spec-list> ]
                        "{" { <class-member> } "}" ";"

<base-spec-list> ::= <base-spec> { "," <base-spec> }
<base-spec> ::= [ "public" | "private" ] <identifier> 

<class-member> ::= [ "public" ":" | "private" ":" ]    
                 | <member-declaration>                
                 | <method-declaration>

<method-declaration> ::= { <specifier> }+ <declarator> ( <block> | ";" )
                       | <constructor-declaration>

<constructor-declaration> ::= <identifier> "(" [ <param> { "," <param> } ] ")" ( <block> | ";" )

```

### `<member-declaration>`
```ebnf
<member-declaration> ::= { <type-specifier> }+ <member-declarator-list> ";"

<member-declarator-list> ::= <declarator> { "," <declarator> }
```

### `<declarator>`
```ebnf
<declarator> ::= <ptr-ref-seq> <direct-declarator>
<ptr-ref-seq> ::= { "*" } [ "&" ]
```
**Examples:**
- Direct: `x`
- Pointer: `*ptr`
- Pointer to pointer: `**ptr`

### `<direct-declarator>`
```ebnf
<direct-declarator> ::= <simple-declarator> [ <declarator-suffix> ]
```
**Examples:**
- Simple: `x`
- Function: `func(int a, int b)`
- Array: `arr[10]`

### `<declarator-suffix>`
```ebnf
<declarator-suffix> ::= <param-list> | { "[" [ <exp> ] "]" }+
```
**Examples:**
- Parameter list: `(int x, char y)`
- Array dimensions: `[10][20]`

### `<param-list>`
```ebnf
<param-list> ::=    "(" "void" ")"
                    | "(" <param> { "," <param> } [ "," "..." ] ")"
```
**Examples:**
- Void parameters: `(void)`
- Multiple parameters: `(int x, char *str, double val)`

### `<param>`
```ebnf
<param> ::= { <type-specifier> }+ ( <declarator> | [ <abstract-declarator> ] )
```
**Example:** `int *ptr`

### `<simple-declarator>`
```ebnf
<simple-declarator> ::= <identifier> | "(" <declarator> ")"
```
**Examples:**
- Identifier: `variable`
- Parenthesized: `(*func_ptr)`

## Type System

### `<type-specifier>`
```ebnf
<type-specifier> ::=    "int" | "long" | "unsigned" | "signed" | "double" | "char" 
                        | "void" 
                        | <struct-or-union-specifier> 
                        | <enum-specifier>
                        | "va_list" 
                        | <typedef-name>

// Parser requirement: maintain a typedef-name set (or a TYPEDEF_NAME token) so <typedef-name> is distinguishable from an identifier.

<struct-or-union-specifier> ::= "struct" <identifier>
                              | "struct" "{" { <member-declaration> } "}"
                              | "union"  <identifier>                     
                              | "union"  "{" { <member-declaration> } "}"

<enum-specifier> ::=    "enum" [ <identifier> ] "{" <enumerator-list> "}"
                        | "enum" <identifier>

<enumerator-list> ::= <enumerator> { "," <enumerator> }

<enumerator> ::= <identifier> [ "=" <const> ]
```
**Examples:** `int`, `unsigned long`, `char`, `void`

### `<specifier>`
```ebnf
<specifier> ::= <type-specifier> | "static" | "extern"
```
**Examples:** `static`, `extern int`, `unsigned`

### `<type-name>`
```ebnf
<type-name> ::= { <type-specifier> }+ [ <abstract-declarator> ]
```
**Examples:**
- `int`
- `unsigned long *`
- `char[10]`

### `<abstract-declarator>`
```ebnf
<abstract-declarator> ::= "*" [ <abstract-declarator> ]
                        | <direct-abstract-declarator>
```
**Examples:**
- `*`
- `**`
- `[10]`

### `<direct-abstract-declarator>`
```ebnf
<direct-abstract-declarator> ::= "(" <abstract-declarator> ")" { "[" <const> "]" }
                               | { "[" <const> "]" }+
```
**Examples:**
- `(*)[10]`
- `[5][10]`

## Statements and Blocks

### `<block>`
```ebnf
<block> ::= "{" { <block-item> } "}"
```
**Example:** `{ int x = 5; printf("%d", x); }`

### `<block-item>`
```ebnf
<block-item> ::= <statement> | <declaration>
```
**Examples:**
- Statement: `x = 10;`
- Declaration: `int y = 20;`

### `<statement>`
```ebnf
<statement> ::= "return" [ <exp> ] ";"
              | <exp> ";"
              | "if" "(" <exp> ")" <statement> [ "else" <statement> ]
              | <block>
              | "break" ";"
              | "continue" ";"
              | "while" "(" <exp> ")" <statement>
              | "do" <statement> "while" "(" <exp> ")" ";"
              | "for" "(" <for-init> [ <exp> ] ";" [ <exp> ] ")" <statement>
              | ";" 
              | "switch" "(" <exp> ")" <statement>
              | <labeled-statement>
              | "goto" <identifier> ";"

<labeled-statement> ::= <identifier> ":" <statement>
                      | "case" <exp> ":" <statement>
                      | "default" ":" <statement>

//"case" <exp> --> (* restricted by semantics to integer constant expression *)
```
**Examples:**
- Return: `return 42;`
- Expression: `x = y + 1;`
- If-else: `if (x > 0) return x; else return -x;`
- While loop: `while (i < 10) i++;`
- Do-while: `do { x++; } while (x < 5);`
- For loop: `for (int i = 0; i < 10; i++) sum += i;`
- Empty statement: `;`

### `<for-init>`
```ebnf
<for-init> ::= <variable-declaration> | [ <exp> ] ";"
```
**Examples:**
- Declaration: `int i = 0;`
- Expression: `i = 0;`
- Empty: `;`

## Expressions

### `<exp>`
```ebnf
<exp> ::= <cast-exp> | <exp> <binop> <exp> | <exp> "?" <exp> ":" <exp>
```
**Examples:**
- Simple: `x`
- Binary operation: `a + b * c`
- Conditional: `x > 0 ? x : -x`

### `<cast-exp>`
```ebnf
<cast-exp> ::= "(" <type-name> ")" <cast-exp>
             | <unary-exp>
```
**Examples:**
- Cast: `(int)3.14`
- Unary: `-x`

### `<unary-exp>`
```ebnf
<unary-exp> ::= <unop> <cast-exp>
              | "sizeof" <unary-exp>
              | "sizeof" "(" <type-name> ")"
              | <postfix-exp>
              | "++" <unary-exp> 
              | "--" <unary-exp>
```
**Examples:**
- Unary operator: `-x`, `!flag`, `*ptr`
- Sizeof expression: `sizeof x`
- Sizeof type: `sizeof(int)`
- Postfix: `arr[i]`

### `<postfix-exp>`
```ebnf
<postfix-exp> ::= <postfix-core> { <postfix-suffix> }

<postfix-core> ::= <primary-exp> | <lib-func>

<lib-func> ::= "printf" | "scanf" | "malloc" | "free"
             | "fopen" | "fclose" | "fread" | "fwrite"
             | "fprintf" | "fscanf"

<postfix-suffix> ::= "[" <exp> "]"
                   | "." <identifier>
                   | "->" <identifier>
                   | "(" [ <argument-list> ] ")"
                   | "++"
                   | "--"
```
**Examples:**
- Array access: `arr[0]`
- Multi-dimensional: `matrix[i][j]`
- function calls are handled by the suffix

### `<primary-exp>`
```ebnf
<primary-exp> ::= <const> 
                | <identifier>
                | "(" <exp> ")"
                | { <string> }+
                | "va_start" "(" <exp> "," <identifier> ")"  
                | "va_end"   "(" <exp> ")"                    
                | "va_arg"   "(" <exp> "," <type-name> ")"  
                | <lambda-exp>  
```
**Examples:**
- Constant: `42`, `3.14`, `'a'`
- Identifier: `variable`
- Parenthesized: `(x + y)`
- String: `"Hello World"`
- Lambda: `[](int x) -> int { return x + 1; }`


### `<lambda-exp>`
```ebnf
<lambda-exp> ::= "[" <capture-spec> "]"
                 "(" [ <param> { "," <param> } ] [ "," "..." ] ")"
                 [ "->" <type-name> ]
                 <block>                                         

<capture-spec> ::= "&"                              
                    | "="                              
                    | <capture-list>                   
                    | /* empty */                      

<capture-list> ::= <capture> { "," <capture> }                  

<capture> ::= "&" <identifier> | <identifier>                   
```
**Example:** `int f = [=](int x){ return x + k; }; `

### `<argument-list>`
```ebnf
<argument-list> ::= <exp> { "," <exp> }
```
**Example:** `x + 1, y * 2, "string"`

## Initializers

### `<initializer>`
```ebnf
<initializer> ::= <exp> | "{" <initializer> { "," <initializer> } [ "," ] "}"
```
**Examples:**
- Expression: `42`
- Array initializer: `{1, 2, 3, 4}`
- Nested initializer: `{{1, 2}, {3, 4}}`

## Operators

### `<unop>`
```ebnf
<unop> ::= "-" | "~" | "!" | "*" | "&"
```
**Examples:** `-x`, `~bits`, `!flag`, `*ptr`, `&var`

### `<binop>`
```ebnf
<binop> ::= "-" | "+" | "*" | "/" | "%"
          | "<<" | ">>"
          | "&" | "^" | "|"
          | "&&" | "||"
          | "==" | "!=" | "<" | "<=" | ">" | ">="
          | "=" | "+=" | "-=" | "*=" | "/=" | "%="
          | "<<=" | ">>=" | "&=" | "^=" | "|="
```
**Examples:**
- Arithmetic: `a + b`, `x * y`, `n % 2`
- Logical: `a && b`, `x || y`
- Comparison: `x == y`, `a < b`, `p != NULL`
- Assignment: `x = 5`

## Constants and Tokens

### `<const>`
```ebnf
<const> ::= <int> | <long> | <uint> | <ulong> | <double> | <char>
```
**Examples:** `42`, `123L`, `456U`, `789UL`, `3.14`, `'a'`

### `<identifier>`
```ebnf
<identifier> ::= ? An identifier token ?
```
**Examples:** `variable`, `function_name`, `MAX_SIZE`

### `<string>`
```ebnf
<string> ::= ? A string token ?
```
**Examples:** `"Hello"`, `"World\n"`, `""`

### `<int>`
```ebnf
<int> ::= ? An int token ?
```
**Examples:** `0`, `42`, `0x1A`, `077`

### `<char>`
```ebnf
<char> ::= ? A char token ?
```
**Examples:** `'a'`, `'\n'`, `'\0'`, `'\x41'`

### `<long>`
```ebnf
<long> ::= ? An int or long token ?
```
**Examples:** `123L`, `0x1234L`

### `<uint>`
```ebnf
<uint> ::= ? An unsigned int token ?
```
**Examples:** `123U`, `0xFFFFU`

### `<ulong>`
```ebnf
<ulong> ::= ? An unsigned int or unsigned long token ?
```
**Examples:** `123UL`, `456LU`, `0xFFFFFFFFUL`

### `<double>`
```ebnf
<double> ::= ? A floating-point constant token ?
```
**Examples:** `3.14`, `2.5e10`, `1.23E-4`, `.5`, `10.`