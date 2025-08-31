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
            | UnionDecl(union_declaration)
            | EnumDecl(enum_declaration)
            | TypedefDecl(typedef_declaration)
            | ClassDecl(class_declaration)  
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

### `union_declaration`
```
union_declaration = (identifier tag, member_declaration* members)
```

### `member_declaration`
```
member_declaration = (identifier member_name, type member_type)
```

### `enum_declaration`
```
enum_declaration = (identifier? tag, enumerator* enumerators)

enumerator = (identifier name, int? value) 
```
//Here we can also have `const? value` in place of `int? value`. But then `value` should be restricted to integer constants (you can enforce that in the type checker).

### `typedef_declaration`
```
typedef_declaration = (typedef_entry+ entries)

typedef_entry = (identifier name, type alias_type)
```
### `class_declaration`
```
class_declaration = (identifier name, class_member* members)

class_member = Field(field_member) | Method(method_member) | Ctor(constructor_member)

field_member = (variable_declaration var, access_spec access)

method_member =
  (function_declaration fun,
   access_spec access,
   bool is_static)                  // false = instance method, true = static method

constructor_member =
  (identifier class_name,           // must match the class name
   identifier* params,              // parameter names
   block? body,                     // null => declared only
   access_spec access)

access_spec = Public | Private    //Access checking is semantic check not a parsing concern
```

## Initializers

### `initializer`
```
initializer = SingleInit(exp) 
            | CompoundInit(initializer*)
            | CtorInit(exp* args)       //for classes
```

## Type System

### `type`
```
type = Char | SChar | UChar | Int | Long | UInt | ULong | Double | Void
     | FunType(type* params, type ret)
     | Pointer(type referenced)
     | Array(type element, int size)
     | Structure(identifier tag)
     | Union(identifier tag)
     | Enum(identifier tag)
     | NamedType(identifier name) 
     | Class(identifier name) 
```

//During type checking you resolve NamedType via the symbol table to the underlying type

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
    | FunctionCall(exp callee, exp* args)
    | Dereference(exp)
    | AddrOf(exp)
    | Subscript(exp, exp)
    | SizeOf(exp)
    | SizeOfT(type)
    | Dot(exp structure, identifier member)
    | Arrow(exp pointer, identifier member)
    | EnumValue(identifier name)  
```
//During codegen (or a lower pass), you may replace e.g. EnumValue("MEDIUM") with Constant(ConstInt(k)) where k is the resolved integer.

//FunctionCall(exp callee, exp* args)
Why:
- This lets you call function pointers (FunctionCall(Var("fp"), ...)).
- It also allows chained calls 

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