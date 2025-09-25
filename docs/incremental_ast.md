program = Program(function_definition)

declaration = FunDecl(function_declaration) | VarDecl(variable_declaration)

variable_declaration = (identifier name, exp? init,type var_type, storage_class?)

function_declaration = (identifier name, identifier* params, block? body,type fun_type, storage_class?)

type = Int | Long | FunType(type* params, type ret)

storage_class = Static | Extern

block = Block(block_item*)

for_init = InitDecl(declaration) | InitExp(exp?)

block_item = S(statement) | D(declaration)  

statement = Return(exp)  
| Expression(exp) 
| If(exp condition, statment then, statement? else) 
| Compound(block) 
| Break(identifier label)
| Continue(identifier label)
| While(exp condition, statement body, identifier label)
| DoWhile(statement body, exp condition, identifier label)
| For(for_init init, exp? condition, exp? post, statement body, identifier label)
| Null 
| Goto(label) 
| Label

exp = Constant(const) 
| Unary(unary_operator, exp) 
| Binary(binary_operator, exp, exp) 
| Var(identifier) 
| Assignment(exp,exp) 
| Conditional(exp condition, exp, exp)
| FunctionCall(identifier, exp* args)
| Cast(type target_type, exp)

const = ConstInt(int) | ConstLong(int)

unary_operator = Complement | Negate

binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or | Xor | LeftShift | RightShift | Land | Lor
| Equal(==) | NotEqual(!=) | LessThan(<) | LessOrEqual(<=)
| GreaterThan(>) | GreaterOrEqual(>=)