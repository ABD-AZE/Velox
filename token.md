0. Non‑emitting category sentinels
    | Token (enum)                | Description                                                                       | Regex |
    | --------------------------- | --------------------------------------------------------------------------------- | ----- |
    | UNKNOWN                     | Placeholder for unknown token kind (shouldn’t be emitted once errors are handled) | —     |
    | CONSTANTSTART / CONSTANTEND | Delimit “constants” block in enum                                                 | —     |
    | KEYWORDSTART / KEYWORDEND   | Delimit “keywords” block in enum                                                  | —     |
    | GRAMMARSTART / GRAMMAREND   | Delimit “grammar/punctuator” block in enum                                        | —     |
    | UNARYSTART / UNARYEND       | Delimit “unary operators” block                                                   | —     |
    | BINARYSTART / BINARYEND     | Delimit “binary operators” block                                                  | —     |
    | BITWISESTART / BITWISEEND   | Delimit “bitwise operators” block                                                 | —     |
    | LOGICALSTART / LOGICALEND   | Delimit “logical operators” block                                                 | —     |

1. Whitespace & Comments (skip)
    | Token          | Description                         | Regex                          |
    | -------------- | ----------------------------------- | ------------------------------ |
    | WS             | Whitespace (spaces, tabs, newlines) | `\s+`                          |
    | LINE\_COMMENT  | `// …` to end of line               | `\/\/[^\n]*`                   |
    | BLOCK\_COMMENT | `/* … */` (non‑nested)              | `\/\*[\s\S]*?\*\/`             |

2. Identifiers
    | Token      | Description       | Regex                    |
    | ---------- | ----------------- | ------------------------ |
    | IDENTIFIER | User‑defined name | `[_A-Za-z][_A-Za-z0-9]*` |

3. Constants (literals)
   
    3.1 Integer constants
   
    | Token           | Description                        | Regex                            | 
    | --------------- | ---------------------------------- | -------------------------------- | 
    | INT\_CONSTANT   | Signed decimal integer (no suffix) | `[0-9]`                          | 
    | UINT\_CONSTANT  | Unsigned decimal integer (u/U)     | `[0-9]+[uU]\b`                   |     
    | LONG\_CONSTANT  | Long decimal integer (l/L)         | `[0-9]+[lL]\b`                   | 
    | ULONG\_CONSTANT | Unsigned long (ul/lu, any case)    | `[0-9]+(([uU][lL])/|([lL][uU]))` | 

    3.2 Floating constants
   
    | Token            | Description                                        | Regex               |                          
    | ---------------- | -------------------------------------------------- | ------------------- | 
    | DOUBLE\_CONSTANT | Floating‑point constant (fraction and/or exponent) | `[0-9]+\.[0-9]+\b`  |
   
    3.3 Character & string constants
   
    | Token     | Description                          | Regex             | 
    | --------- | ------------------------------------ | ----------------- | 
    | CHARACTER | Character literal (supports escapes) | `'(\\.|[^'])'`    | 
    | STRING    | String literal (supports escapes)    | `"(\\.|[^"\\])*"` |

5. Keywords
   
    | Token         | Description               | Regex          |
    | ------------- | ------------------------- | -------------- |
    | VOID          | `void`                    | `\bvoid\b`     |
    | RETURN        | `return`                  | `\breturn\b`   |
    | IF            | `if`                      | `\bif\b`       |
    | ELSE          | `else`                    | `\belse\b`     |
    | DO            | `do`                      | `\bdo\b`       |
    | WHILE         | `while`                   | `\bwhile\b`    |
    | UNTIL         | `until`                   | `\buntil\b`    |
    | FOR           | `for`                     | `\bfor\b`      |
    | BREAK         | `break`                   | `\bbreak\b`    |
    | CONTINUE      | `continue`                | `\bcontinue\b` |
    | STATIC        | `static`                  | `\bstatic\b`   |
    | EXTERN        | `extern`                  | `\bextern\b`   |
    | INT           | `int`                     | `\bint\b`      |
    | LONG          | `long`                    | `\blong\b`     |
    | SIGNED        | `signed`                  | `\bsigned\b`   |
    | UNSIGNED      | `unsigned`                | `\bunsigned\b` |
    | DOUBLE        | `double`                  | `\bdouble\b`   |
    | CHAR          | `char`                    | `\bchar\b`     |
    | SIZEOF        | `sizeof`                  | `\bsizeof\b`   |
    | STRUCT        | `struct`                  | `\bstruct\b`   |
    | GOTO          | `goto`                    | `\bgoto\b`     |
    | SWITCH        | `switch`                  | `\bswitch\b`   |
    | CASE          | `case`                    | `\bcase\b`     |
    | DEFAULT\_CASE | `default`                 | `\bdefault\b`  |

7. Punctuators / Grammar tokens
    | Token              | Description         | Regex    |
    | ------------------ | ------------------- | -------- |
    | OPEN\_PARANTHESES  | `(`                 | `\(`     |
    | CLOSE\_PARANTHESES | `)`                 | `\)`     |
    | OPEN\_BRACE        | `{`                 | `\{`     |
    | CLOSE\_BRACE       | `}`                 | `\}`     |
    | OPEN\_BRACKET      | `[`                 | `\[`     |
    | CLOSE\_BRACKET     | `]`                 | `\]`     |
    | SEMICOLON          | `;`                 | `;`      |
    | COLON              | `:`                 | `:`      |
    | COMMA              | `,`                 | `,`      |
    | DOT                | `.` (member access) | `\.`     |
    | QUESTION\_MARK     | `?` (ternary)       | `\?`     |
    | ELLIPSIS           | `...`               | `\.\.\.` |

8. Operators
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
    | AOR          | `|` bitwise OR  | \|`           |
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
    | COMPOUND\_OR         | \`          | =\`   | 
    | COMPOUND\_LEFTSHIFT  | `<<=`       | `<<=` |       
    | COMPOUND\_RIGHTSHIFT | `>>=`       | `>>=` |       

9. Error handling
    | Token         | Description                                                                      | Regex                      |
    | ------------- | -------------------------------------------------------------------------------- | -------------------------- |
    | INVALID\_CHAR | Any single unmatched character (emit and continue to collect all lexical errors) | `.` (final catch‑all rule) |

