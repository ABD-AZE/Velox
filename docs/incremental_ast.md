program = Program(function_definition)

function_definition = Function(identifier name, block body)

block = Block(block_item*)

declaration = Declaration(identifier name, exp? init)

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
| Switch()
| Null 
| Goto(label) 
| Label
| Switch(case* , )

exp = Constant(int) | Unary(unary_operator, exp) | Binary(binary_operator, exp, exp) | Var(identifier) | Assignment(exp,exp) | Conditional(exp condition, exp, exp)

unary_operator = Complement | Negate

binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or | Xor | LeftShift | RightShift | Land | Lor
| Equal(==) | NotEqual(!=) | LessThan(<) | LessOrEqual(<=)
| GreaterThan(>) | GreaterOrEqual(>=)