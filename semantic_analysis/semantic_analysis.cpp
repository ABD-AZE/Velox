#include "semantic_analysis.hpp"

std::string make_temp(const std::string &var_name)
{
  static int counter = 0;
  return "$" + var_name + "." + std::to_string(counter++);
}

bool isLvalue(ASTNode *expr)
{
  if (!expr)
    return false;

  // Variable expressions are lvalues
  if (dynamic_cast<VariableExpression *>(expr))
  {
    return true;
  }

  // Dereferenced pointers are lvalues: *ptr
  if (dynamic_cast<DereferenceExpression *>(expr))
  {
    return true;
  }

  // All other expressions are not lvalues
  return false;
}

// Constructor
SemanticAnalyzer::SemanticAnalyzer() : success(true) {}

// Main entry point
void SemanticAnalyzer::analyze(ASTNodePtr &ast)
{
  if (ast)
  {
    ast->accept(*this);
  }
}

void SemanticAnalyzer::pushScope()
{
  scope_stack.push_back(variable_map);
}

void SemanticAnalyzer::popScope()
{
  if (!scope_stack.empty())
  {
    variable_map = scope_stack.back();
    scope_stack.pop_back();
  }
}

// Program and function visitors
void SemanticAnalyzer::visit(ProgramNode &node)
{
  for (auto &decl : node.Declarations)
  {
    if (decl)
    {
      decl->accept(*this);
    }
  }
}

void SemanticAnalyzer::visit(FunctionDefinitionNode &node)
{
  // Visit the function declaration
  if (node.body)
  {
    node.body->accept(*this);
  }

  // Visit the function body with a new scope
  // if (node.body)
  // {
  //   pushScope();
  //   node.body->accept(*this);
  //   popScope();
  // }
}

void SemanticAnalyzer::visit(FunDeclNode &node)
{
  if(node.body)
  {
    node.body.value()->accept(*this);
  }
}

void SemanticAnalyzer::visit(VarDeclNode &node)
{
  // Check for redeclaration
  if (variable_map.find(node.name) != variable_map.end())
  {
    success = 0;
    errors.push_back("Variable '" + node.name + "' redeclared");
    return;
  }

  // Generate unique name BEFORE resolving initializer
  std::string uniqueName = make_temp(node.name);
  variable_map[node.name] = uniqueName;

  // Resolve initializer if present (after adding to map)
  if (node.init)
  {
    node.init.value()->accept(*this);
  }

  // Update node with unique name
  node.name = uniqueName;
}

void SemanticAnalyzer::visit(BlockNode &node)
{
  for (auto &item : node.block_items)
  {
    if (item)
    {
      item->accept(*this);
    }
  }
}

void SemanticAnalyzer::visit(BlockItemNode &node)
{
  if (node.block_item)
  {
    node.block_item->accept(*this);
  }
}

void SemanticAnalyzer::visit(DeclarationNode &node)
{
  // Declarations are handled by their specific types
  (void)node;
}

void SemanticAnalyzer::visit(FunctionCallNode &node)
{
  // Resolve all argument expressions
  for (auto &arg : node.args)
  {
    if (arg)
    {
      arg->accept(*this);
    }
  }
}

// Statement visitors
void SemanticAnalyzer::visit(ReturnStatement &node)
{
  if (node.expression)
  {
    node.expression->accept(*this);
  }
}

void SemanticAnalyzer::visit(ExpressionStatement &node)
{
  if (node.expression)
  {
    node.expression->accept(*this);
  }
}

void SemanticAnalyzer::visit(IfStatement &node)
{
  // Resolve condition
  if (node.condition)
  {
    node.condition->accept(*this);
  }

  // Resolve then branch
  if (node.thenBranch)
  {
    node.thenBranch->accept(*this);
  }

  // Resolve else branch if present
  if (node.elseBranch)
  {
    node.elseBranch.value()->accept(*this);
  }
}

void SemanticAnalyzer::visit(CompoundStatement &node)
{
  if (node.block)
  {
    node.block->accept(*this);
  }
}

void SemanticAnalyzer::visit(NullStatement &node)
{
  (void)node;
}

void SemanticAnalyzer::visit(GotoStatement &node)
{
  (void)node;
}

void SemanticAnalyzer::visit(LabelStatement &node)
{
  (void)node;
}

void SemanticAnalyzer::visit(WhileNode &node)
{
  if (node.condition)
  {
    node.condition->accept(*this);
  }
  if (node.body)
  {
    node.body->accept(*this);
  }
}

void SemanticAnalyzer::visit(DoWhileNode &node)
{
  if (node.body)
  {
    node.body->accept(*this);
  }
  if (node.condition)
  {
    node.condition->accept(*this);
  }
}

void SemanticAnalyzer::visit(ForNode &node)
{
  pushScope();

  // Resolve init
  if (node.init)
  {
    node.init->accept(*this);
  }

  // Resolve condition
  if (node.condition)
  {
    node.condition.value()->accept(*this);
  }

  // Resolve post
  if (node.post)
  {
    node.post.value()->accept(*this);
  }

  // Resolve body
  if (node.body)
  {
    node.body->accept(*this);
  }

  popScope();
}

void SemanticAnalyzer::visit(BreakNode &node)
{
  (void)node;
}

void SemanticAnalyzer::visit(ContinueNode &node)
{
  (void)node;
}

void SemanticAnalyzer::visit(ForInit &node)
{
  if (node.init)
  {
    node.init->accept(*this);
  }
}

void SemanticAnalyzer::visit(InitDecl &node)
{
  if (node.init)
  {
    node.init->accept(*this);
  }
}

void SemanticAnalyzer::visit(InitExp &node)
{
  if (node.init)
  {
    node.init.value()->accept(*this);
  }
}

// Expression visitors
void SemanticAnalyzer::visit(BinaryExpression &node)
{
  if (node.left)
  {
    node.left->accept(*this);
  }
  if (node.right)
  {
    node.right->accept(*this);
  }
}

void SemanticAnalyzer::visit(UnaryExpression &node)
{
  // Check if it's increment/decrement operator
  if (node.op == TokenType::INCREMENT_OPERATOR ||
      node.op == TokenType::DECREMENT_OPERATOR)
  {
    // Check if operand is an lvalue
    if (!isLvalue(node.operand.get()))
    {
      success = 0;
      errors.push_back("Operand of prefix increment/decrement must be an lvalue");
      return;
    }
  }

  if (node.operand)
  {
    node.operand->accept(*this);
  }
}

void SemanticAnalyzer::visit(AssignmentExpression &node)
{
  // Check if left side is an lvalue
  if (!isLvalue(node.left.get()))
  {
    success = 0;
    errors.push_back("Left side of assignment must be an lvalue");
    return;
  }

  if (node.left)
  {
    node.left->accept(*this);
  }
  if (node.right)
  {
    node.right->accept(*this);
  }
}

void SemanticAnalyzer::visit(PostfixExpression &node)
{
  // Check if operand is an lvalue
  if (!isLvalue(node.operand.get()))
  {
    success = 0;

    errors.push_back("Operand of postfix increment/decrement must be an lvalue");
    return;
  }

  if (node.operand)
  {
    node.operand->accept(*this);
  }
}

void SemanticAnalyzer::visit(ConstantExpression &node)
{
  // Constants don't need resolution
  (void)node;
}

void SemanticAnalyzer::visit(VariableExpression &node)
{
  // Check if variable is declared
  auto it = variable_map.find(node.identifier);
  if (it == variable_map.end())
  {
    success = 0;
    errors.push_back("Variable '" + node.identifier + "' used before declaration");
    return;
  }

  // Replace with unique name
  node.identifier = it->second;
}

void SemanticAnalyzer::visit(ConditionalExpression &node)
{
  if (node.condition)
  {
    node.condition->accept(*this);
  }
  if (node.trueExpr)
  {
    node.trueExpr->accept(*this);
  }
  if (node.falseExpr)
  {
    node.falseExpr->accept(*this);
  }
}

void SemanticAnalyzer::visit(CastExpression &node)
{
  if (node.expression)
  {
    node.expression->accept(*this);
  }
}

void SemanticAnalyzer::visit(DereferenceExpression &node)
{
  if (node.pointerExpr)
  {
    node.pointerExpr->accept(*this);
  }
}

void SemanticAnalyzer::visit(AddressOfExpression &node)
{
  // The operand of & should be an lvalue
  if (!isLvalue(node.variableExpr.get()))
  {
    success = 0;
    errors.push_back("Operand of address-of operator must be an lvalue");
    return;
  }

  if (node.variableExpr)
  {
    node.variableExpr->accept(*this);
  }
}

// Type and declarator visitors (no-ops for variable resolution)
void SemanticAnalyzer::visit(Type &node) { (void)node; }
void SemanticAnalyzer::visit(Ident &node) { (void)node; }
void SemanticAnalyzer::visit(DeclaratorNode &node) { (void)node; }
void SemanticAnalyzer::visit(PointerDeclarator &node) { (void)node; }
void SemanticAnalyzer::visit(FunDeclarator &node) { (void)node; }
void SemanticAnalyzer::visit(paraminfo &node) { (void)node; }
void SemanticAnalyzer::visit(AbstractPointer &node) { (void)node; }
void SemanticAnalyzer::visit(AbstractBase &node) { (void)node; }