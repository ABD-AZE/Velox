program = Program(function_definition)

function_definition = Function(identifier name, block_item* body)

declaration = Declaration(identifier name, exp? init)

block_item = S(statement) | D(declaration)  

statement = Return(exp)  | Expression(exp) | Null

exp = Constant(int) | Unary(unary_operator, exp) | Binary(binary_operator, exp, exp) | Var(identifier) | Assignment(exp,exp)

unary_operator = Complement | Negate

binary_operator = Add | Subtract | Multiply | Divide | Remainder | And | Or | Xor | LeftShift | RightShift | Land | Lor
| Equal(==) | NotEqual(!=) | LessThan(<) | LessOrEqual(<=)
| GreaterThan(>) | GreaterOrEqual(>=)