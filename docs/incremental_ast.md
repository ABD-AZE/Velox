program = Program(function_definition)

function_definition = Function(identifier name, block body)

block = Block(block_item*)

declaration = Declaration(identifier name, exp? init)

block_item = S(statement) | D(declaration)  

statement = Return(exp)  | Expression(exp) | If(exp condition, statment then, statement? else) | Compound(block) | Null | Goto(label) | Label

exp = Constant(int) | Unary(unary_operator, exp) | Binary(binary_operator, exp, exp) | Var(identifier) | Assignment(exp,exp) | Conditional(exp condition, exp, exp)

unary_operator = Complement | Negate

binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or | Xor | LeftShift | RightShift | Land | Lor
| Equal(==) | NotEqual(!=) | LessThan(<) | LessOrEqual(<=)
| GreaterThan(>) | GreaterOrEqual(>=)