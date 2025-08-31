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

enumerator = (identifier name, const? value) 
```
// then `value` should be restricted to integer constants (you can enforce that in the type checker).

### `typedef_declaration`
```
typedef_declaration = (typedef_entry+ entries)

typedef_entry = (identifier name, type alias_type)
```
### `class_declaration`
```
class_declaration =
  (identifier name,
   base_spec* bases,           
   class_member* members)

base_spec = (identifier base_name, access_spec access)

class_member = Field(field_member) | Method(method_member) | Ctor(constructor_member)

field_member = (variable_declaration var, access_spec access)

method_member =
  (function_declaration fun,
   access_spec access)

constructor_member =
  (identifier class_name,
   identifier* params,             // names
   type ctor_type,                 // FunType(param_types, Void, false)
   block? body,
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
     | FunType(type* params, type ret, bool is_varargs)
     | Pointer(type referenced)
     | Array(type element, array_size size)
     | Structure(identifier tag)
     | Union(identifier tag)
     | Enum(identifier tag)
     | NamedType(identifier name) 
     | Class(identifier name) 
     | VaList
     | Reference(type referenced)

array_size = Known(int) | Unspecified | VLA(exp)
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
          | Labeled(label* labels, statement stmt)           //Allow multiple labels on one statement
          | Switch(exp selector, statement body)    
          | Goto(identifier target)                   
```

## Expressions

### `exp`
```
exp = Constant(const)
    | String(string)
    | Var(identifier)
    | Cast(type target_type, exp)
    | Unary(unary_operator, exp)
    | PreInc(exp)      // ++x
    | PreDec(exp)      // --x
    | PostInc(exp)     // x++
    | PostDec(exp)     // x--
    | Binary(binary_operator, exp, exp)
    | Assignment(exp lhs, assign_op op, exp rhs)   // replace old 2-arg form
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
    | VaStart(exp ap, identifier last_param)           
    | VaEnd(exp ap)                                    
    | VaArg(exp ap, type target_type)
    | Builtin(identifier name)             
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
binary_operator =   Add | Subtract | Multiply | Divide | Remainder 
                    | BitAnd | BitOr | BitXor
                    | ShiftLeft | ShiftRight
                    | LogicalAnd | LogicalOr
                    | Equal | NotEqual | LessThan | LessOrEqual | GreaterThan | GreaterOrEqual
```
### `assignment_operator`

assign_op = Assign | AddAssign | SubAssign | MulAssign | DivAssign | ModAssign
          | ShlAssign | ShrAssign | AndAssign | XorAssign | OrAssign


## Constants

### `const`
```
const = ConstInt(int) | ConstLong(int) | ConstUInt(int) | ConstULong(int)
      | ConstDouble(double) | ConstChar(int) | ConstUChar(int)
```

## Label Support
```
label =
    UserLabel(identifier name)     // e.g., foo:
  | CaseLabel(exp value)         // e.g., case 42:
  | DefaultLabel                   // e.g., default:
```