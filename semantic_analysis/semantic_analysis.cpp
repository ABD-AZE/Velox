#include "semantic_analysis.hpp"
/*
// defining foo after its usage won't work because we are doing semantic analysis in a single pass
int foo = 3;
int main(void) {
  int outer = 1;
  int foo = 0;
  if (outer) {
    extern int foo;
    extern int foo;
    return foo;
  }
  return 0;
}


*/
std::unordered_map<std::string, SymbolTableEntry> global_symbol_table;

std::string SemanticAnalyzer::make_temp(const std::string &var_name)
{
  static int counter = 0;
  return "$" + var_name + "." + std::to_string(counter++);
}

std::string SemanticAnalyzer::make_label()
{
  return "label." + std::to_string(label_counter++);
}

bool SemanticAnalyzer::isLvalue(ASTNode *expr)
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

// push the parent scope's variable map onto the stack and reset the current variable map
void SemanticAnalyzer::pushScope()
{
  scope_stack.push_back(identifier_map);
  identifier_map.clear();
}

void SemanticAnalyzer::popScope()
{
  if (!scope_stack.empty())
  {
    identifier_map = scope_stack.back();
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
  if(identifier_map.find(node.name) != identifier_map.end())
  {
    auto prev_entry = identifier_map[node.name];
  }
  // Visit the function declaration
  if (node.body)
  {
    node.body->accept(*this);
  }
}

void SemanticAnalyzer::visit(FunDeclNode &node)
{
  if (inFunctionScope){
    if(node.body){
      success = 0;
      errors.push_back("Nested function definitions are not allowed");
      return;
    }
    if(node.storage_class.has_value() && node.storage_class.value() == StorageClass::STATIC){
      success = 0;
      errors.push_back("Function declarations inside function scope cannot have 'static' storage class");
      return;
    }
  }
  auto temp_inFileScope = inFileScope;
  inFileScope = false;
  auto temp_inFunctionScope = inFunctionScope;
  inFunctionScope = true;
  auto global = (!node.storage_class.has_value())||(node.storage_class.has_value() && node.storage_class.value() != StorageClass::STATIC);
  if(global_symbol_table.find(node.name) != global_symbol_table.end()){
    auto old_decl = global_symbol_table[node.name];
    if(node.type != old_decl.type){
      success = 0;
      errors.push_back("Function '" + node.name + "' redeclared with different type");
      return;
    }
    if(node.body.has_value() && old_decl.initType == InitType::INITIALIZED){
      success = 0;
      errors.push_back("Function '" + node.name + "' already defined");
      return;
    }
    if(old_decl.linkage==LinkageType::EXTERNAL && !global){
      success = 0;
      errors.push_back("Function '" + node.name + "' redeclared with different linkage");
      return;
    }
    InitType new_initType = old_decl.initType;
    if(node.body.has_value()){
      new_initType = InitType::INITIALIZED;
    }
    // no need to update linkage as this declaration would follow the linkage of the first declaration
    global_symbol_table[node.name] = SymbolTableEntry(node.name,SymbolType::FUNCTION,new_initType,node.type);
  }
  else{
    global_symbol_table[node.name] = SymbolTableEntry(node.name, SymbolType::FUNCTION, node.body.has_value() ? InitType::INITIALIZED : InitType::TENTATIVE, node.type);
    global_symbol_table[node.name].linkage = global ? LinkageType::EXTERNAL : LinkageType::INTERNAL;
  }
  // Check for redeclaration
  if (identifier_map.find(node.name) != identifier_map.end())
  {
    auto prev_entry = identifier_map[node.name];
    if(prev_entry.second == 0) // internal linkage
    {
      success = 0;
      errors.push_back("Declaration of function '" + node.name + "' conflicts with previous declaration");
      return;
    }
  }
  identifier_map[node.name] = {node.name,1}; // set linkage to 1 (external) for functions
  // visit params
  pushScope();
  bool hasbody = node.body.has_value();
  for (auto &param_name : node.param_names)
  {
    // generate unique name for parameter
    if(identifier_map.find(param_name) != identifier_map.end())
    {
      success = 0;
      errors.push_back("Parameter '" + param_name + "' redeclared");
      continue;
    }
    std::string uniqueName = make_temp(param_name);
    identifier_map[param_name] = {uniqueName,0}; // default linkage to 0 (internal) for parameters
    if(hasbody){
      global_symbol_table[uniqueName] = SymbolTableEntry(uniqueName,SymbolType::VARIABLE,InitType::TENTATIVE,Type::Int()); // assuming all params are int for simplicity
      global_symbol_table[uniqueName].linkage = LinkageType::NONE; 
      global_symbol_table[uniqueName].storageClass = StorageClass::AUTO;
    }
  }
  isFunctionBlock = true;
  if(node.body)
  {
    node.body.value()->accept(*this);
  }
  isFunctionBlock = false;
  inFunctionScope = temp_inFunctionScope;
  inFileScope = temp_inFileScope;
  popScope();  
}

void SemanticAnalyzer::visit(VarDeclNode &node)
{
  // FILE SCOPE VARIABLE CHECK
  if(inFileScope){
    identifier_map[node.name] = {node.name,1}; // set linkage to 1 (external) for file scope variables 
    auto initType = node.init.has_value() ? InitType::INITIALIZED : node.storage_class == StorageClass::EXTERN ? InitType::TENTATIVE : InitType::UNINITIALIZED;
    auto constInit = node.init.has_value()? dynamic_cast<ConstantExpression*>(node.init.value().get()): nullptr;

    if(initType == InitType::INITIALIZED && !constInit){
      success = 0;
      errors.push_back("File scope variable '" + node.name + "' must be initialized with a constant value");
      return;
    }

    auto global = (!node.storage_class.has_value())||(node.storage_class.has_value() && node.storage_class.value() != StorageClass::STATIC);

    if(global_symbol_table.find(node.name) != global_symbol_table.end()){
      auto old_decl = global_symbol_table[node.name];
      if(node.type != old_decl.type){
        success = 0;
        errors.push_back("Variable '" + node.name + "' redeclared with different type at file scope");
        return;
      }
      if(node.storage_class.has_value() && node.storage_class.value() == StorageClass::EXTERN){
        global = old_decl.linkage == LinkageType::EXTERNAL; 
      }
      if((old_decl.linkage==LinkageType::EXTERNAL && !global) || (old_decl.linkage==LinkageType::INTERNAL && global)){
        success = 0;
        errors.push_back("Variable '" + node.name + "' redeclared with different linkage at file scope");
        return;
      }
      InitType new_initType = old_decl.initType;
      if(old_decl.initType == InitType::INITIALIZED && initType == InitType::INITIALIZED){
        success = 0;
        errors.push_back("Variable '" + node.name + "' already defined at file scope");
        return;
      } else if(initType == InitType::INITIALIZED || old_decl.initType == InitType::INITIALIZED){
        new_initType = InitType::INITIALIZED;
      }
      else if(initType == InitType::TENTATIVE || old_decl.initType == InitType::TENTATIVE){
        new_initType = InitType::TENTATIVE;
      }
      else if(initType == InitType::UNINITIALIZED && old_decl.initType == InitType::UNINITIALIZED){
        new_initType = InitType::UNINITIALIZED;
      } 
      global_symbol_table[node.name] = SymbolTableEntry(node.name,SymbolType::VARIABLE,new_initType,node.type);
      global_symbol_table[node.name].linkage = global ? LinkageType::EXTERNAL : LinkageType::INTERNAL;
      if(initType == InitType::INITIALIZED){
        // update value if initialized
        global_symbol_table[node.name].value = constInit->value;
      } 
    }
    else{
      global_symbol_table[node.name] = SymbolTableEntry(node.name, SymbolType::VARIABLE, initType, node.type);
      global_symbol_table[node.name].linkage = global ? LinkageType::EXTERNAL : LinkageType::INTERNAL;
      if(initType == InitType::INITIALIZED){
        global_symbol_table[node.name].value = constInit->value;
      }
    }
    return;
  }


  // LOCAL SCOPE VARIABLE CHECK
  // Check for redeclaration
  if (forInit && node.storage_class.has_value()) {
    success = 0;
    errors.push_back("Variables declared in for-loop initialization cannot have storage class specifiers");
    return;
  }
  if (identifier_map.find(node.name) != identifier_map.end())
  {
    auto prev_entry = identifier_map[node.name];
    if(!(prev_entry.second && node.storage_class.has_value() && node.storage_class.value() == StorageClass::EXTERN)){ // internal linkage
    success = 0;
    errors.push_back("Variable '" + node.name + "' redeclared");
    return;
    }
  }
  std::string uniqueName;
  if(node.storage_class.has_value() && node.storage_class.value() == StorageClass::EXTERN){
    // extern variable redeclaration
    identifier_map[node.name] = {node.name,1}; // set linkage to 1 (external)
  }
  else{
    // Generate unique name BEFORE resolving initializer
    uniqueName = make_temp(node.name);
    identifier_map[node.name] = {uniqueName, 0}; // default linkage to 0 (internal) for variables
  }
  auto initType = node.init.has_value() ? InitType::INITIALIZED : node.storage_class == StorageClass::EXTERN ? InitType::TENTATIVE : InitType::UNINITIALIZED;

  if(node.storage_class.has_value() && node.storage_class.value() == StorageClass::EXTERN){
    if(initType == InitType::INITIALIZED){
      success = 0;
      errors.push_back("Extern variable '" + node.name + "' cannot be initialized");
      return;
    }
    if(global_symbol_table.find(node.name) != global_symbol_table.end()){
      auto old_decl = global_symbol_table[node.name];
      if(node.type != old_decl.type){
        success = 0;
        errors.push_back("Variable '" + node.name + "' redeclared with different type");
        return;
      }
    }
    else{
      global_symbol_table[node.name] = SymbolTableEntry(node.name,SymbolType::VARIABLE,InitType::UNINITIALIZED,node.type);
      global_symbol_table[node.name].linkage = LinkageType::EXTERNAL;
    }
    return;
  } else if(node.storage_class.has_value() && node.storage_class.value() == StorageClass::STATIC){
    // static variable
    if(initType == InitType::INITIALIZED){
      auto constInit = node.init.has_value()? dynamic_cast<ConstantExpression*>(node.init.value().get()): nullptr;
      if(initType == InitType::INITIALIZED && !constInit){
        success = 0;
        errors.push_back("File scope variable '" + node.name + "' must be initialized with a constant value");
        return;
      }
      global_symbol_table[uniqueName] = SymbolTableEntry(uniqueName, SymbolType::VARIABLE, InitType::INITIALIZED, node.type);
      global_symbol_table[uniqueName].linkage = LinkageType::INTERNAL;
      global_symbol_table[uniqueName].storageClass = StorageClass::STATIC;
      global_symbol_table[uniqueName].value = constInit->value;
      return;
    } else if(initType == InitType::UNINITIALIZED){
      // zero-initialized static variable
      global_symbol_table[uniqueName] = SymbolTableEntry(uniqueName, SymbolType::VARIABLE, InitType::ZERO_INITIALIZED, node.type);
      global_symbol_table[uniqueName].linkage = LinkageType::INTERNAL;
      global_symbol_table[uniqueName].storageClass = StorageClass::STATIC;
      return;
    } else{
      success = 0;
      errors.push_back("non constant initializer on Static variable '" + node.name + "'");
      return;
    }
  } else{
    // automatic variable
    global_symbol_table[uniqueName] = SymbolTableEntry(uniqueName, SymbolType::VARIABLE, initType, node.type);
    global_symbol_table[uniqueName].linkage = LinkageType::NONE;
    global_symbol_table[uniqueName].storageClass = StorageClass::AUTO;
  }
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
  auto temp_isFunctionBlock = isFunctionBlock;
  if(isFunctionBlock)
  {
    isFunctionBlock = false;
  }else{
    pushScope();
  }
  for (auto &item : node.block_items)
  {
    if (item)
    {
      item->accept(*this);
    }
  }
  // popscope if not function block
  if(!temp_isFunctionBlock)
  {
    popScope();
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
  // check if function is declared
  auto found_current = identifier_map.find(node.name);
  if (found_current == identifier_map.end())
  {
    for(auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it)
    {
      auto found = it->find(node.name);
      if (found != it->end())
      {
        found_current = found;
        break;
      }
    }
    if (found_current == identifier_map.end())
    {
      success = 0;
      errors.push_back("Function '" + node.name + "' not declared");
      return;
    }
    else
    {
      // Replace with unique name
      node.name = found_current->second.first;
    }
  }
  else{
    // Replace with unique name
    node.name = found_current->second.first;
  }
  // Resolve all argument expressions
  for (auto &arg : node.args)
  {
    if (arg)
    {
      arg->accept(*this);
    }
  }

  auto entry = global_symbol_table[node.name];
  if(entry.symbolType != SymbolType::FUNCTION){
    success = 0;
    errors.push_back("Variable '" + node.name + "' used as function");
    return;
  }
  if(node.args.size() != (entry.type.data.index() == 1 ? std::get<FunType>(entry.type.data).params.size() : -1)){
    success = 0;
    errors.push_back("Function '" + node.name + "' called with incorrect number of arguments");
    return;
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
  node.label = make_label();
  std::string tmp_label = current_label;
  current_label = node.label;
  if (node.condition)
  {
    node.condition->accept(*this);
  }
  if (node.body)
  {
    node.body->accept(*this);
  }
  current_label = tmp_label;
}

void SemanticAnalyzer::visit(DoWhileNode &node)
{
  node.label = make_label();
  std::string tmp_label = current_label;
  current_label = node.label;
  if (node.condition)
  {
    node.condition->accept(*this);
  }
  if (node.body)
  {
    node.body->accept(*this);
  }
  current_label = tmp_label;
}

void SemanticAnalyzer::visit(ForNode &node)
{
  node.label = make_label();
  std::string tmp_label = current_label;
  current_label = node.label;
  pushScope();
  // Resolve init
  forInit = true;
  if (node.init)
  {
    node.init->accept(*this);
  }
  forInit = false;
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
  current_label = tmp_label;
}

void SemanticAnalyzer::visit(BreakNode &node)
{
  node.label = current_label;
  if(current_label == ""){
    success = 0;
    errors.push_back("Break statement not within a loop");
    return;
  }
}

void SemanticAnalyzer::visit(ContinueNode &node)
{
  node.label = current_label;
  if(current_label == ""){
    success = 0;
    errors.push_back("Continue statement not within a loop");
    return;
  }
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
  auto varnode = dynamic_cast<VarDeclNode*>(node.init.get());
  if (varnode && varnode->storage_class.has_value()){
    success = 0;
    errors.push_back("Storage class specifier not allowed in for-init declaration");
    return;
  }
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

  // check if variable is in current scope
  auto found_current = identifier_map.find(node.identifier);
  if (found_current != identifier_map.end())
  {
    // Replace with unique name
    node.identifier = found_current->second.first;
    auto entry = global_symbol_table[node.identifier];
    if(entry.symbolType == SymbolType::FUNCTION){
      success = 0;
      errors.push_back("Function '" + node.identifier + "' used as variable");
    }
    return;
  }
  // Check if variable is declared in parent scopes
  for(auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it)
  {
    auto found = it->find(node.identifier);
    if (found != it->end())
    {
      // Replace with unique name
      node.identifier = found->second.first;
      auto entry = global_symbol_table[node.identifier];
      if(entry.symbolType == SymbolType::FUNCTION){
        success = 0;
        errors.push_back("Function '" + node.identifier + "' used as variable");
      }
      return;
    }
  }
  success = 0;
  errors.push_back("Variable '" + node.identifier + "' used before declaration");
  return;
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