# Velox

**Velox** is a C compiler implementation that translates C source code to x86-64 assembly. Built from scratch with a custom lexer, parser, intermediate representation (IR), and code generator.

## Roadmap

### Core Compiler Infrastructure
- [x] Implement a compiler driver with custom lexer, parser, IR generation, and assembly emission
- [x] Add support for preprocessing and linking (via GCC)

### Operators & Expressions
- [x] Add support for Unary Operations (Negation, Logical NOT, Bitwise NOT)
- [x] Add support for Binary Operations (Add, Sub, Mul, Div, Rem)
- [x] Add support for Bitwise Binary Operations (And, Or, Xor, LeftShift, RightShift)
- [x] Add support for logical and relational operators (`!` `&&` `||` `==` `!=` `<` `>` `<=` `>=`)
- [x] Add support for ternary operator (`? :`)
- [x] Add support for increment/decrement operators (`++` `--`)
- [x] Add support for compound assignment operators (`+=` `-=` `*=` `/=` `<<=` `>>=` `%=` `&=` `|=` `^=`)

### Variables & Scope
- [x] Add support for user-defined local variables
- [x] Add support for static keyword and global scope
- [x] Add support for extern keyword

### Control Flow
- [x] Add support for if-else construct
- [x] Add support for compound statements
- [x] Add support for goto statement and labels
- [x] Add support for Loops (for, while, do-while, until, do-until)
- [x] Add support for break and continue

### Functions
- [x] Add support for function declarations and definitions
- [x] Add support for function calls
- [x] Add support for variadic argument function calling (printf, scanf, etc.)
- [x] Add support for returning void

### Type System
- [x] Add support for a type system (int, long, unsigned, double, char, pointers)
- [x] Add support for Long Integers (64-bit)
- [x] Add support for Unsigned Integers (uint, ulong)
- [x] Add support for double precision floating point
- [x] Add support for characters (char, signed char, unsigned char)
- [x] Add support for type casting and implicit conversions

### Pointers & Memory
- [x] Add support for Pointers
- [x] Add support for Pointer Arithmetic
- [x] Add support for address-of (`&`) and dereference (`*`) operators
- [x] Add support for dynamic memory allocation (malloc, free)

### Arrays
- [x] Add support for one-dimensional Arrays
- [x] Add support for multi-dimensional Arrays
- [x] Add support for array subscripting (`[]`)
- [x] Add support for string literals and array initialization
- [x] Add support for arrays of structs

### Structures
- [x] Add support for user-defined structures (struct)
- [x] Add support for struct member access (`.` operator)
- [x] Add support for struct pointer member access (`->` operator)
- [x] Add support for struct compound initializers
- [x] Add support for nested structs
- [x] Add support for arrays of structs
- [ ] Add support for struct function parameters (pass by value)
- [ ] Add support for struct function return values (return by value)

## Features

#### Syntax Restrictions
- **Multiple declarations**: Declaration like `int x, y;` are not supported. Use separate lines: `int x;` and `int y;`
- **Empty parameter lists**: You must explicitly add `void`. Write `int foo(void);` instead of `int foo();`
- **Unnamed parameters**: Function declarations must name all parameters. Write `int foo(int a, int b);` instead of `int foo(int, int);`
- **Array parameters**: Not supported. Rewrite `int foo(int a[])` to `int foo(int *a)`
- **Const keyword**: Not supported
- **Function pointers**: Not supported

#### Semantic Differences
- **Compound operations**: These are expanded to their base expressions (e.g., `a += 2` becomes `a = a + 2`, `b++` becomes `b = b + 1`)
- **Pointer comparison**: For comparing pointers, the base type needs to be the same. Cast to `void*` if needed
- **sizeof('a')**: Evaluates to 1 (character size), unlike GCC/Clang which return 4 (int promotion)

#### Not Yet Implemented
- **Struct pass/return by value**: Structures cannot be passed as function parameters or returned from functions by value
- **Static struct variables**: Partial support - some edge cases may not work
- **Optimizations**: No optimization passes are implemented yet

## Resources

Our compiler implementation draws primarily from Nora Sandler's **"Writing a C Compiler"** book, the test suite credit also goes to Nora Sandler.

## Building Velox

```bash
make
```

The compiler binary will be generated in the `build/` directory.

## Usage

```bash
./build/velox source.vlx 
```

The command would generate the executable binary with the name "source"

## Examples

The `tests/` directory contains various example programs demonstrating Velox's features:

- **Control Flow**: `test_ifelse.vlx`, `test_loops.vlx`, `test_control_flow.vlx`
- **Operators**: `test_arithmetic.vlx`, `test_logical.vlx`
- **Arrays**: `test_array.vlx`
- **Pointers**: `test_pointer.vlx`
- **Memory**: `test_malloc.vlx`
- **Functions**: `test_variadic.vlx`, `test_recursive.vlx`
- **File I/O**: `test_file_manipulation.vlx`
- **Command Line**: `test_cmdline.vlx`
- **Static Variables**: `test_static.vlx`
- **Algorithms**: `tower_of_hanoi.vlx`, `string_reversal.vlx`

Feel free to explore these examples to understand Velox's capabilities. **Contributions of new example programs are welcome!** If you create interesting test cases or example programs, please submit them to the `tests/` directory via pull request.

## Contributing

We welcome contributions to Velox! Here are some guidelines to get you started:

### Code Formatting

We use `clang-format` for code formatting. Please ensure all your modified files are formatted before submitting:

```bash
clang-format -i path/to/modified/files/
```

You can also run the formatter on all source files:

```bash
find -name "*.[ch]pp" | xargs clang-format -i
```

### Pull Requests

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Ensure your code is formatted
5. Submit a pull request

More detailed contribution guidelines will be added as the project evolves.

## Authors

[Abdullah Azeem](https://github.com/ABD-AZE) <br>
[Pradyuman Singh Shekhawat](https://github.com/ss-prady) <br>
[Pradyumn Kejriwal](https://github.com/Pradyumn2004) <br>
[Nisarg Prajapati](https://github.com/Munnond)
