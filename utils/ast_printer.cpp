#include "ast_printer.hpp"
#include "ast/ast_variant.hpp"

#include <iostream>
#include <string>
#include <variant>
#include <vector>

namespace velox_astpp {

namespace {

struct Indent { int n; };
static std::ostream& operator<<(std::ostream& os, const Indent& i) {
  for (int k = 0; k < i.n; ++k) os << ' ';
  return os;
}

// Helpers to read values out of std::variant<... unique_ptr<T> ...>
template <typename T, typename Variant>
const T* getPtr(const Variant& v) {
  if (auto p = std::get_if<std::unique_ptr<T>>(&v)) return p->get();
  return nullptr;
}

static std::string constValue(const AST_Node_const* c) {
  if (!c) return "?";
  switch (c->type) {
    case ConstInt:     return std::to_string(c->value.intValue);
    case ConstLong:    return std::to_string(c->value.longValue);
    case ConstUInt:    return std::to_string(c->value.uintValue);
    case ConstULong:   return std::to_string(c->value.ulongValue);
    case ConstDouble:  return std::to_string(c->value.doubleValue);
    case ConstChar:    return std::string(1, c->value.charValue);
    case ConstUChar:   return std::to_string((unsigned)c->value.ucharValue);
  }
  return "?";
}

static const char* binOpStr(BinOpType o) {
  switch (o) {
    case BinOpAdd: return "+";
    case BinOpSub: return "-";
    case BinOpMul: return "*";
    case BinOpDiv: return "/";
    case BinOpMod: return "%";
    case BinOpAnd: return "&";
    case BinOpOr:  return "|";
    case BinOpXor: return "^";
    case BinOpShl: return "<<";
    case BinOpShr: return ">>";
    case LogicalAnd: return "&&";
    case LogicalOr:  return "||";
    case Equal: return "==";
    case NotEqual: return "!=";
    case LessThan: return "<";
    case GreaterThan: return ">";
    case LessEqual: return "<=";
    case GreaterEqual: return ">=";
  }
  return "?";
}

// Forward decls
static void printDecl(const AST_Node_declaration* d, std::ostream& os, int ind);
static void printStmtBody(const AST_Node_statement* s, std::ostream& os, int ind);
static void printStmtInline(const AST_Node_statement* s, std::ostream& os, int ind);
static void printExp(const AST_Node_exp* e, std::ostream& os, int ind);

// ---------------- expressions (minimal) ----------------

static void printExp(const AST_Node_exp* e, std::ostream& os, int ind) {
  if (!e) { os << Indent{ind} << "?\n"; return; }

  switch (e->expr_type) {
    case ExprConstant: {
      auto* c = getPtr<AST_Node_const>(e->info);
      os << Indent{ind} << "Constant(" << constValue(c) << ")\n";
      return;
    }
    case ExprString: {
      auto* s = getPtr<AST_Node_string>(e->info);
      os << Indent{ind} << "String(\"" << (s ? s->value : std::string("?")) << "\")\n";
      return;
    }
    case ExprVar: {
      auto* v = getPtr<AST_Node_var>(e->info);
      os << Indent{ind} << "Var(" << (v ? v->identifier : std::string("?")) << ")\n";
      return;
    }
    case ExprBinary: {
      auto* be = getPtr<AST_Node_binary_exp>(e->info);
      os << Indent{ind} << "Binary(" << (be && be->op ? binOpStr(be->op->op) : "?") << ",\n";
      if (be) {
        printExp(be->left.get(),  os, ind + 2);
        printExp(be->right.get(), os, ind + 2);
      } else {
        os << Indent{ind+2} << "?\n" << Indent{ind+2} << "?\n";
      }
      os << Indent{ind} << ")\n";
      return;
    }
    case ExprAssignment: {
      os << Indent{ind} << "Assign(\n";
      auto* as = getPtr<AST_Node_assignment>(e->info);
      if (as) {
        printExp(as->left.get(),  os, ind + 2);
        printExp(as->right.get(), os, ind + 2);
      }
      os << Indent{ind} << ")\n";
      return;
    }
    case ExprFunctionCall: {
      auto* fc = getPtr<AST_Node_function_call>(e->info);
      os << Indent{ind} << "Call(" << (fc ? fc->identifier : std::string("?")) << ")";
      if (fc && !fc->args.empty()) {
        os << "(\n";
        for (auto& a : fc->args) printExp(a.get(), os, ind + 2);
        os << Indent{ind} << ")";
      }
      os << "\n";
      return;
    }
    // keep the rest very compact
    default:
      os << Indent{ind} << "Expr\n";
      return;
  }
}

// ---------------- statements (minimal) ----------------

static void printStmtInline(const AST_Node_statement* s, std::ostream& os, int ind) {
  // prints: Return(Constant(2))  or: Expr(...)
  if (!s) { os << Indent{ind} << "?\n"; return; }

  switch (s->stmt_type) {
    case StmtReturn: {
      auto* r = getPtr<AST_Node_return_statement>(s->info);
      os << "Return(";
      if (r && r->expr.has_value())
        printExp(r->expr.value().get(), os, 0); // printExp prints newline; we don't want newline here
      else
        os << " ";
      // remove trailing newline if any: simplest is just ensure printExp was on a single line for constants/vars
      // For simplicity we reprint expression in compact way if it was constant/var.
      return;
    }
    case StmtExpression: {
      auto* e = getPtr<AST_Node_expression_statement>(s->info);
      os << "Expr(";
      if (e && e->expr) {
        // print one-liner for common cases
        auto* inner = e->expr.get();
        if (inner && (inner->expr_type == ExprConstant || inner->expr_type == ExprVar)) {
          // inline single-line
          if (inner->expr_type == ExprConstant) {
            auto* c = getPtr<AST_Node_const>(inner->info);
            os << "Constant(" << constValue(c) << ")";
          } else {
            auto* v = getPtr<AST_Node_var>(inner->info);
            os << "Var(" << (v ? v->identifier : std::string("?")) << ")";
          }
        } else {
          os << "\n";
          printExp(inner, os, ind + 2);
          os << Indent{ind};
        }
      }
      os << ")";
      return;
    }
    default:
      os << "Stmt";
      return;
  }
}

static void printStmtBody(const AST_Node_statement* s, std::ostream& os, int ind) {
  if (!s) { os << Indent{ind} << "?\n"; return; }

  switch (s->stmt_type) {
    case StmtReturn: {
      os << "Return(\n";
      auto* r = getPtr<AST_Node_return_statement>(s->info);
      if (r && r->expr.has_value())
        printExp(r->expr.value().get(), os, ind + 2);
      os << Indent{ind} << ")";
      return;
    }
    case StmtExpression: {
      os << "Expr(\n";
      auto* e = getPtr<AST_Node_expression_statement>(s->info);
      if (e && e->expr)
        printExp(e->expr.get(), os, ind + 2);
      os << Indent{ind} << ")";
      return;
    }
    case StmtBlock: {
      os << "Block(";
      // keep blocks compact here; detailed printing is intentionally omitted
      os << ")";
      return;
    }
    default:
      os << "Stmt";
      return;
  }
}

// ---------------- declarations (minimal) ----------------

static void printVarDecl(const AST_Node_variable_declaration* v, std::ostream& os, int ind) {
  if (!v) { os << Indent{ind} << "VarDecl(?)\n"; return; }
  os << Indent{ind} << "VarDecl(name=\"" << v->identifier << "\"";
  if (v->initializer.has_value()) {
    // if initializer holds an expression, show it
    const auto* init = v->initializer.value().get();
    if (auto pe = std::get_if<std::unique_ptr<AST_Node_exp>>(&init->info)) {
      os << ", init=\n";
      printExp(pe->get(), os, ind + 2);
      os << Indent{ind} << ")\n";
      return;
    }
  }
  os << ")\n";
}

static void printFuncDecl(const AST_Node_function_declaration* f, std::ostream& os, int ind) {
  if (!f) { os << Indent{ind} << "Function(?)\n"; return; }

  os << Indent{ind} << "Function(\n";
  os << Indent{ind+2} << "name=\"" << f->identifier << "\"";

  // body=
  if (f->body.has_value()) {
    const auto* b = f->body.value().get();
    // If body has exactly one statement item, print that inline like the example.
    const AST_Node_statement* single = nullptr;
    if (b && b->items.size() == 1 && b->items[0] && b->items[0]->item_type == BlockItemStatement) {
      single = std::get_if<std::unique_ptr<AST_Node_statement>>(&b->items[0]->info) ?
               std::get<std::unique_ptr<AST_Node_statement>>(b->items[0]->info).get() : nullptr;
    }
    if (single) {
      os << ",\n" << Indent{ind+2} << "body=";
      printStmtBody(single, os, ind + 2);
      os << "\n";
    } else {
      os << ",\n" << Indent{ind+2} << "body=Block()\n";
    }
  }
  os << Indent{ind} << ")\n";
}

static void printDecl(const AST_Node_declaration* d, std::ostream& os, int ind) {
  if (!d) { os << Indent{ind} << "?\n"; return; }

  switch (d->decl_type) {
    case DeclFunction: {
      if (auto* f = getPtr<AST_Node_function_declaration>(d->info))
        printFuncDecl(f, os, ind);
      else
        os << Indent{ind} << "Function(?)\n";
      return;
    }
    case DeclVariable: {
      if (auto* v = getPtr<AST_Node_variable_declaration>(d->info))
        printVarDecl(v, os, ind);
      else
        os << Indent{ind} << "VarDecl(?)\n";
      return;
    }
    case DeclStruct: {
      os << Indent{ind} << "Struct\n"; return;
    }
    case DeclUnion: {
      os << Indent{ind} << "Union\n"; return;
    }
    case DeclEnum: {
      os << Indent{ind} << "Enum\n"; return;
    }
    case DeclTypedef: {
      os << Indent{ind} << "Typedef\n"; return;
    }
    case DeclClass: {
      os << Indent{ind} << "Class\n"; return;
    }
  }
}

} // namespace

// ---------------- public entry ----------------

void PrintAST(const AST_Node_program* root, std::ostream& os) {
  if (!root) { os << "Program(?)\n"; return; }
  os << "Program(\n";
  for (auto& d : root->declarations) {
    printDecl(d.get(), os, 2);
  }
  os << ")\n";
}

} // namespace velox_astpp
