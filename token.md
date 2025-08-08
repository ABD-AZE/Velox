# List of Velox Tokens
## 1. Whitespace & Comments (skip)

| Token          | Description                         | Regex                          |
| -------------- | ----------------------------------- | ------------------------------ |
| WS             | Whitespace (spaces, tabs, newlines) | `[ \t\n\r\f\v]+`               |
| LINE\_COMMENT  | `// …` to end of line               | `\/\/[^\n]*`                   |
| BLOCK\_COMMENT | `/* … */` (non‑nested)              | `\/\*([^*]\|\*+[^*/])*\*+\/`    |

## 2. Identifiers

| Token      | Description       | Regex                    |
| ---------- | ----------------- | ------------------------ |
| IDENTIFIER | User‑defined name | `[_A-Za-z][_A-Za-z0-9]*` |

## 3. Constants (literals)

### 3.1 Integer constants

| Token           | Description                        | Regex                            | 
| --------------- | ---------------------------------- | -------------------------------- | 
| INT\_CONSTANT   | Signed decimal integer (no suffix) | `[0-9]+`                         | 
| UINT\_CONSTANT  | Unsigned decimal integer (u/U)     | `[0-9]+[uU]`                     |     
| LONG\_CONSTANT  | Long decimal integer (l/L)         | `[0-9]+[lL]`                     | 
| ULONG\_CONSTANT | Unsigned long (ul/lu, any case)    | `[0-9]+(([uU][lL])\|([lL][uU]))` | 

### 3.2 Floating constants
   
| Token            | Description                                        | Regex                            |               
| ---------------- | -------------------------------------------------- | -------------------------------- | 
| FLOAT\_CONSTANT  | Floating‑point constant (123. , .123 , 123.123)    | `([0-9]+\.[0-9]*)\|(\.[0-9]+)`   |
   
### 3.3 Character & string constants
   
| Token     | Description                          | Regex             | 
| --------- | ------------------------------------ | ----------------- | 
| CHARACTER | Character literal (supports escapes) | `'(\\.\|[^'])'`    | 
| STRING    | String literal (supports escapes)    | `"(\\.\|[^"\\])*"` |

## 4. Keywords
   
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
| `PRINTF`       | `printf` function call                        | `printf`   |
| `SCANF`        | `scanf` function call                         | `scanf`    |
| `TYPEDEF`      | Create new type alias                         | `typedef`  |
| `ENUM`         | User-defined integer constants group          | `enum`     |
| `UNION`        | Shared memory for different members           | `union`    |


## 5. Punctuators / Grammar tokens
Note: Some of these characters need the escape sequence because they have special meaning in regex syntax
   
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

## 6. Operators
### 6.1 Unary operators

| Token                  | Description                            | Regex  |
| ---------------------- | -------------------------------------- | ------ |
| TILDE                  | `~` bitwise complement                 | `~`    |
| HYPHEN                 | `-` unary negation (also binary minus) | `-`    |
| DECREMENT\_OPERATOR    | `--`                                   | `--`   |
| INCREMENT\_OPERATOR    | `++`                                   | `\+\+` |
| NOT                    | `!` logical NOT                        | `!`    |
| AMP                    | `&` address‑of (also bitwise AND)      | `&`    |
| ASTERISK               | `*` dereference (also multiply)        | `\*`   |

### 6.2 Binary arithmetic & misc

| Token           | Description             | Regex |
| --------------- | ----------------------- | ----- |
| PLUS            | `+` addition            | `\+`  |
| ASTERISK        | `*` multiplication      | `\*`  |
| FORWARD\_SLASH  | `/` division            | `/`   |
| PERCENT\_SIGN   | `%` remainder           | `%`   |
| ASSIGNMENT      | `=` assignment          | `=`   |
| ARROW\_OPERATOR | `->` member via pointer | `->`  |

### 6.3 Comparisons (logical group)

| Token            | Description           | Regex |
| ---------------- | --------------------- | ----- |
| EQUAL            | `==` equal to         | `==`  |
| NOTEQUAL         | `!=` not equal        | `!=`  |
| LESSTHAN         | `<` less than         | `<`   |
| GREATERTHAN      | `>` greater than      | `>`   |
| LESSTHANEQUAL    | `<=` less or equal    | `<=`  |
| GREATERTHANEQUAL | `>=` greater or equal | `>=`  |

### 6.4 Logical connective operators

| Token | Description      | Regex | 
| ----- | ---------------- | ----- | 
| LAND  | `&&` logical AND | `&&`  |  
| LOR   | `\|\|` logical OR| `\|\|`| 

### 6.5 Bitwise operators (and shifts)

| Token        | Description     | Regex         |  
| ------------ | --------------- | ------------- | 
| AAND         | `&` bitwise AND | `&`           |   
| AOR          | `\|` bitwise OR | `\|`          |
| XOR          | `^` bitwise XOR | `\^`          |  
| LEFT\_SHIFT  | `<<`            | `<<`          |  
| RIGHT\_SHIFT | `>>`            | `>>`          |  

### 6.6 Compound assignments

| Token                | Description | Regex | 
| -------------------- | ----------- | ----- | 
| COMPOUND\_SUM        | `+=`        | `\+=` |  
| COMPOUND\_DIFFERENCE | `-=`        | `-=`  |   
| COMPOUND\_PRODUCT    | `*=`        | `\*=` |
| COMPOUND\_DIVISION   | `/=`        | `/=`  |  
| COMPOUND\_REMAINDER  | `%=`        | `%=`  |       
| COMPOUND\_AND        | `&=`        | `&=`  |       
| COMPOUND\_XOR        | `^=`        | `\^=` |       
| COMPOUND\_OR         | `\`         | `\|=` | 
| COMPOUND\_LEFTSHIFT  | `<<=`       | `<<=` |       
| COMPOUND\_RIGHTSHIFT | `>>=`       | `>>=` |   

# Priority Order
Key Considerations When Ordering:
1. Longest match wins: Multi-character tokens (like >>= or +=) must be placed before their prefixes (like >, > or +), otherwise the lexer will prematurely match a shorter token.
2. Keywords before identifiers: int, for, if, etc., should match as keywords, not identifiers.
3. Whitespace and comments: These should be matched early so they can be skipped cleanly.
4. Specificity and greediness:
   a. Floating-point regex must come before integer constants.
   b. IDENTIFIER must come after all keywords to avoid misclassification.
5. Operators: Multi-character operators like >>=, <<=, ++, -- should be matched before their constituent symbols.
6. Compound vs basic operators: += before +, >>= before >>, >> before > etc.


Here is the **complete and correctly ordered priority list** of all the tokens you provided, arranged according to lexical analysis principles like **longest match wins**, **keywords before identifiers**, **whitespace/comments early**, and **compound before base operators**.

---
1. `WS`
2. `LINE_COMMENT`
3. `BLOCK_COMMENT`
---
4. `ELLIPSIS`
5. `COMPOUND_RIGHTSHIFT`
6. `COMPOUND_LEFTSHIFT`
7. `COMPOUND_SUM`
8. `COMPOUND_DIFFERENCE`
9. `COMPOUND_PRODUCT`
10. `COMPOUND_DIVISION`
11. `COMPOUND_REMAINDER`
12. `COMPOUND_AND`
13. `COMPOUND_XOR`
14. `COMPOUND_OR`
15. `EQUAL`
16. `NOTEQUAL`
17. `LESSTHANEQUAL`
18. `GREATERTHANEQUAL`
19. `INCREMENT_OPERATOR`
20. `DECREMENT_OPERATOR`
21. `ARROW_OPERATOR`
22. `LEFT_SHIFT`
23. `RIGHT_SHIFT`
24. `LAND`
25. `LOR`
---
26. `VOID`
27. `RETURN`
28. `IF`
29. `ELSE`
30. `DO`
31. `WHILE`
32. `FOR`
33. `BREAK`
34. `CONTINUE`
35. `STATIC`
36. `EXTERN`
37. `INT`
38. `LONG`
39. `SIGNED`
40. `UNSIGNED`
41. `DOUBLE`
42. `CHAR`
43. `SIZEOF`
44. `STRUCT`
45. `GOTO`
46. `SWITCH`
47. `CASE`
48. `DEFAULT_CASE`
49. `PRINTF`
50. `SCANF`
51. `TYPEDEF`
52. `ENUM`
53. `UNION`
---
54. `FLOAT_CONSTANT`
55. `ULONG_CONSTANT`
56. `LONG_CONSTANT`
57. `UINT_CONSTANT`
58. `INT_CONSTANT`
---
59. `CHARACTER`
60. `STRING`
---
61. `IDENTIFIER`
---
62. `OPEN_PARANTHESES`
63. `CLOSE_PARANTHESES`
64. `OPEN_BRACE`
65. `CLOSE_BRACE`
66. `OPEN_BRACKET`
67. `CLOSE_BRACKET`
68. `SEMICOLON`
69. `COLON`
70. `COMMA`
71. `DOT`
72. `QUESTION_MARK`
---
73. `ASSIGNMENT`
74. `PLUS`
75. `HYPHEN`
76. `ASTERISK`
77. `FORWARD_SLASH`
78. `PERCENT_SIGN`
---
79. `TILDE`
80. `NOT`
81. `AMP`
---
82. `LESSTHAN`
83. `GREATERTHAN`
84. `AAND`
85. `AOR`
86. `XOR`
