# Velox C Compiler AST Structure

This document defines the Abstract Syntax Tree (AST) structure for the Velox C compiler using algebraic data type notation.

## Program Structure

### `program`
```
program = Program(declaration*)
```

### `declaration`
```
declaration = FunDecl(function_declaration) 
            | VarDecl(variable_declaration)
            | StructDecl(struct_declaration)
```

## Declarations

### `variable_declaration`
```
variable_declaration = (identifier name, initializer? init, type var_type, storage_class?)
```

### `function_declaration`
```
function_declaration = (identifier name, identifier* params, block? body, type fun_type, storage_class?)
```

### `struct_declaration`
```
struct_declaration = (identifier tag, member_declaration* members)
```

### `member_declaration`
```
member_declaration = (identifier member_name, type member_type)
```

## Initializers

### `initializer`
```
initializer = SingleInit(exp) 
            | CompoundInit(initializer*)
```

## Type System

### `type`
```
type = Char | SChar | UChar | Int | Long | UInt | ULong | Double | Void
     | FunType(type* params, type ret)
     | Pointer(type referenced)
     | Array(type element, int size)
     | Structure(identifier tag)
```

### `storage_class`
```
storage_class = Static | Extern
```

## Statements and Blocks

### `block_item`
```
block_item = S(statement) | D(declaration)
```

### `block`
```
block = Block(block_item*)
```

### `for_init`
```
for_init = InitDecl(variable_declaration) | InitExp(exp?)
```

### `statement`
```
statement = Return(exp?)
          | Expression(exp)
          | If(exp condition, statement then, statement? else)
          | Compound(block)
          | Break
          | Continue
          | While(exp condition, statement body)
          | DoWhile(statement body, exp condition)
          | For(for_init init, exp? condition, exp? post, statement body)
          | Null
```

## Expressions

### `exp`
```
exp = Constant(const)
    | String(string)
    | Var(identifier)
    | Cast(type target_type, exp)
    | Unary(unary_operator, exp)
    | Binary(binary_operator, exp, exp)
    | Assignment(exp, exp)
    | Conditional(exp condition, exp, exp)
    | FunctionCall(identifier, exp* args)
    | Dereference(exp)
    | AddrOf(exp)
    | Subscript(exp, exp)
    | SizeOf(exp)
    | SizeOfT(type)
    | Dot(exp structure, identifier member)
    | Arrow(exp pointer, identifier member)
```

## Operators

### `unary_operator`
```
unary_operator = Complement | Negate | Not
```

### `binary_operator`
```
binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or
                | Equal | NotEqual | LessThan | LessOrEqual
                | GreaterThan | GreaterOrEqual
```

## Constants

### `const`
```
const = ConstInt(int) | ConstLong(int) | ConstUInt(int) | ConstULong(int)
      | ConstDouble(double) | ConstChar(int) | ConstUChar(int)
```