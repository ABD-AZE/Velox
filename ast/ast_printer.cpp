#include "ast_printer.hpp"

<<<<<<< Updated upstream
#include <iostream>
#include <typeinfo>
#include <variant>

    using namespace termcolor;

// ---------- tiny pretty-print helpers ----------
namespace {
inline void indent_spaces(int indent) {
  for (int i = 0; i < indent; ++i)
    std::cout << "  ";
=======
  void ASTPrinter::print(const ASTNodePtr &node, int indent) {
    if (!node) {
      return;
>>>>>>> Stashed changes
    }

    inline void print_open(const char *name, int indent) {
      indent_spaces(indent);
      std::cout << bold << cyan << name << reset << "(" << std::endl;
    }

    inline void print_close(int indent) {
      indent_spaces(indent);
      std::cout << ")" << std::endl;
    }

    inline void print_label(const char *key, int indent) {
      indent_spaces(indent);
      std::cout << blue << key << reset << "=" << std::endl;
    }

    template <class T>
    inline void print_kv(const char *key, const T &value, int indent) {
      indent_spaces(indent);
      std::cout << blue << key << reset << "=" << value << std::endl;
    }

    inline void print_string_kv(const char *key, const std::string &value,
                                int indent) {
      indent_spaces(indent);
      std::cout << blue << key << reset << "=" << yellow << "\"" << value
                << "\"" << reset << std::endl;
    }

    inline void print_simple_named(const char *name, const std::string &value,
                                   int indent) {
      indent_spaces(indent);
      std::cout << bold << cyan << name << reset << "(" << yellow << value
                << reset << ")" << std::endl;
    }
  } // namespace

  // ---------- ASTPrinter methods ----------

  void ASTPrinter::print(const ASTNodePtr &node, int indent) {
    if (!node)
      return;
    ASTPrinter printer(indent);
    node->accept(printer);
  }

  void ASTPrinter::printIndent() {
<<<<<<< Updated upstream
    indent_spaces(indent_);
=======
    for (int i = 0; i < indent_; ++i) {
      std::cout << "  "; // 2 spaces per indent level
    }
>>>>>>> Stashed changes
  }

  void ASTPrinter::visit(ProgramNode & node) {
    print_open("Program", indent_);

<<<<<<< Updated upstream
    if (!node.Declarations.empty()) {
=======
    // Print the function declarations
    if (node.Declarations.size() > 0) {
>>>>>>> Stashed changes
      increaseIndent();
      for (const auto &funcDecl : node.Declarations) {
        funcDecl->accept(*this);
      }
      decreaseIndent();
    }

    print_close(indent_);
  }

  void ASTPrinter::visit(FunctionDefinitionNode & node) {
    print_open("Function", indent_);

    increaseIndent();
    print_string_kv("name", node.name, indent_);

<<<<<<< Updated upstream
    if (node.body) {
=======
    // Print function body
    if (node.body) {
>>>>>>> Stashed changes
      increaseIndent();
      node.body->accept(*this);
      decreaseIndent();
    }

    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(ReturnStatement & node) {
<<<<<<< Updated upstream
    print_open("Return", indent_);
    if (node.expression) {
=======
    printIndent();
    std::cout << "Return(" << std::endl;
    if (node.expression) {
>>>>>>> Stashed changes
      increaseIndent();
      node.expression->accept(*this);
      decreaseIndent();
    }
    print_close(indent_);
  }

  void ASTPrinter::visit(NullStatement &) {
    indent_spaces(indent_);
    std::cout << bold << cyan << "NullStatement" << reset << "()" << std::endl;
  }

  void ASTPrinter::visit(ExpressionStatement & node) {
<<<<<<< Updated upstream
    print_open("ExpressionStatement", indent_);
    if (node.expression) {
=======
    printIndent();
    std::cout << "ExpressionStatement(" << std::endl;
    if (node.expression) {
>>>>>>> Stashed changes
      increaseIndent();
      node.expression->accept(*this);
      decreaseIndent();
    }
    print_close(indent_);
  }

  void ASTPrinter::visit(IfStatement & node) {
    print_open("If", indent_);
    increaseIndent();
<<<<<<< Updated upstream

    print_label("condition", indent_);
    if (node.condition) {
      increaseIndent();
      node.condition->accept(*this);
      decreaseIndent();
    }

    print_label("then", indent_);
    if (node.thenBranch) {
      increaseIndent();
      node.thenBranch->accept(*this);
      decreaseIndent();
    }

    if (node.elseBranch && node.elseBranch.value()) {
      print_label("else", indent_);
      increaseIndent();
      node.elseBranch.value()->accept(*this);
      decreaseIndent();
=======
    printIndent();
    std::cout << "condition=" << std::endl;
    if (node.condition) {
      increaseIndent();
      node.condition->accept(*this);
      decreaseIndent();
    }
    printIndent();
    std::cout << "then=" << std::endl;
    if (node.thenBranch) {
      increaseIndent();
      node.thenBranch->accept(*this);
      decreaseIndent();
    }
    if (node.elseBranch.has_value() && node.elseBranch.value()) {
      printIndent();
      std::cout << "else=" << std::endl;
      increaseIndent();
      node.elseBranch.value()->accept(*this);
      decreaseIndent();
>>>>>>> Stashed changes
    }

    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(GotoStatement & node) {
    print_open("Goto", indent_);
    increaseIndent();
    print_string_kv("label", node.label, indent_);
    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(LabelStatement & node) {
    print_open("Label", indent_);
    increaseIndent();
    print_string_kv("label", node.label, indent_);
    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(CompoundStatement & node) {
<<<<<<< Updated upstream
    print_open("CompoundStatement", indent_);
    if (node.block) {
=======
    printIndent();
    std::cout << "CompoundStatement(" << std::endl;
    if (node.block) {
>>>>>>> Stashed changes
      increaseIndent();
      node.block->accept(*this);
      decreaseIndent();
    }
    print_close(indent_);
  }

  void ASTPrinter::visit(BinaryExpression & node) {
    print_open("Binary", indent_);
    increaseIndent();
<<<<<<< Updated upstream

    print_kv("op", TokenTypeToString(node.op), indent_);

    print_label("left", indent_);
    if (node.left) {
      increaseIndent();
      node.left->accept(*this);
      decreaseIndent();
    }

    print_label("right", indent_);
    if (node.right) {
      increaseIndent();
      node.right->accept(*this);
      decreaseIndent();
    }

=======
    printIndent();
    std::cout << "op=" << TokenTypeToString(node.op) << "," << std::endl;
    printIndent();
    std::cout << "left=" << std::endl;
    if (node.left) {
      increaseIndent();
      node.left->accept(*this);
      decreaseIndent();
    }
    printIndent();
    std::cout << "right=" << std::endl;
    if (node.right) {
      increaseIndent();
      node.right->accept(*this);
      decreaseIndent();
    }
>>>>>>> Stashed changes
    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(UnaryExpression & node) {
    print_open("Unary", indent_);
    increaseIndent();
<<<<<<< Updated upstream

    print_kv("op", TokenTypeToString(node.op), indent_);

    print_label("operand", indent_);
    if (node.operand) {
      increaseIndent();
      node.operand->accept(*this);
      decreaseIndent();
    }

=======
    printIndent();
    std::cout << "op=" << TokenTypeToString(node.op) << "," << std::endl;
    printIndent();
    std::cout << "operand=" << std::endl;
    if (node.operand) {
      increaseIndent();
      node.operand->accept(*this);
      decreaseIndent();
    }
>>>>>>> Stashed changes
    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(ConstantExpression & node) {
    print_open("Constant", indent_);
    increaseIndent();

    indent_spaces(indent_);
    std::visit(
        [&](const auto &value) {
          std::cout << blue << "value" << reset << "=" << magenta << value
                    << reset << ", " << blue << "type" << reset << "=" << cyan
                    << typeid(value).name() << reset << std::endl;
        },
        node.value);

    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(VariableExpression & node) {
    print_simple_named("Variable", node.identifier, indent_);
  }

  void ASTPrinter::visit(AssignmentExpression & node) {
    print_open("Assignment", indent_);
    increaseIndent();
<<<<<<< Updated upstream

    print_label("left", indent_);
    if (node.left) {
      increaseIndent();
      node.left->accept(*this);
      decreaseIndent();
    }

    print_label("right", indent_);
    if (node.right) {
      increaseIndent();
      node.right->accept(*this);
      decreaseIndent();
    }

    // note: if node.type is std::optional<TokenType>, replace condition with:
    // if (node.type.has_value())
    if (node.type) {
      print_label("type", indent_);
=======
    printIndent();
    std::cout << "left=" << std::endl;
    if (node.left) {
      increaseIndent();
      node.left->accept(*this);
      decreaseIndent();
    }
    printIndent();
    std::cout << "right=" << std::endl;
    if (node.right) {
      increaseIndent();
      node.right->accept(*this);
      decreaseIndent();
    }
    if (node.type) {
      printIndent();
      std::cout << "type=" << std::endl;
>>>>>>> Stashed changes
      increaseIndent();
      indent_spaces(indent_);
      std::cout << TokenTypeToString(node.type) << std::endl;
      decreaseIndent();
    }

    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(PostfixExpression & node) {
    print_open("Postfix", indent_);
    increaseIndent();
<<<<<<< Updated upstream

    print_kv("op", TokenTypeToString(node.op), indent_);

    print_label("operand", indent_);
    if (node.operand) {
      increaseIndent();
      node.operand->accept(*this);
      decreaseIndent();
    }

=======
    printIndent();
    std::cout << "op=" << TokenTypeToString(node.op) << "," << std::endl;
    printIndent();
    std::cout << "operand=" << std::endl;
    if (node.operand) {
      increaseIndent();
      node.operand->accept(*this);
      decreaseIndent();
    }
>>>>>>> Stashed changes
    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(ConditionalExpression & node) {
    print_open("Conditional", indent_);
    increaseIndent();
<<<<<<< Updated upstream

    print_label("condition", indent_);
    if (node.condition) {
      increaseIndent();
      node.condition->accept(*this);
      decreaseIndent();
    }

    print_label("trueExp", indent_);
    if (node.trueExpr) {
      increaseIndent();
      node.trueExpr->accept(*this);
      decreaseIndent();
    }

    print_label("falseExp", indent_);
    if (node.falseExpr) {
      increaseIndent();
      node.falseExpr->accept(*this);
      decreaseIndent();
    }

=======
    printIndent();
    std::cout << "condition=" << std::endl;
    if (node.condition) {
      increaseIndent();
      node.condition->accept(*this);
      decreaseIndent();
    }
    printIndent();
    std::cout << "trueExp=" << std::endl;
    if (node.trueExpr) {
      increaseIndent();
      node.trueExpr->accept(*this);
      decreaseIndent();
    }
    printIndent();
    std::cout << "falseExp=" << std::endl;
    if (node.falseExpr) {
      increaseIndent();
      node.falseExpr->accept(*this);
      decreaseIndent();
    }
>>>>>>> Stashed changes
    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(DereferenceExpression & node) {
    print_open("Dereference", indent_);
    increaseIndent();
<<<<<<< Updated upstream

    print_label("pointerExpr", indent_);
    if (node.pointerExpr) {
      increaseIndent();
      node.pointerExpr->accept(*this);
      decreaseIndent();
    }

=======
    printIndent();
    std::cout << "pointerExpr=" << std::endl;
    if (node.pointerExpr) {
      increaseIndent();
      node.pointerExpr->accept(*this);
      decreaseIndent();
    }
>>>>>>> Stashed changes
    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(AddressOfExpression & node) {
    print_open("AddressOf", indent_);
    increaseIndent();
<<<<<<< Updated upstream

    print_label("variableExpr", indent_);
    if (node.variableExpr) {
      increaseIndent();
      node.variableExpr->accept(*this);
      decreaseIndent();
    }

=======
    printIndent();
    std::cout << "variableExpr=" << std::endl;
    if (node.variableExpr) {
      increaseIndent();
      node.variableExpr->accept(*this);
      decreaseIndent();
    }
>>>>>>> Stashed changes
    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(BlockItemNode & node) {
<<<<<<< Updated upstream
    print_open("BlockItem", indent_);
    if (node.block_item) {
=======
    printIndent();
    std::cout << "BlockItem(" << std::endl;

    if (node.block_item) {
>>>>>>> Stashed changes
      increaseIndent();
      node.block_item->accept(*this);
      decreaseIndent();
    }
    print_close(indent_);
  }

  void ASTPrinter::visit(DeclarationNode & node) {
<<<<<<< Updated upstream
    print_open("Declaration", indent_);
    if (node.declaration) {
=======
    printIndent();
    std::cout << "Declaration(" << std::endl;
    if (node.declaration) {
>>>>>>> Stashed changes
      increaseIndent();
      node.declaration->accept(*this);
      decreaseIndent();
    }
    print_close(indent_);
  }

  void ASTPrinter::visit(BlockNode & node) {
    print_open("Block", indent_);

    increaseIndent();
<<<<<<< Updated upstream
    for (const auto &item : node.block_items) {
      if (item)
        item->accept(*this);
=======
    for (const auto &item : node.block_items) {
      if (item) {
        item->accept(*this);
      }
>>>>>>> Stashed changes
    }
    decreaseIndent();

    print_close(indent_);
  }

  void ASTPrinter::visit(InitDecl & node) {
<<<<<<< Updated upstream
    print_open("InitDecl", indent_);
    if (node.init) {
=======
    printIndent();
    std::cout << "InitDecl(" << std::endl;
    if (node.init) {
>>>>>>> Stashed changes
      increaseIndent();
      node.init->accept(*this);
      decreaseIndent();
    }
    print_close(indent_);
  }

  void ASTPrinter::visit(InitExp & node) {
<<<<<<< Updated upstream
    print_open("InitExp", indent_);
    if (node.init) {
=======
    printIndent();
    std::cout << "InitExp(" << std::endl;
    if (node.init) {
>>>>>>> Stashed changes
      increaseIndent();
      node.init.value()->accept(*this);
      decreaseIndent();
    }
    print_close(indent_);
  }

  void ASTPrinter::visit(ForInit & node) {
<<<<<<< Updated upstream
    print_open("ForInit", indent_);
    if (node.init) {
=======
    printIndent();
    std::cout << "ForInit(" << std::endl;
    if (node.init) {
>>>>>>> Stashed changes
      increaseIndent();
      node.init->accept(*this);
      decreaseIndent();
    }
    print_close(indent_);
  }

  void ASTPrinter::visit(BreakNode & node) {
    print_open("Break", indent_);
    increaseIndent();
    print_string_kv("label", node.label, indent_);
    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(ContinueNode & node) {
    print_open("Continue", indent_);
    increaseIndent();
    print_string_kv("label", node.label, indent_);
    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(WhileNode & node) {
    print_open("While", indent_);
    increaseIndent();
<<<<<<< Updated upstream

    print_label("condition", indent_);
    if (node.condition) {
      increaseIndent();
      node.condition->accept(*this);
      decreaseIndent();
    }

    print_label("body", indent_);
    if (node.body) {
      increaseIndent();
      node.body->accept(*this);
      decreaseIndent();
    }

=======
    printIndent();
    std::cout << "condition=" << std::endl;
    if (node.condition) {
      increaseIndent();
      node.condition->accept(*this);
      decreaseIndent();
    }
    printIndent();
    std::cout << "body=" << std::endl;
    if (node.body) {
      increaseIndent();
      node.body->accept(*this);
      decreaseIndent();
    }
>>>>>>> Stashed changes
    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(DoWhileNode & node) {
    print_open("DoWhile", indent_);
    increaseIndent();
<<<<<<< Updated upstream

    print_label("body", indent_);
    if (node.body) {
      increaseIndent();
      node.body->accept(*this);
      decreaseIndent();
    }

    print_label("condition", indent_);
    if (node.condition) {
      increaseIndent();
      node.condition->accept(*this);
      decreaseIndent();
    }

=======
    printIndent();
    std::cout << "body=" << std::endl;
    if (node.body) {
      increaseIndent();
      node.body->accept(*this);
      decreaseIndent();
    }
    printIndent();
    std::cout << "condition=" << std::endl;
    if (node.condition) {
      increaseIndent();
      node.condition->accept(*this);
      decreaseIndent();
    }
>>>>>>> Stashed changes
    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(ForNode & node) {
    print_open("For", indent_);
    increaseIndent();
<<<<<<< Updated upstream

    print_label("init", indent_);
    if (node.init) {
      increaseIndent();
      node.init->accept(*this);
      decreaseIndent();
    }

    print_label("condition", indent_);
    if (node.condition) {
      increaseIndent();
      node.condition.value()->accept(*this);
      decreaseIndent();
    }

    print_label("post", indent_);
    if (node.post) {
      increaseIndent();
      node.post.value()->accept(*this);
      decreaseIndent();
    }

    print_label("body", indent_);
    if (node.body) {
      increaseIndent();
      node.body->accept(*this);
      decreaseIndent();
    }

=======
    printIndent();
    std::cout << "init=" << std::endl;
    if (node.init) {
      increaseIndent();
      node.init->accept(*this);
      decreaseIndent();
    }
    printIndent();
    std::cout << "condition=" << std::endl;
    if (node.condition) {
      increaseIndent();
      node.condition.value()->accept(*this);
      decreaseIndent();
    }
    printIndent();
    std::cout << "post=" << std::endl;
    if (node.post) {
      increaseIndent();
      node.post.value()->accept(*this);
      decreaseIndent();
    }
    printIndent();
    std::cout << "body=" << std::endl;
    if (node.body) {
      increaseIndent();
      node.body->accept(*this);
      decreaseIndent();
    }
>>>>>>> Stashed changes
    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(FunDeclNode & node) {
    print_open("FunDecl", indent_);
    increaseIndent();

<<<<<<< Updated upstream
    print_string_kv("name", node.name, indent_);

    print_label("type", indent_);
    increaseIndent();
    node.type.accept(*this);
    decreaseIndent();

    if (node.storage_class.has_value()) {
      print_kv("storage_class", TokenTypeToString(node.storage_class.value()),
               indent_);
    }

    print_label("parameters", indent_);
    indent_spaces(indent_);
    std::cout << "[" << std::endl;
    if (!node.param_names.empty()) {
      increaseIndent();
      for (const auto &param : node.param_names) {
        print_string_kv("param", param, indent_);
=======
    // Print parameters and storage class specifier if present
    if (node.storage_class.has_value()) {
      printIndent();
      std::cout << "storage_class="
                << TokenTypeToString(node.storage_class.value()) << ","
                << std::endl;
    }
    printIndent();
    std::cout << "parameters=[" << std::endl;
    if (node.param_names.size() > 0) {
      increaseIndent();
      for (const auto &param : node.param_names) {
        printIndent();
        std::cout << "\"" << param << "\"," << std::endl;
>>>>>>> Stashed changes
      }
      decreaseIndent();
    }
    indent_spaces(indent_);
    std::cout << "]," << std::endl;

<<<<<<< Updated upstream
    if (node.body) {
      print_label("body", indent_);
      increaseIndent();
      node.body.value()->accept(*this);
      decreaseIndent();
=======
    // Print body if present
    if (node.body) {
      printIndent();
      std::cout << "body=" << std::endl;
      increaseIndent();
      node.body.value()->accept(*this);
      decreaseIndent();
>>>>>>> Stashed changes
    } else {
      print_kv("body", "null", indent_);
    }

    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(VarDeclNode & node) {
    print_open("VarDecl", indent_);
    increaseIndent();

    print_string_kv("name", node.name, indent_);

    print_label("type", indent_);
    increaseIndent();
    node.type.accept(*this);
    decreaseIndent();

<<<<<<< Updated upstream
    if (node.storage_class.has_value()) {
      print_kv("storage_class", TokenTypeToString(node.storage_class.value()),
               indent_);
    }

    if (node.init.has_value()) {
      print_label("init", indent_);
      increaseIndent();
      node.init.value()->accept(*this);
      decreaseIndent();
=======
    if (node.storage_class.has_value()) {
      printIndent();
      std::cout << "storage_class="
                << TokenTypeToString(node.storage_class.value()) << ","
                << std::endl;
    }
    // Print initializer and storage class specifier if present
    if (node.init.has_value()) {
      printIndent();
      std::cout << "init=" << std::endl;
      increaseIndent();
      node.init.value()->accept(*this);
      decreaseIndent();
    } else {
      std::cout << std::endl;
>>>>>>> Stashed changes
    }

    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(FunctionCallNode & node) {
    print_open("FunctionCall", indent_);
    increaseIndent();

<<<<<<< Updated upstream
    print_string_kv("name", node.name, indent_);

    print_label("arguments", indent_);
    indent_spaces(indent_);
    std::cout << "[" << std::endl;
    if (!node.args.empty()) {
      increaseIndent();
      for (const auto &arg : node.args) {
        if (arg)
          arg->accept(*this);
=======
    // Print arguments
    printIndent();
    std::cout << "arguments=[" << std::endl;
    if (node.args.size() > 0) {
      increaseIndent();
      for (const auto &arg : node.args) {
        arg->accept(*this);
>>>>>>> Stashed changes
      }
      decreaseIndent();
    }
    indent_spaces(indent_);
    std::cout << "]" << std::endl;

    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(Type & node) {
    print_open("Type", indent_);
    increaseIndent();
    print_kv("kind", TypeKindToString(node.kind), indent_);
    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(CastExpression & node) {
    print_open("Cast", indent_);
    increaseIndent();
<<<<<<< Updated upstream

    print_label("targetType", indent_);
    increaseIndent();
    node.targetType.accept(*this);
    decreaseIndent();

    print_label("expression", indent_);
    if (node.expression) {
      increaseIndent();
      node.expression->accept(*this);
      decreaseIndent();
    }

=======
    node.targetType.accept(*this);
    decreaseIndent();
    printIndent();
    std::cout << "expression=" << std::endl;
    if (node.expression) {
      increaseIndent();
      node.expression->accept(*this);
      decreaseIndent();
    }
>>>>>>> Stashed changes
    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(Ident & node) {
    print_open("Identifier", indent_);
    increaseIndent();
    print_string_kv("name", node.identifier, indent_);
    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(DeclaratorNode & node) {
    (void)node;
    indent_spaces(indent_);
    std::cout << bold << cyan << "DeclaratorNode" << reset << "()" << std::endl;
  }

  void ASTPrinter::visit(PointerDeclarator & node) {
    print_open("PointerDeclarator", indent_);
    increaseIndent();
<<<<<<< Updated upstream
    if (node.declarator)
      node.declarator->accept(*this);
=======
    if (node.declarator) {
      node.declarator->accept(*this);
    }
>>>>>>> Stashed changes
    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(FunDeclarator & node) {
    print_open("FunDeclarator", indent_);
    increaseIndent();

    print_label("params", indent_);
    increaseIndent();
<<<<<<< Updated upstream
    for (auto &param : node.params) {
      param.accept(*this);
    }
=======
    for (auto &param : node.params) {
      param.accept(*this);
    }
>>>>>>> Stashed changes
    decreaseIndent();

<<<<<<< Updated upstream
    print_label("declarator", indent_);
    if (node.declarator) {
      increaseIndent();
      node.declarator->accept(*this);
      decreaseIndent();
    }
=======
    printIndent();
    std::cout << "declarator=" << std::endl;
    if (node.declarator) {
      increaseIndent();
      node.declarator->accept(*this);
      decreaseIndent();
    }
>>>>>>> Stashed changes

    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(paraminfo & node) {
    print_open("Parameter", indent_);
    increaseIndent();

<<<<<<< Updated upstream
    print_label("type", indent_);
    increaseIndent();
    node.type.accept(*this);
    decreaseIndent();

    print_label("declarator", indent_);
    if (node.declarator) {
      increaseIndent();
      node.declarator->accept(*this);
      decreaseIndent();
=======
    printIndent();
    std::cout << "declarator=" << std::endl;
    if (node.declarator) {
      increaseIndent();
      node.declarator->accept(*this);
      decreaseIndent();
>>>>>>> Stashed changes
    } else {
      increaseIndent();
      indent_spaces(indent_);
      std::cout << "null" << std::endl;
      decreaseIndent();
    }

    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(AbstractPointer & node) {
    print_open("AbstractPointer", indent_);
    increaseIndent();
<<<<<<< Updated upstream
    if (node.base)
      node.base->accept(*this);
=======
    if (node.base) {
      node.base->accept(*this);
    }
>>>>>>> Stashed changes
    decreaseIndent();
    print_close(indent_);
  }

  void ASTPrinter::visit(AbstractBase & node) {
    (void)node;
    indent_spaces(indent_);
    std::cout << bold << cyan << "AbstractBase" << reset << "()" << std::endl;
  }
