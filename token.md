1. Whitespace & Comments (skip)
    | Token          | Description                         | Regex                          |
    | -------------- | ----------------------------------- | ------------------------------ |
    | WS             | Whitespace (spaces, tabs, newlines) | `[ \t\n\r\f\v]+`               |
    | LINE\_COMMENT  | `// …` to end of line               | `\/\/[^\n]*`                   |
    | BLOCK\_COMMENT | `/* … */` (non‑nested)              | `\/\*([^*]|\*+[^*/])*\*+\/`    |

2. Identifiers
    | Token      | Description       | Regex                    |
    | ---------- | ----------------- | ------------------------ |
    | IDENTIFIER | User‑defined name | `[_A-Za-z][_A-Za-z0-9]*` |

3. Constants (literals)
   
    3.1 Integer constants
   
    | Token           | Description                        | Regex                            | 
    | --------------- | ---------------------------------- | -------------------------------- | 
    | INT\_CONSTANT   | Signed decimal integer (no suffix) | `[0-9]+`                         | 
    | UINT\_CONSTANT  | Unsigned decimal integer (u/U)     | `[0-9]+[uU]`                     |     
    | LONG\_CONSTANT  | Long decimal integer (l/L)         | `[0-9]+[lL]`                     | 
    | ULONG\_CONSTANT | Unsigned long (ul/lu, any case)    | `[0-9]+(([uU][lL])\|([lL][uU]))` | 

    3.2 Floating constants
   
    | Token            | Description                                        | Regex                          |               
    | ---------------- | -------------------------------------------------- | ----------------------------   | 
    | DOUBLE\_CONSTANT | Floating‑point constant (fraction with no exponent)| `([0-9]+\.[0-9]*)|(\.[0-9]+)`  |
   
    3.3 Character & string constants
   
    | Token     | Description                          | Regex             | 
    | --------- | ------------------------------------ | ----------------- | 
    | CHARACTER | Character literal (supports escapes) | `'(\\.\|[^'])'`    | 
    | STRING    | String literal (supports escapes)    | `"(\\.\|[^"\\])*"` |

4. Keywords
   
    | Token          | Description                                   | Regex      |
    | -------------- | --------------------------------------------- | ---------- |
    | `VOID`         | No return value or no data type               | `void`     |
    | `RETURN`       | Exit a function and optionally return value   | `return`   |
    | `IF`           | Conditional branching                         | `if`       |
    | `ELSE`         | Alternate path for `if` condition             | `else`     |
    | `DO`           | Start of a do-while loop                      | `do`       |
    | `WHILE`        | Loop that runs while condition is true        | `while`    |
    | `FOR`          | General-purpose counting loop                 | `for`      |
    | `BREAK`        | Exit from loops or switch                     | `break`    |
    | `CONTINUE`     | Skip to next iteration of loop                | `continue` |
    | `STATIC`       | Persist across calls or limit linkage         | `static`   |
    | `EXTERN`       | Declare global variable/function elsewhere    | `extern`   |
    | `INT`          | Integer data type                             | `int`      |
    | `LONG`         | Extended-size integer                         | `long`     |
    | `SIGNED`       | Signed integer type                           | `signed`   |
    | `UNSIGNED`     | Only non-negative integers                    | `unsigned` |
    | `DOUBLE`       | Double-precision floating-point type          | `double`   |
    | `CHAR`         | Single character data type                    | `char`     |
    | `SIZEOF`       | Yields size of a type or variable (in bytes)  | `sizeof`   |
    | `STRUCT`       | User-defined data structure                   | `struct`   |
    | `GOTO`         | Unconditional jump to a label                 | `goto`     |
    | `SWITCH`       | Multi-branch conditional                      | `switch`   |
    | `CASE`         | Label within a `switch` block                 | `case`     |
    | `DEFAULT_CASE` | Fallback label in a `switch` block            | `default`  |


5. Punctuators / Grammar tokens
    Note: Some of these characgters need the escape sequence because they have special meaning in regex syntax
   
    | Token              | Description         | Regex     |
    | ------------------ | ------------------- | --------- |
    | OPEN\_PARANTHESES  | `(`                 | `\(`      |
    | CLOSE\_PARANTHESES | `)`                 | `\)`      |
    | OPEN\_BRACE        | `{`                 | `\{`      |
    | CLOSE\_BRACE       | `}`                 | `\}`      |
    | OPEN\_BRACKET      | `[`                 | `\[`      |
    | CLOSE\_BRACKET     | `]`                 | `\]`      |
    | SEMICOLON          | `;`                 | `;`       |
    | COLON              | `:`                 | `:`       |
    | COMMA              | `,`                 | `,`       |
    | DOT                | `.` (member access) | `\.`      |
    | QUESTION\_MARK     | `?` (ternary)       | `\?`      |
    | ELLIPSIS           | `...`               | `\.\.\.`  |

6. Operators
    6.1 Unary operators
    | Token                  | Description                            | Regex  |
    | ---------------------- | -------------------------------------- | ------ |
    | TILDE                  | `~` bitwise complement                 | `~`    |
    | HYPHEN                 | `-` unary negation (also binary minus) | `-`    |
    | DECREMENT\_OPERATOR    | `--`                                   | `--`   |
    | INCREMENT\_OPERATOR    | `++`                                   | `\+\+` |
    | NOT                    | `!` logical NOT                        | `!`    |
    | AMP (address‑of)       | `&` address‑of (also bitwise AND)      | `&`    |
    | ASTERISK (dereference) | `*` dereference (also multiply)        | `\*`   |

    6.2 Binary arithmetic & misc
    | Token           | Description             | Regex |
    | --------------- | ----------------------- | ----- |
    | PLUS            | `+` addition            | `\+`  |
    | ASTERISK        | `*` multiplication      | `\*`  |
    | FORWARD\_SLASH  | `/` division            | `/`   |
    | PERCENT\_SIGN   | `%` remainder           | `%`   |
    | ASSIGNMENT      | `=` assignment          | `=`   |
    | ARROW\_OPERATOR | `->` member via pointer | `->`  |

    6.3 Comparisons (logical group)
    | Token            | Description           | Regex |
    | ---------------- | --------------------- | ----- |
    | EQUAL            | `==` equal to         | `==`  |
    | NOTEQUAL         | `!=` not equal        | `!=`  |
    | LESSTHAN         | `<` less than         | `<`   |
    | GREATERTHAN      | `>` greater than      | `>`   |
    | LESSTHANEQUAL    | `<=` less or equal    | `<=`  |
    | GREATERTHANEQUAL | `>=` greater or equal | `>=`  |

    6.4 Logical connective operators
    | Token | Description      | Regex | 
    | ----- | ---------------- | ----- | 
    | LAND  | `&&` logical AND | `&&`  |  
    | LOR   | `||` logical OR  | `||`  | 

    6.5 Bitwise operators (and shifts)
    | Token        | Description     | Regex         |  
    | ------------ | --------------- | ------------- | 
    | AAND         | `&` bitwise AND | `&`           |   
    | AOR          | `|` bitwise OR  | `\|`          |
    | XOR          | `^` bitwise XOR | `\^`          |  
    | LEFT\_SHIFT  | `<<`            | `<<`          |  
    | RIGHT\_SHIFT | `>>`            | `>>`          |  

    6.6 Compound assignments
    | Token                | Description | Regex | 
    | -------------------- | ----------- | ----- | 
    | COMPOUND\_SUM        | `+=`        | `\+=` |  
    | COMPOUND\_DIFFERENCE | `-=`        | `-=`  |   
    | COMPOUND\_PRODUCT    | `*=`        | `\*=` |
    | COMPOUND\_DIVISION   | `/=`        | `/=`  |  
    | COMPOUND\_REMAINDER  | `%=`        | `%=`  |       
    | COMPOUND\_AND        | `&=`        | `&=`  |       
    | COMPOUND\_XOR        | `^=`        | `\^=` |       
    | COMPOUND\_OR         | `\`         | `=\`  | 
    | COMPOUND\_LEFTSHIFT  | `<<=`       | `<<=` |       
    | COMPOUND\_RIGHTSHIFT | `>>=`       | `>>=` |       

9. Error handling
    | Token         | Description                                                                      | Regex                      |
    | ------------- | -------------------------------------------------------------------------------- | -------------------------- |
    | INVALID\_CHAR | Any single unmatched character (emit and continue to collect all lexical errors) | `.` (final catch‑all rule) |

