#include "ast_printer.hpp"

#include <iostream>
#include <typeinfo>
#include <variant>

using namespace termcolor;

// ---------- tiny pretty-print helpers ----------
namespace {
inline void indent_spaces(int indent) {
  for (int i = 0; i < indent; ++i)
    std::cout << "  ";
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
  std::cout << blue << key << reset << "=" << yellow << "\"" << value << "\""
            << reset << std::endl;
}

inline void print_simple_named(const char *name, const std::string &value,
                               int indent) {
  indent_spaces(indent);
  std::cout << bold << cyan << name << reset << "(" << yellow << value << reset
            << ")" << std::endl;
}
} // namespace

// ---------- ASTPrinter methods ----------

void ASTPrinter::print(const ASTNodePtr &node, int indent) {
  if (!node)
    return;
  ASTPrinter printer(indent);
  node->accept(printer);
}

void ASTPrinter::printIndent() { indent_spaces(indent_); }

void ASTPrinter::visit(ProgramNode &node) {
  print_open("Program", indent_);

  if (!node.Declarations.empty()) {
    increaseIndent();
    for (const auto &funcDecl : node.Declarations) {
      funcDecl->accept(*this);
    }
    decreaseIndent();
  }

  print_close(indent_);
}

void ASTPrinter::visit(FunctionDefinitionNode &node) {
  print_open("Function", indent_);

  increaseIndent();
  print_string_kv("name", node.name, indent_);

  if (node.body) {
    increaseIndent();
    node.body->accept(*this);
    decreaseIndent();
  }

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(ReturnStatement &node) {
  print_open("Return", indent_);
  if (node.expression) {
    increaseIndent();
    if(node.expression) {
      node.expression->accept(*this);
    }
    decreaseIndent();
  }
  print_close(indent_);
}

void ASTPrinter::visit(NullStatement &) {
  indent_spaces(indent_);
  std::cout << bold << cyan << "NullStatement" << reset << "()" << std::endl;
}

void ASTPrinter::visit(ExpressionStatement &node) {
  print_open("ExpressionStatement", indent_);
  if (node.expression) {
    increaseIndent();
    node.expression->accept(*this);
    decreaseIndent();
  }
  print_close(indent_);
}

void ASTPrinter::visit(IfStatement &node) {
  print_open("If", indent_);
  increaseIndent();

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
  }

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(GotoStatement &node) {
  print_open("Goto", indent_);
  increaseIndent();
  print_string_kv("label", node.label, indent_);
  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(LabelStatement &node) {
  print_open("Label", indent_);
  increaseIndent();
  print_string_kv("label", node.label, indent_);
  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(CompoundStatement &node) {
  print_open("CompoundStatement", indent_);
  if (node.block) {
    increaseIndent();
    node.block->accept(*this);
    decreaseIndent();
  }
  print_close(indent_);
}

void ASTPrinter::visit(BinaryExpression &node) {
  print_open("Binary", indent_);
  increaseIndent();

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

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(UnaryExpression &node) {
  print_open("Unary", indent_);
  increaseIndent();

  print_kv("op", TokenTypeToString(node.op), indent_);

  print_label("operand", indent_);
  if (node.operand) {
    increaseIndent();
    node.operand->accept(*this);
    decreaseIndent();
  }

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(ConstantExpression &node) {
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

void ASTPrinter::visit(VariableExpression &node) {
  print_simple_named("Variable", node.identifier, indent_);
}

void ASTPrinter::visit(AssignmentExpression &node) {
  print_open("Assignment", indent_);
  increaseIndent();

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
  if (node.assignment_type) {
    print_label("type", indent_);
    increaseIndent();
    indent_spaces(indent_);
    std::cout << TokenTypeToString(node.assignment_type) << std::endl;
    decreaseIndent();
  }

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(PostfixExpression &node) {
  print_open("Postfix", indent_);
  increaseIndent();

  print_kv("op", TokenTypeToString(node.op), indent_);

  print_label("operand", indent_);
  if (node.operand) {
    increaseIndent();
    node.operand->accept(*this);
    decreaseIndent();
  }

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(ConditionalExpression &node) {
  print_open("Conditional", indent_);
  increaseIndent();

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

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(DereferenceExpression &node) {
  print_open("Dereference", indent_);
  increaseIndent();

  print_label("pointerExpr", indent_);
  if (node.pointerExpr) {
    increaseIndent();
    node.pointerExpr->accept(*this);
    decreaseIndent();
  }

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(AddressOfExpression &node) {
  print_open("AddressOf", indent_);
  increaseIndent();

  print_label("variableExpr", indent_);
  if (node.variableExpr) {
    increaseIndent();
    node.variableExpr->accept(*this);
    decreaseIndent();
  }

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(SubscriptExpression &node) {
  print_open("Subscript", indent_);
  increaseIndent();

  print_label("arrayExpr", indent_);
  if (node.arrayExpr) {
    increaseIndent();
    node.arrayExpr->accept(*this);
    decreaseIndent();
  }

  print_label("indexExpr", indent_);
  if (node.indexExpr) {
    increaseIndent();
    node.indexExpr->accept(*this);
    decreaseIndent();
  }

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(StringLiteralExpression &node) {
  print_open("StringLiteral", indent_);
  increaseIndent();
  print_string_kv("value", node.value, indent_);
  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(SizeofExpression &node) {
  print_open("SizeofExpression", indent_);
  increaseIndent();
  if (node.expr) {
    print_label("expression", indent_);
    increaseIndent();
    node.expr->accept(*this);
    decreaseIndent();
  }
  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(SizeofTypeExpression &node) {
  print_open("SizeofTypeExpression", indent_);
  increaseIndent();
  print_label("type", indent_);
  increaseIndent();
  node.typeOperand->accept(*this);
  decreaseIndent();
  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(DotExpression &node) {
  print_open("DotExpression", indent_);
  increaseIndent();

  print_label("structExpr", indent_);
  if (node.structExpr) {
    increaseIndent();
    node.structExpr->accept(*this);
    decreaseIndent();
  }

  print_string_kv("memberName", node.memberName, indent_);

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(ArrowExpression &node) {
  print_open("ArrowExpression", indent_);
  increaseIndent();

  print_label("pointerExpr", indent_);
  if (node.pointerExpr) {
    increaseIndent();
    node.pointerExpr->accept(*this);
    decreaseIndent();
  }

  print_string_kv("memberName", node.memberName, indent_);

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(BlockItemNode &node) {
  print_open("BlockItem", indent_);
  if (node.block_item) {
    increaseIndent();
    node.block_item->accept(*this);
    decreaseIndent();
  }
  print_close(indent_);
}

void ASTPrinter::visit(DeclarationNode &node) {
  print_open("Declaration", indent_);
  if (node.declaration) {
    increaseIndent();
    node.declaration->accept(*this);
    decreaseIndent();
  }
  print_close(indent_);
}

void ASTPrinter::visit(BlockNode &node) {
  print_open("Block", indent_);

  increaseIndent();
  for (const auto &item : node.block_items) {
    if (item)
      item->accept(*this);
  }
  decreaseIndent();

  print_close(indent_);
}

void ASTPrinter::visit(InitDecl &node) {
  print_open("InitDecl", indent_);
  if (node.init) {
    increaseIndent();
    node.init->accept(*this);
    decreaseIndent();
  }
  print_close(indent_);
}

void ASTPrinter::visit(InitExp &node) {
  print_open("InitExp", indent_);
  if (node.init) {
    increaseIndent();
    node.init.value()->accept(*this);
    decreaseIndent();
  }
  print_close(indent_);
}

void ASTPrinter::visit(ForInit &node) {
  print_open("ForInit", indent_);
  if (node.init) {
    increaseIndent();
    node.init->accept(*this);
    decreaseIndent();
  }
  print_close(indent_);
}

void ASTPrinter::visit(BreakNode &node) {
  print_open("Break", indent_);
  increaseIndent();
  print_string_kv("label", node.label, indent_);
  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(ContinueNode &node) {
  print_open("Continue", indent_);
  increaseIndent();
  print_string_kv("label", node.label, indent_);
  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(WhileNode &node) {
  print_open("While", indent_);
  increaseIndent();

  print_label("label", indent_);
  increaseIndent();
  print_string_kv("label", node.label, indent_);

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

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(DoWhileNode &node) {
  print_open("DoWhile", indent_);
  increaseIndent();

  print_label("label", indent_);
  increaseIndent();
  print_string_kv("label", node.label, indent_);

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

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(ForNode &node) {
  print_open("For", indent_);
  increaseIndent();

  print_label("label", indent_);
  increaseIndent();
  print_string_kv("label", node.label, indent_);

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

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(ArrayDeclarator &node) {
  print_open("ArrayDeclarator", indent_);
  increaseIndent();

  print_kv("size", node.size, indent_);

  print_label("declarator", indent_);
  if (node.declarator) {
    increaseIndent();
    node.declarator->accept(*this);
    decreaseIndent();
  }

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(FunDeclNode &node) {
  print_open("FunDecl", indent_);
  increaseIndent();

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
    }
    decreaseIndent();
  }
  indent_spaces(indent_);
  std::cout << "]," << std::endl;

  if (node.body) {
    print_label("body", indent_);
    increaseIndent();
    node.body.value()->accept(*this);
    decreaseIndent();
  } else {
    print_kv("body", "null", indent_);
  }

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(InitializerNode &node) {
  print_open("Initializer", indent_);
  increaseIndent();

  print_kv("kind", (node.kind == InitializerKind::SINGLE_INIT) ? "Single" : "Compound", indent_);

  if (node.kind == InitializerKind::SINGLE_INIT) {
    const SingleInit &singleInit = std::get<SingleInit>(node.data);
    print_label("expression", indent_);
    if (singleInit.expression) {
      increaseIndent();
      singleInit.expression->accept(*this);
      decreaseIndent();
    }
  } else if (node.kind == InitializerKind::COMPOUND_INIT) {
    CompoundInit &compoundInit = std::get<CompoundInit>(node.data);
    print_label("initializers", indent_);
    indent_spaces(indent_);
    std::cout << "[" << std::endl;
    if (!compoundInit.initializers.empty()) {
      increaseIndent();
      for (InitializerNode &init : compoundInit.initializers) {
        init.accept(*this);
      }
      decreaseIndent();
    }
    indent_spaces(indent_);
    std::cout << "]" << std::endl;
  }

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(VarDeclNode &node) {
  print_open("VarDecl", indent_);
  increaseIndent();

  print_string_kv("name", node.name, indent_);

  print_label("type", indent_);
  increaseIndent();
  node.type.accept(*this);
  decreaseIndent();

  if (node.storage_class.has_value()) {
    print_kv("storage_class", TokenTypeToString(node.storage_class.value()),
             indent_);
  }

  if (node.init.has_value()) {
    print_label("init", indent_);
    increaseIndent();
    node.init.value()->accept(*this);
    decreaseIndent();
  }

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(StructDeclarationNode &node) {
  print_open("StructDeclaration", indent_);
  increaseIndent();

  print_string_kv("name", node.name, indent_);

  print_label("members", indent_);
  indent_spaces(indent_);
  std::cout << "[" << std::endl;
  if (!node.members.empty()) {
    increaseIndent();
    for (const auto &member : node.members) {
      member->accept(*this);
    }
    decreaseIndent();
  }
  indent_spaces(indent_);
  std::cout << "]" << std::endl;

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(MemberDeclarationNode &node) {
  print_open("MemberDeclaration", indent_);
  increaseIndent();

  print_string_kv("name", node.name, indent_);

  print_label("type", indent_);
  increaseIndent();
  node.type->accept(*this);
  decreaseIndent();

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(FunctionCallNode &node) {
  print_open("FunctionCall", indent_);
  increaseIndent();

  print_string_kv("name", node.name, indent_);

  print_label("arguments", indent_);
  indent_spaces(indent_);
  std::cout << "[" << std::endl;
  if (!node.args.empty()) {
    increaseIndent();
    for (const auto &arg : node.args) {
      if (arg)
        arg->accept(*this);
    }
    decreaseIndent();
  }
  indent_spaces(indent_);
  std::cout << "]" << std::endl;

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(Type &node) {
  print_open("Type", indent_);
  increaseIndent();
  print_kv("kind", TypeKindToString(node.kind), indent_);
  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(CastExpression &node) {
  print_open("Cast", indent_);
  increaseIndent();

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

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(Ident &node) {
  print_open("Identifier", indent_);
  increaseIndent();
  print_string_kv("name", node.identifier, indent_);
  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(DeclaratorNode &node) {
  (void)node;
  indent_spaces(indent_);
  std::cout << bold << cyan << "DeclaratorNode" << reset << "()" << std::endl;
}

void ASTPrinter::visit(PointerDeclarator &node) {
  print_open("PointerDeclarator", indent_);
  increaseIndent();
  if (node.declarator)
    node.declarator->accept(*this);
  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(FunDeclarator &node) {
  print_open("FunDeclarator", indent_);
  increaseIndent();

  print_label("params", indent_);
  increaseIndent();
  for (auto &param : node.params) {
    param.accept(*this);
  }
  decreaseIndent();

  print_label("declarator", indent_);
  if (node.declarator) {
    increaseIndent();
    node.declarator->accept(*this);
    decreaseIndent();
  }

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(paraminfo &node) {
  print_open("Parameter", indent_);
  increaseIndent();

  print_label("type", indent_);
  increaseIndent();
  node.type.accept(*this);
  decreaseIndent();

  print_label("declarator", indent_);
  if (node.declarator) {
    increaseIndent();
    node.declarator->accept(*this);
    decreaseIndent();
  } else {
    increaseIndent();
    indent_spaces(indent_);
    std::cout << "null" << std::endl;
    decreaseIndent();
  }

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(AbstractPointer &node) {
  print_open("AbstractPointer", indent_);
  increaseIndent();
  if (node.base)
    node.base->accept(*this);
  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(AbstractArray &node) {
  print_open("AbstractArray", indent_);
  increaseIndent();

  print_kv("size", node.size, indent_);

  print_label("declarator", indent_);
  if (node.base) {
    increaseIndent();
    node.base->accept(*this);
    decreaseIndent();
  }

  decreaseIndent();
  print_close(indent_);
}

void ASTPrinter::visit(AbstractBase &node) {
  (void)node;
  indent_spaces(indent_);
  std::cout << bold << cyan << "AbstractBase" << reset << "()" << std::endl;
}
