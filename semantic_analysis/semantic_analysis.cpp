#include "semantic_analysis.hpp"
/*
// defining foo after its usage won't work because we are doing semantic
analysis in a single pass int foo = 3; int main(void) { int outer = 1; int foo =
0; if (outer) { extern int foo; extern int foo; return foo;
  }
  return 0;
}


*/
std::unordered_map<std::string, SymbolTableEntry> global_symbol_table;

bool is_null_pointer_constant(ExpressionNode *expr)
{
  if (!expr)
    return false;

  // Check if it's a constant expression with value 0
  if (auto constExpr = dynamic_cast<ConstantExpression *>(expr))
  {
    return std::visit(
        [](auto&& value)
        {
          using T = std::decay_t<decltype(value)>;
          if (std::is_same_v<T,int>)
          {
            return value == 0;
          }
          else if (std::is_same_v<T,unsigned int>)
          {
            return value == 0u;
          }
          else if  (std::is_same_v<T, long>)
          {
            return value == 0l;
          }
          else if (std::is_same_v<T, unsigned long>)
          {
            return value == 0ul;
          }
          return false;
        },
        constExpr->value);
  }
  return false;
}

PointerType SemanticAnalyzer::getCommonPointerType(ExpressionNode *first,
                                                   ExpressionNode *second)
{
  if (is_null_pointer_constant(first))
  {
    return second->type->kind == TypeKind::POINTER
               ? std::get<PointerType>(second->type->data)
               : PointerType{std::make_shared<Type>(Type::Error())};
  }
  if (is_null_pointer_constant(second))
  {
    return first->type->kind == TypeKind::POINTER
               ? std::get<PointerType>(first->type->data)
               : PointerType{std::make_shared<Type>(Type::Error())};
  }
  if(first->type->kind != TypeKind::POINTER ||
     second->type->kind != TypeKind::POINTER)
  {
    success = 0;
    errors.push_back("Incompatible pointer types");
    return PointerType{std::make_shared<Type>(Type::Error())};
  }
  auto first_ptr_type = std::get<PointerType>(first->type->data);
  auto second_ptr_type = std::get<PointerType>(second->type->data);
  if (*(first_ptr_type.base) == *(second_ptr_type.base))
  {
    return first_ptr_type;
  }
  else
  {
    success = 0;
    errors.push_back("Incompatible pointer types");
    return PointerType{std::make_shared<Type>(Type::Error())};
  }
}

ASTNodePtr SemanticAnalyzer::convertByAssignment(ASTNodePtr exp,
                                            Type &targetType)
{
  auto expr = dynamic_cast<ExpressionNode *>(exp.get());
  if(!expr || !expr->type){
    success = 0;
    errors.push_back("Expression has no type information");
    return exp;
  }
  if(*expr->type == targetType){
    return exp;
  }
  if(expr->type->kind!=TypeKind::ERROR && expr->type->kind != TypeKind::POINTER && expr->type->kind != TypeKind::FUNC && expr->type->kind!= TypeKind::ARRAY && targetType.kind != TypeKind::ERROR && targetType.kind != TypeKind::POINTER && targetType.kind != TypeKind::FUNC && targetType.kind!= TypeKind::ARRAY){
    exp = convertTo(std::move(exp), targetType);
    return exp;
  }
  if(is_null_pointer_constant(expr)&& targetType.kind == TypeKind::POINTER){
    exp = convertTo(std::move(exp), targetType);
    return exp;
  }
  success = 0;
  errors.push_back("Incompatible types in assignment");
  return exp;
}

std::string SemanticAnalyzer::make_temp(const std::string &var_name)
{
  static int counter = 0;
  return "$" + var_name + "." + std::to_string(counter++);
}

std::string SemanticAnalyzer::make_label() {
  return "label." + std::to_string(label_counter++);
}

bool SemanticAnalyzer::isLvalue(ASTNode *expr) {
  if (!expr)
    return false;

  // Variable expressions are lvalues
  if (dynamic_cast<VariableExpression *>(expr)) {
    return true;
  }

  // Dereferenced pointers are lvalues: *ptr
  if (dynamic_cast<DereferenceExpression *>(expr)) {
    return true;
  }

  // All other expressions are not lvalues
  return false;
}

// Helper function to make implicit type conversions explicit
ASTNodePtr SemanticAnalyzer::convertTo(ASTNodePtr expr,
                                              Type &targetType)
{
  auto exp = dynamic_cast<ExpressionNode *>(expr.get());
  if (!exp || !exp->type)
    return expr;

  // If expression already has the correct type, return unchanged
  if (*exp->type == targetType)
    return expr;

  // Create a Cast expression to explicitly convert
  auto castExpr = std::make_unique<CastExpression>();
  castExpr->targetType = targetType;
  castExpr->expression = std::move(expr);
  castExpr->type = std::make_shared<Type>(targetType);
  return castExpr;
}

// Constructor
SemanticAnalyzer::SemanticAnalyzer() : success(true) {}

// Main entry point
void SemanticAnalyzer::analyze(ASTNodePtr &ast) {
  if (ast) {
    ast->accept(*this);
  }
}

// push the parent scope's variable map onto the stack and reset the current
// variable map
void SemanticAnalyzer::pushScope() {
  scope_stack.push_back(identifier_map);
  identifier_map.clear();
}

void SemanticAnalyzer::popScope() {
  if (!scope_stack.empty()) {
    identifier_map = scope_stack.back();
    scope_stack.pop_back();
  }
}

// Program and function visitors
void SemanticAnalyzer::visit(ProgramNode &node) {
  for (auto &decl : node.Declarations) {
    if (decl) {
      decl->accept(*this);
    }
  }
}

void SemanticAnalyzer::visit(FunctionDefinitionNode &node) {
  if (identifier_map.find(node.name) != identifier_map.end()) {
    auto prev_entry = identifier_map[node.name];
  }
  // Visit the function declaration
  if (node.body) {
    node.body->accept(*this);
  }
}

void SemanticAnalyzer::visit(FunDeclNode &node) {
  if (inFunctionScope) {
    if (node.body) {
      success = 0;
      errors.push_back("Nested function definitions are not allowed");
      return;
    }
    if (node.storage_class.has_value() &&
        node.storage_class.value() == StorageClass::STATIC) {
      success = 0;
      errors.push_back("Function declarations inside function scope cannot "
                       "have 'static' storage class");
      return;
    }
  }
  auto temp_inFileScope = inFileScope;
  inFileScope = false;
  auto temp_inFunctionScope = inFunctionScope;
  inFunctionScope = true;
  auto global = (!node.storage_class.has_value()) ||
                (node.storage_class.has_value() &&
                 node.storage_class.value() != StorageClass::STATIC);
  if (global_symbol_table.find(node.name) != global_symbol_table.end()) {
    auto old_decl = global_symbol_table[node.name];
    if (node.type != old_decl.type) {
      success = 0;
      errors.push_back("Function '" + node.name +
                       "' redeclared with different type");
      return;
    }
    if (node.body.has_value() && old_decl.initType == InitType::INITIALIZED) {
      success = 0;
      errors.push_back("Function '" + node.name + "' already defined");
      return;
    }
    if (old_decl.linkage == LinkageType::EXTERNAL && !global) {
      success = 0;
      errors.push_back("Function '" + node.name +
                       "' redeclared with different linkage");
      return;
    }
    InitType new_initType = old_decl.initType;
    if (node.body.has_value()) {
      new_initType = InitType::INITIALIZED;
    }
    // no need to update linkage as this declaration would follow the linkage of
    // the first declaration
    global_symbol_table[node.name] =
        SymbolTableEntry(node.name, SymbolType::FUNCTION, new_initType,
                         node.type, node.param_types);
  } else {
    global_symbol_table[node.name] = SymbolTableEntry(
        node.name, SymbolType::FUNCTION,
        node.body.has_value() ? InitType::INITIALIZED : InitType::TENTATIVE,
        node.type, node.param_types);
    global_symbol_table[node.name].linkage =
        global ? LinkageType::EXTERNAL : LinkageType::INTERNAL;
  }
  // Check for redeclaration
  if (identifier_map.find(node.name) != identifier_map.end()) {
    auto prev_entry = identifier_map[node.name];
    if (prev_entry.second == 0) // internal linkage
    {
      success = 0;
      errors.push_back("Declaration of function '" + node.name +
                       "' conflicts with previous declaration");
      return;
    }
  }
  identifier_map[node.name] = {node.name,
                               1}; // set linkage to 1 (external) for functions
  // visit params
  pushScope();
  bool hasbody = node.body.has_value();
  for (auto &param_name : node.param_names) {
    // generate unique name for parameter
    if (identifier_map.find(param_name) != identifier_map.end()) {
      success = 0;
      errors.push_back("Parameter '" + param_name + "' redeclared");
      continue;
    }
    std::string uniqueName = make_temp(param_name);
    identifier_map[param_name] = {
        uniqueName, 0}; // default linkage to 0 (internal) for parameters
    if (hasbody) {
      Type type;
      // find type from param_types
      auto it = &node.param_types[&param_name - &node.param_names[0]];
      if (it) {
        type = *it;
      }
      global_symbol_table[uniqueName] = SymbolTableEntry(
          uniqueName, SymbolType::VARIABLE, InitType::TENTATIVE, type,
          {}); // assuming all params are int for simplicity
      global_symbol_table[uniqueName].linkage = LinkageType::NONE;
      global_symbol_table[uniqueName].storageClass = StorageClass::AUTO;
    }
  }
  isFunctionBlock = true;
  auto temp_currentFunction = currentFunction;
  currentFunction = &node;

  if (node.body) {
    node.body.value()->accept(*this);
  }
  currentFunction = temp_currentFunction;
  isFunctionBlock = false;
  inFunctionScope = temp_inFunctionScope;
  inFileScope = temp_inFileScope;
  popScope();
}

void SemanticAnalyzer::visit(VarDeclNode &node) {
  // FILE SCOPE VARIABLE CHECK
  if (inFileScope) {
    identifier_map[node.name] = {
        node.name, 1}; // set linkage to 1 (external) for file scope variables
    if(node.init.has_value()){
      node.init.value()->accept(*this);
    }
    auto initType = node.init.has_value() ? InitType::INITIALIZED
                    : node.storage_class == StorageClass::EXTERN
                        ? InitType::TENTATIVE
                        : InitType::UNINITIALIZED;
    auto InitNode =
        node.init.has_value()
            ? dynamic_cast<InitializerNode *>(node.init.value().get())
            : nullptr;
    auto constInit =
        (InitNode && InitNode->kind == InitializerKind::SINGLE_INIT)
            ? dynamic_cast<ConstantExpression *>(
                  std::get<SingleInit>(InitNode->data).expression.get())
            : nullptr;

    if (initType == InitType::INITIALIZED && !constInit) {
      success = 0;
      errors.push_back("File scope variable '" + node.name +
                       "' must be initialized with a constant value");
      return;
    }

    auto global = (!node.storage_class.has_value()) ||
                  (node.storage_class.has_value() &&
                   node.storage_class.value() != StorageClass::STATIC);

    if (global_symbol_table.find(node.name) != global_symbol_table.end()) {
      auto old_decl = global_symbol_table[node.name];
      if (node.type != old_decl.type) {
        success = 0;
        errors.push_back("Variable '" + node.name +
                         "' redeclared with different type at file scope");
        return;
      }
      if (node.storage_class.has_value() &&
          node.storage_class.value() == StorageClass::EXTERN) {
        global = old_decl.linkage == LinkageType::EXTERNAL;
      }
      if ((old_decl.linkage == LinkageType::EXTERNAL && !global) ||
          (old_decl.linkage == LinkageType::INTERNAL && global)) {
        success = 0;
        errors.push_back("Variable '" + node.name +
                         "' redeclared with different linkage at file scope");
        return;
      }
      InitType new_initType = old_decl.initType;
      if (old_decl.initType == InitType::INITIALIZED &&
          initType == InitType::INITIALIZED) {
        success = 0;
        errors.push_back("Variable '" + node.name +
                         "' already defined at file scope");
        return;
      } else if (initType == InitType::INITIALIZED ||
                 old_decl.initType == InitType::INITIALIZED) {
        new_initType = InitType::INITIALIZED;
      } else if (initType == InitType::TENTATIVE ||
                 old_decl.initType == InitType::TENTATIVE) {
        new_initType = InitType::TENTATIVE;
      } else if (initType == InitType::UNINITIALIZED &&
                 old_decl.initType == InitType::UNINITIALIZED) {
        new_initType = InitType::UNINITIALIZED;
      }
      global_symbol_table[node.name] = SymbolTableEntry(
          node.name, SymbolType::VARIABLE, new_initType, node.type);
      global_symbol_table[node.name].linkage =
          global ? LinkageType::EXTERNAL : LinkageType::INTERNAL;
      if (initType == InitType::INITIALIZED) {
        // update value if initialized
        global_symbol_table[node.name].setValue(constInit->value);
        if (global_symbol_table[node.name].type != *constInit->type)
        {
          // extracting the expression from the initializer node and converting it
          std::visit(
              [&](auto&& value)
              {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, SingleInit>)
                {
                  auto expr = std::move(value.expression);
                  auto castExp = convertByAssignment(
                      std::move(expr),
                      global_symbol_table[node.name].type);
                  node.init = std::make_unique<InitializerNode>(
                      std::move(castExp));
                }
                else
                {
                  return;
                }
              },
              InitNode->data);
        }
      }
    } else {
      global_symbol_table[node.name] = SymbolTableEntry(
          node.name, SymbolType::VARIABLE, initType, node.type);
      global_symbol_table[node.name].linkage =
          global ? LinkageType::EXTERNAL : LinkageType::INTERNAL;
      if (initType == InitType::INITIALIZED) {
        global_symbol_table[node.name].setValue(constInit->value);
        if (global_symbol_table[node.name].type != *constInit->type)
        {
          // extracting the expression from the initializer node and converting it
          std::visit(
              [&](auto&& value)
              {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, SingleInit>)
                {
                  auto expr = std::move(value.expression);
                  auto castExp = convertByAssignment(
                      std::move(expr),
                      global_symbol_table[node.name].type);
                  node.init = std::make_unique<InitializerNode>(
                      std::move(castExp));
                }
                else
                {
                  return;
                }
              },
              InitNode->data);
        }
      }
    }
    return;
  }

  // LOCAL SCOPE VARIABLE CHECK
  // Check for redeclaration
  if (forInit && node.storage_class.has_value()) {
    success = 0;
    errors.push_back("Variables declared in for-loop initialization cannot "
                     "have storage class specifiers");
    return;
  }
  if(node.init.has_value()){
    node.init.value()->accept(*this);
  }

  if (identifier_map.find(node.name) != identifier_map.end())
  {
    auto prev_entry = identifier_map[node.name];
    if (!(prev_entry.second && node.storage_class.has_value() &&
          node.storage_class.value() ==
              StorageClass::EXTERN)) { // internal linkage
      success = 0;
      errors.push_back("Variable '" + node.name + "' redeclared");
      return;
    }
  }
  std::string uniqueName;
  if (node.storage_class.has_value() &&
      node.storage_class.value() == StorageClass::EXTERN) {
    // extern variable redeclaration
    identifier_map[node.name] = {node.name, 1}; // set linkage to 1 (external)
  } else {
    // Generate unique name BEFORE resolving initializer
    uniqueName = make_temp(node.name);
    identifier_map[node.name] = {
        uniqueName, 0}; // default linkage to 0 (internal) for variables
  }
  auto initType = node.init.has_value() ? InitType::INITIALIZED
                  : node.storage_class == StorageClass::EXTERN
                      ? InitType::TENTATIVE
                      : InitType::UNINITIALIZED;

  if (node.storage_class.has_value() &&
      node.storage_class.value() == StorageClass::EXTERN) {
    if (initType == InitType::INITIALIZED) {
      success = 0;
      errors.push_back("Extern variable '" + node.name +
                       "' cannot be initialized");
      return;
    }
    if (global_symbol_table.find(node.name) != global_symbol_table.end()) {
      auto old_decl = global_symbol_table[node.name];
      if (node.type != old_decl.type) {
        success = 0;
        errors.push_back("Variable '" + node.name +
                         "' redeclared with different type");
        return;
      }
    } else {
      global_symbol_table[node.name] = SymbolTableEntry(
          node.name, SymbolType::VARIABLE, InitType::UNINITIALIZED, node.type);
      global_symbol_table[node.name].linkage = LinkageType::EXTERNAL;
    }
    return;
  } else if (node.storage_class.has_value() &&
             node.storage_class.value() == StorageClass::STATIC) {
    // static variable
    if (initType == InitType::INITIALIZED) {
      auto InitNode =
          node.init.has_value()
              ? dynamic_cast<InitializerNode *>(node.init.value().get())
              : nullptr;
      auto constInit =
          (InitNode && InitNode->kind == InitializerKind::SINGLE_INIT)
              ? dynamic_cast<ConstantExpression *>(
                    std::get<SingleInit>(InitNode->data).expression.get())
              : nullptr;
      if (initType == InitType::INITIALIZED && !constInit) {
        success = 0;
        errors.push_back("File scope variable '" + node.name +
                         "' must be initialized with a constant value");
        return;
      }
      global_symbol_table[uniqueName] = SymbolTableEntry(
          uniqueName, SymbolType::VARIABLE, InitType::INITIALIZED, node.type);
      global_symbol_table[uniqueName].linkage = LinkageType::INTERNAL;
      global_symbol_table[uniqueName].storageClass = StorageClass::STATIC;
      global_symbol_table[uniqueName].setValue(constInit->value);
      if (global_symbol_table[node.name].type != *constInit->type)
        {
          // extracting the expression from the initializer node and converting it
          std::visit(
              [&](auto&& value)
              {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, SingleInit>)
                {
                  auto expr = std::move(value.expression);
                  auto castExp = convertByAssignment(
                      std::move(expr),
                      global_symbol_table[node.name].type);
                  node.init = std::make_unique<InitializerNode>(
                      std::move(castExp));
                }
                else
                {
                  return;
                }
              },
              InitNode->data);
        }
      return;
    } else if (initType == InitType::UNINITIALIZED) {
      // zero-initialized static variable
      global_symbol_table[uniqueName] =
          SymbolTableEntry(uniqueName, SymbolType::VARIABLE,
                           InitType::ZERO_INITIALIZED, node.type);
      global_symbol_table[uniqueName].linkage = LinkageType::INTERNAL;
      global_symbol_table[uniqueName].storageClass = StorageClass::STATIC;
      return;
    } else {
      success = 0;
      errors.push_back("non constant initializer on Static variable '" +
                       node.name + "'");
      return;
    }
  } else {
    // automatic variable
    global_symbol_table[uniqueName] =
        SymbolTableEntry(uniqueName, SymbolType::VARIABLE, initType, node.type);
    global_symbol_table[uniqueName].linkage = LinkageType::NONE;
    global_symbol_table[uniqueName].storageClass = StorageClass::AUTO;
  }
  // Resolve initializer if present (after adding to map)
  if (node.init)
  {
    // Convert initializer to the variable's type if it's an expression
    if (auto *initnode =
            dynamic_cast<InitializerNode *>(node.init.value().get()))
    {
      std::visit(
          [&](auto &&value)
          {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, SingleInit>)
            {
              auto initExp =
                  dynamic_cast<ExpressionNode *>(value.expression.get());
              // Check if type conversion is needed
              if (initExp->type && *initExp->type != node.type)
              {
                // Create a cast expression to convert to variable's type
                auto castExpr =
                    convertByAssignment(std::move(value.expression), node.type);
                value.expression = std::move(castExpr);
              }
            }
            else if constexpr (std::is_same_v<T, CompoundInit>)
            {
              // Handle compound initializers if needed
              return;
            }
          },
          initnode->data);
    }
  }

  // Update node with unique name
  node.name = uniqueName;
}

void SemanticAnalyzer::visit(StructDeclarationNode &node) {}

void SemanticAnalyzer::visit(MemberDeclarationNode &node) {}

void SemanticAnalyzer::visit(BlockNode &node) {
  auto temp_isFunctionBlock = isFunctionBlock;
  if (isFunctionBlock) {
    isFunctionBlock = false;
  } else {
    pushScope();
  }
  for (auto &item : node.block_items) {
    if (item) {
      item->accept(*this);
    }
  }
  // popscope if not function block
  if (!temp_isFunctionBlock) {
    popScope();
  }
}

void SemanticAnalyzer::visit(BlockItemNode &node) {
  if (node.block_item) {
    node.block_item->accept(*this);
  }
}

void SemanticAnalyzer::visit(DeclarationNode &node) {
  // Declarations are handled by their specific types
  (void)node;
}

void SemanticAnalyzer::visit(FunctionCallNode &node) {
  // check if function is declared
  auto found_current = identifier_map.find(node.name);
  if (found_current == identifier_map.end()) {
    for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
      auto found = it->find(node.name);
      if (found != it->end()) {
        found_current = found;
        break;
      }
    }
    if (found_current == identifier_map.end()) {
      success = 0;
      errors.push_back("Function '" + node.name + "' not declared");
      return;
    } else {
      // Replace with unique name
      node.name = found_current->second.first;
    }
  } else {
    // Replace with unique name
    node.name = found_current->second.first;
  }
  auto entry = global_symbol_table[node.name];
  auto param_types = entry.param_types;
  // Resolve all argument expressions
  std::vector<std::shared_ptr<Type>> arg_types;
  for (size_t i = 0; i < node.args.size(); ++i) {
    auto &arg = node.args[i];
    if (arg) {
      arg->accept(*this);
      auto argExp = dynamic_cast<ExpressionNode *>(arg.get());
      if (argExp && argExp->type) {
        if (i < param_types.size()) {
          Type expectedType = param_types[i];
          // Convert argument to parameter type
          if (*argExp->type != expectedType)
          {
            auto castExpr = convertByAssignment(std::move(arg), expectedType);
            node.args[i] = std::move(castExpr);
          }
          arg_types.push_back(std::make_shared<Type>(expectedType));
        } else {
          arg_types.push_back(argExp->type);
        }
      }
    }
  }
  node.param_types = arg_types;
  std::visit(
      [&](auto value)
      {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, FunType>)
        {
          node.type = value.ret;
        }
        else
        {
          success = 0;
          errors.push_back("Expected function type in function call");
          return;
        }
      },
      entry.type.data);
  if (entry.symbolType != SymbolType::FUNCTION)
  {
    success = 0;
    errors.push_back("Variable '" + node.name + "' used as function");
    return;
  }
  if (node.args.size() !=
      (entry.type.data.index() == 1
           ? std::get<FunType>(entry.type.data).params.size()
           : -1)) {
    success = 0;
    errors.push_back("Function '" + node.name +
                     "' called with incorrect number of arguments");
    return;
  }
}

// Statement visitors
void SemanticAnalyzer::visit(ReturnStatement &node) {
  if (node.expression) {
    node.expression->accept(*this);

    // Convert return value to function's return type
    if (currentFunction && currentFunction->type.data.index() == 1) {
      auto &funType = std::get<FunType>(currentFunction->type.data);
      Type returnType = *funType.ret; // Dereference the shared_ptr

      auto returnExp = dynamic_cast<ExpressionNode *>(node.expression.get());
      if (returnExp && returnExp->type && *returnExp->type != returnType)
      {
        auto castExpr = convertByAssignment(std::move(node.expression), returnType);
        node.expression = std::move(castExpr);
      }
    }
  }
  node.type = std::make_shared<Type>(currentFunction->type);
}

void SemanticAnalyzer::visit(ExpressionStatement &node) {
  if (node.expression) {
    node.expression->accept(*this);
  }
}

void SemanticAnalyzer::visit(IfStatement &node) {
  // Resolve condition
  if (node.condition) {
    node.condition->accept(*this);
  }

  // Resolve then branch
  if (node.thenBranch) {
    node.thenBranch->accept(*this);
  }

  // Resolve else branch if present
  if (node.elseBranch) {
    node.elseBranch.value()->accept(*this);
  }
}

void SemanticAnalyzer::visit(CompoundStatement &node) {
  if (node.block) {
    node.block->accept(*this);
  }
}

void SemanticAnalyzer::visit(NullStatement &node) { (void)node; }

void SemanticAnalyzer::visit(GotoStatement &node) { (void)node; }

void SemanticAnalyzer::visit(LabelStatement &node) { (void)node; }

void SemanticAnalyzer::visit(WhileNode &node) {
  node.label = make_label();
  std::string tmp_label = current_label;
  current_label = node.label;
  if (node.condition) {
    node.condition->accept(*this);
  }
  if (node.body) {
    node.body->accept(*this);
  }
  current_label = tmp_label;
}

void SemanticAnalyzer::visit(DoWhileNode &node) {
  node.label = make_label();
  std::string tmp_label = current_label;
  current_label = node.label;
  if (node.condition) {
    node.condition->accept(*this);
  }
  if (node.body) {
    node.body->accept(*this);
  }
  current_label = tmp_label;
}

void SemanticAnalyzer::visit(ForNode &node) {
  node.label = make_label();
  std::string tmp_label = current_label;
  current_label = node.label;
  pushScope();
  // Resolve init
  forInit = true;
  if (node.init) {
    node.init->accept(*this);
  }
  forInit = false;
  // Resolve condition
  if (node.condition) {
    node.condition.value()->accept(*this);
  }

  // Resolve post
  if (node.post) {
    node.post.value()->accept(*this);
  }
  // Resolve body
  if (node.body) {
    node.body->accept(*this);
  }
  popScope();
  current_label = tmp_label;
}

void SemanticAnalyzer::visit(BreakNode &node) {
  node.label = current_label;
  if (current_label == "") {
    success = 0;
    errors.push_back("Break statement not within a loop");
    return;
  }
}

void SemanticAnalyzer::visit(ContinueNode &node) {
  node.label = current_label;
  if (current_label == "") {
    success = 0;
    errors.push_back("Continue statement not within a loop");
    return;
  }
}

void SemanticAnalyzer::visit(ForInit &node) {
  if (node.init) {
    node.init->accept(*this);
  }
}

void SemanticAnalyzer::visit(InitDecl &node) {
  auto varnode = dynamic_cast<VarDeclNode *>(node.init.get());
  if (varnode && varnode->storage_class.has_value()) {
    success = 0;
    errors.push_back(
        "Storage class specifier not allowed in for-init declaration");
    return;
  }
  if (node.init) {
    node.init->accept(*this);
  }
}

void SemanticAnalyzer::visit(InitExp &node) {
  if (node.init) {
    node.init.value()->accept(*this);
  }
}

// Expression visitors
void SemanticAnalyzer::visit(BinaryExpression &node) {
  if (node.left) {
    node.left->accept(*this);
  }
  if (node.right) {
    node.right->accept(*this);
  }
  auto leftExp = dynamic_cast<ExpressionNode *>(node.left.get());
  auto rightExp = dynamic_cast<ExpressionNode *>(node.right.get());
  
  if(!leftExp || !rightExp || !leftExp->type || !rightExp->type){
    success = 0;
    errors.push_back("Binary expression has invalid operands");
    return;
  }

  if(leftExp->type->kind == TypeKind::POINTER || rightExp->type->kind == TypeKind::POINTER){
    if(node.op != TokenType::PLUS && node.op != TokenType::HYPHEN && node.op != TokenType::EQUAL && node.op != TokenType::NOTEQUAL && node.op != TokenType::LESSTHAN && node.op != TokenType::LESSTHANEQUAL && node.op != TokenType::GREATERTHAN && node.op != TokenType::GREATERTHANEQUAL && node.op != TokenType::LAND && node.op != TokenType::LOR){
      success = 0;
      errors.push_back("Invalid operator for pointer types in binary expression");
      return;
    }
  }

  // Logical AND and OR don't perform type conversions
  if (node.op == TokenType::LAND || node.op == TokenType::LOR) {
    node.type = std::make_shared<Type>(Type::Int());
    return;
  }
  if ((node.op == TokenType::EQUAL || node.op == TokenType::NOTEQUAL) && ((leftExp && leftExp->type->kind == TypeKind::POINTER) || (rightExp && rightExp->type->kind == TypeKind::POINTER)))
  {
    // Handle pointer equality comparisons
    PointerType commonPtrType = getCommonPointerType(leftExp, rightExp);
    Type commonType = Type::Pointer(commonPtrType.base);
    if (*leftExp->type != commonType)
    {
      auto castExpr = convertByAssignment(std::move(node.left), commonType);
      node.left = std::move(castExpr);
    }

    if (*rightExp->type != commonType)
    {
      auto castExpr = convertByAssignment(std::move(node.right), commonType);
      node.right = std::move(castExpr);
    }
    node.type = std::make_shared<Type>(Type::Int());
    return;
  }
  // For other binary operations, perform usual arithmetic conversions
  if (leftExp && rightExp && leftExp->type && rightExp->type) {
    Type commonType = Type::getCommonType(*leftExp->type, *rightExp->type);

    // Convert both operands to the common type by wrapping in cast if needed
    if (*leftExp->type != commonType)
    {
      auto castExpr = convertByAssignment(std::move(node.left), commonType);
      node.left = std::move(castExpr);
    }

    if (*rightExp->type != commonType)
    {
      auto castExpr = convertByAssignment(std::move(node.right), commonType);
      node.right = std::move(castExpr);
    }
    if (node.op == TokenType::PERCENT_SIGN &&
        commonType.kind == TypeKind::DOUBLE) {
      success = 0;
      errors.push_back(
          "Modulo operator '%' cannot be applied to type 'double'");
      return;
    }
    // Determine result type based on operator
    switch (node.op) {
    case TokenType::PLUS:
    case TokenType::HYPHEN:
    case TokenType::ASTERISK:
    case TokenType::FORWARD_SLASH:
    case TokenType::PERCENT_SIGN:
    case TokenType::AAND:
    case TokenType::AOR:
    case TokenType::XOR:
    case TokenType::LEFT_SHIFT:
    case TokenType::RIGHT_SHIFT:
      // Arithmetic operations: result has common type
      node.type = std::make_shared<Type>(commonType);
      break;
    case TokenType::EQUAL:
    case TokenType::NOTEQUAL:
    case TokenType::LESSTHAN:
    case TokenType::LESSTHANEQUAL:
    case TokenType::GREATERTHAN:
    case TokenType::GREATERTHANEQUAL:
      // Comparison operations: result is int
      node.type = std::make_shared<Type>(Type::Int());
      break;
    default:
      node.type = std::make_shared<Type>(commonType);
      break;
    }
  }
}

void SemanticAnalyzer::visit(UnaryExpression &node) {
  // Check if it's increment/decrement operator
  if (node.op == TokenType::INCREMENT_OPERATOR ||
      node.op == TokenType::DECREMENT_OPERATOR) {
    // Check if operand is an lvalue
    if (!isLvalue(node.operand.get())) {
      success = 0;
      errors.push_back(
          "Operand of prefix increment/decrement must be an lvalue");
      return;
    }
  }

  if (node.operand) {
    node.operand->accept(*this);
  }
  auto exp = dynamic_cast<ExpressionNode *>(node.operand.get());
  if (exp) {
    node.type = exp->type;
  }
  if (node.op == TokenType::TILDE && (node.type)->kind == TypeKind::DOUBLE) {
    success = 0;
    errors.push_back(
        "Bitwise NOT operator '~' cannot be applied to type 'double'");
    return;
  }
  if((node.op == TokenType::HYPHEN ||node.op==TokenType::TILDE )&&node.type->kind==TypeKind::POINTER){
    success = 0;
    errors.push_back("Unary '-' and '~' operators cannot be applied to pointer types");
    return;
  }
  switch (node.op)
  {
  case TokenType::NOT:
    node.type =
        std::make_shared<Type>(Type::Int()); // logical NOT results in int
    break;
  default:
    node.type = exp->type;
    break;
  }
}

void SemanticAnalyzer::visit(AssignmentExpression &node) {
  // Check if left side is an lvalue
  if (!isLvalue(node.left.get())) {
    success = 0;
    errors.push_back("Left side of assignment must be an lvalue");
    return;
  }

  auto binexp = std::make_unique<BinaryExpression>();
  if (node.assignment_type != TokenType::ASSIGNMENT) {
    binexp->left = node.left->clone();
    binexp->right = std::move(node.right);
  }
  if (node.left) {
    node.left->accept(*this);
  }
  if (node.right) {
    node.right->accept(*this);
  }

  switch (node.assignment_type) {
  case TokenType::ASSIGNMENT:
    break;
  case TokenType::COMPOUND_SUM: {
    binexp->op = TokenType::PLUS;
    break;
  }
  case TokenType::COMPOUND_DIFFERENCE: {
    binexp->op = TokenType::HYPHEN;
    break;
  }
  case TokenType::COMPOUND_PRODUCT: {
    binexp->op = TokenType::ASTERISK;
    break;
  }
  case TokenType::COMPOUND_DIVISION: {
    binexp->op = TokenType::FORWARD_SLASH;
    break;
  }
  case TokenType::COMPOUND_REMAINDER: {
    binexp->op = TokenType::PERCENT_SIGN;
    break;
  }
  case TokenType::COMPOUND_AND: {
    binexp->op = TokenType::AAND;
    break;
  }
  case TokenType::COMPOUND_OR: {
    binexp->op = TokenType::AOR;
    break;
  }
  case TokenType::COMPOUND_XOR: {
    binexp->op = TokenType::XOR;
    break;
  }
  case TokenType::COMPOUND_LEFTSHIFT: {
    binexp->op = TokenType::LEFT_SHIFT;
    break;
  }
  case TokenType::COMPOUND_RIGHTSHIFT: {
    binexp->op = TokenType::RIGHT_SHIFT;
    break;
  }
  default: {
    success = 0;
    errors.push_back("Unknown assignment operator");
    break;
  }
  }
  if (node.assignment_type != TokenType::ASSIGNMENT)
  {
    binexp->accept(*this);
    node.right = std::move(binexp);
  }
  auto leftExp = dynamic_cast<ExpressionNode *>(node.left.get());
  auto rightExp = dynamic_cast<ExpressionNode *>(node.right.get());
  if (leftExp && leftExp->type) {
    // Result of assignment has the type of the left-hand side
    node.type = leftExp->type;

    if (rightExp && rightExp->type) {
      // Convert right-hand side to the type of left-hand side
      if (*rightExp->type != *leftExp->type)
      {
        auto castExpr = convertByAssignment(std::move(node.right), *leftExp->type);
        node.right = std::move(castExpr);
      }
    }
  }
}

void SemanticAnalyzer::visit(PostfixExpression &node) {
  // Check if operand is an lvalue
  if (!isLvalue(node.operand.get())) {
    success = 0;

    errors.push_back(
        "Operand of postfix increment/decrement must be an lvalue");
    return;
  }

  if (node.operand) {
    node.operand->accept(*this);
  }
  auto exp = dynamic_cast<ExpressionNode *>(node.operand.get());
  if (exp) {
    node.type = exp->type;
  }
}

void SemanticAnalyzer::visit(ConstantExpression &node) {
  std::visit(
      [&](auto value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, int>) {
          node.type = std::make_shared<Type>(Type::Int());
        } else if constexpr (std::is_same_v<T, long>) {
          node.type = std::make_shared<Type>(Type::Long());
        } else if constexpr (std::is_same_v<T, double>) {
          node.type = std::make_shared<Type>(Type::Double());
        } else if constexpr (std::is_same_v<T, unsigned int>) {
          node.type = std::make_shared<Type>(Type::UInt());
        } else if constexpr (std::is_same_v<T, unsigned long>) {
          node.type = std::make_shared<Type>(Type::ULong());
        }
      },
      node.value);
}

void SemanticAnalyzer::visit(VariableExpression &node) {
  // check if variable is in current scope
  auto found_current = identifier_map.find(node.identifier);
  if (found_current != identifier_map.end()) {
    // Replace with unique name
    node.identifier = found_current->second.first;
    auto entry = global_symbol_table[node.identifier];
    if (entry.symbolType == SymbolType::FUNCTION) {
      success = 0;
      errors.push_back("Function '" + node.identifier + "' used as variable");
    }
    node.type = std::make_shared<Type>(entry.type);
    return;
  }
  // Check if variable is declared in parent scopes
  for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
    auto found = it->find(node.identifier);
    if (found != it->end()) {
      // Replace with unique name
      node.identifier = found->second.first;
      auto entry = global_symbol_table[node.identifier];
      if (entry.symbolType == SymbolType::FUNCTION) {
        success = 0;
        errors.push_back("Function '" + node.identifier + "' used as variable");
      }
      node.type = std::make_shared<Type>(entry.type);
      return;
    }
  }
  success = 0;
  errors.push_back("Variable '" + node.identifier +
                   "' used before declaration");
  return;
}

void SemanticAnalyzer::visit(ConditionalExpression &node) {
  if (node.condition) {
    node.condition->accept(*this);
  }
  if (node.trueExpr) {
    node.trueExpr->accept(*this);
  }
  if (node.falseExpr) {
    node.falseExpr->accept(*this);
  }
  auto trueExp = dynamic_cast<ExpressionNode *>(node.trueExpr.get());
  auto falseExp = dynamic_cast<ExpressionNode *>(node.falseExpr.get());
  if (trueExp && falseExp && trueExp->type && falseExp->type)
  {
    if(trueExp->type->kind == TypeKind::POINTER || falseExp->type->kind == TypeKind::POINTER)
    {
      // Handle pointer conditional expressions
      PointerType commonPtrType = getCommonPointerType(trueExp, falseExp);
      Type commonType = Type::Pointer(commonPtrType.base);

      // Convert true expression to common type if needed
      if (*trueExp->type != commonType)
      {
        auto castExpr = std::make_unique<CastExpression>();
        node.trueExpr = std::move(castExpr);
      }

      // Convert false expression to common type if needed
      if (*falseExp->type != commonType)
      {
        auto castExpr = std::make_unique<CastExpression>();
        node.falseExpr = std::move(castExpr);
      }

      node.type = std::make_shared<Type>(commonType);
      return;
    }
    Type commonType = Type::getCommonType(*trueExp->type, *falseExp->type);

    // Convert true expression to common type if needed
    if (*trueExp->type != commonType)
    {
      auto castExpr = convertByAssignment(std::move(node.trueExpr), commonType);
      node.trueExpr = std::move(castExpr);
    }

    // Convert false expression to common type if needed
    if (*falseExp->type != commonType)
    {
      auto castExpr = convertByAssignment(std::move(node.falseExpr), commonType);
      node.falseExpr = std::move(castExpr);
    }

    node.type = std::make_shared<Type>(commonType);
  }
}

void SemanticAnalyzer::visit(CastExpression &node) {
  if (node.expression) {
    node.expression->accept(*this);
  }
  auto exp = dynamic_cast<ExpressionNode *>(node.expression.get());
  if((exp->type->kind == TypeKind::DOUBLE && node.targetType.kind == TypeKind::POINTER)||(exp->type->kind == TypeKind::POINTER && node.targetType.kind == TypeKind::DOUBLE))
  {
    success = 0;
    errors.push_back("Cannot cast from 'double' to pointer type");
    return;
  }
  node.type = std::make_shared<Type>(node.targetType);
}

void SemanticAnalyzer::visit(DereferenceExpression &node) {
  if (node.pointerExpr) {
    node.pointerExpr->accept(*this);
  }
  auto exp = dynamic_cast<ExpressionNode *>(node.pointerExpr.get());
  if (exp && exp->type && exp->type->kind == TypeKind::POINTER)
  {
    auto ptrType = std::get<PointerType>(exp->type->data);
    node.type = ptrType.base;
  }
  else
  {
    success = 0;
    errors.push_back("Operand of dereference operator is not a pointer type");
    return;
  }
}

void SemanticAnalyzer::visit(AddressOfExpression &node) {
  // The operand of & should be an lvalue
  if (!isLvalue(node.variableExpr.get())) {
    success = 0;
    errors.push_back("Operand of address-of operator must be an lvalue");
    return;
  }

  if (node.variableExpr) {
    node.variableExpr->accept(*this);
  }
  if(auto exp = dynamic_cast<DereferenceExpression*>(node.variableExpr.get())){
    auto ptrexp = dynamic_cast<ExpressionNode*>(exp->pointerExpr.get());
    if(ptrexp && ptrexp->type && ptrexp->type->kind == TypeKind::POINTER){
      node.type = ptrexp->type;
      return;
    }
    else{
      success = 0;
      errors.push_back("Operand of address-of operator is not a pointer type");
      return;
    }
  }
  auto type = Type::Pointer((dynamic_cast<ExpressionNode *>(node.variableExpr.get())->type));
  node.type = std::make_shared<Type>(type);
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
void SemanticAnalyzer::visit(ArrayDeclarator &node) { (void)node; }
void SemanticAnalyzer::visit(AbstractArray &node) { (void)node; }
void SemanticAnalyzer::visit(InitializerNode &node)
{
  if (node.kind == InitializerKind::SINGLE_INIT)
  {
    auto &singleInit = std::get<SingleInit>(node.data);
    if (singleInit.expression)
    {
      singleInit.expression->accept(*this);
    }
  }
  else if (node.kind == InitializerKind::COMPOUND_INIT)
  {
    auto &initList = std::get<CompoundInit>(node.data);
    for (auto &init : initList.initializers)
    {
      init.accept(*this);
    }
  }
}
void SemanticAnalyzer::visit(SubscriptExpression &node)
{
  if (node.arrayExpr)
  {
    node.arrayExpr->accept(*this);
  }
  if (node.indexExpr) {
    node.indexExpr->accept(*this);
  }
  auto arrayType = dynamic_cast<ArrayType *>(node.arrayExpr.get());
  // TODO
  if (arrayType) {
    // node.type = arrayType;
  } else {
    success = 0;
    errors.push_back("Operand of subscript operator is not an array type");
    return;
  }
}

void SemanticAnalyzer::visit(StringLiteralExpression &node) {}

void SemanticAnalyzer::visit(SizeofExpression &node) {}

void SemanticAnalyzer::visit(SizeofTypeExpression &node) {}

void SemanticAnalyzer::visit(DotExpression &node) {}

void SemanticAnalyzer::visit(ArrowExpression &node) {}
