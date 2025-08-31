// tools/ast_printer.cpp (enhanced)
#include "ast_printer.hpp"
#include "ast.hpp"

#include <iostream>
#include <string>
#include <variant>
#include <vector>
#include <optional>

namespace velox_astpp {

namespace {

// ----------- Pretty helpers -----------
struct Indent { int n; };
static std::ostream& operator<<(std::ostream& os, const Indent& i) {
  for (int k = 0; k < i.n; ++k) os << ' ';
  return os;
}

// Tweak knobs: set to true if you want function bodies with multiple items to print just "Block()"
// instead of enumerating contents.
constexpr bool kCompactMultiStmtBlocks = false;

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

static const char* assignOpStr(AssignOpType a) {
  switch (a) {
    case Assign:     return "=";
    case AddAssign:  return "+=";
    case SubAssign:  return "-=";
    case MulAssign:  return "*=";
    case DivAssign:  return "/=";
    case ModAssign:  return "%=";
    case AndAssign:  return "&=";
    case OrAssign:   return "|=";
    case XorAssign:  return "^=";
    case ShlAssign:  return "<<=";
    case ShrAssign:  return ">>=";
  }
  return "?=";
}

static const char* typeKindStr(TypeType t) {
  switch (t) {
    case TypeChar: return "Char";
    case TypeSChar: return "SChar";
    case TypeUChar: return "UChar";
    case TypeInt: return "Int";
    case TypeLong: return "Long";
    case TypeUInt: return "UInt";
    case TypeULong: return "ULong";
    case TypeDouble: return "Double";
    case TypeVoid: return "Void";
    case TypeFunction: return "FunType";
    case TypePointer: return "Pointer";
    case TypeArray: return "Array";
    case TypeStructure: return "Structure";
    case TypeUnion: return "Union";
    case TypeEnum: return "Enum";
    case TypeNamedtype: return "NamedType";
    case TypeClass: return "Class";
    case TypeVaList: return "VaList";
  }
  return "?Type";
}

static const char* storageStr(StroageClass sc) {
  switch (sc) {
    case StorageClassNone:   return "None";
    case StorageClassStatic: return "Static";
    case StorageClassExtern: return "Extern";
  }
  return "?SC";
}

static const char* accessStr(AccessSpec a) {
  switch (a) {
    case Public:  return "public";
    case Private: return "private";
  }
  return "?access";
}

// Forward decls
static void printType(const AST_Node_type* t, std::ostream& os, int ind);
static void printArraySize(const AST_Node_array_size* s, std::ostream& os, int ind);
static void printInitializer(const AST_Node_initializer* init, std::ostream& os, int ind);

static void printDecl(const AST_Node_declaration* d, std::ostream& os, int ind);
static void printVarDecl(const AST_Node_variable_declaration* v, std::ostream& os, int ind);
static void printFuncDecl(const AST_Node_function_declaration* f, std::ostream& os, int ind);
static void printStructDecl(const AST_Node_struct_declaration* s, std::ostream& os, int ind);
static void printUnionDecl(const AST_Node_union_declaration* u, std::ostream& os, int ind);
static void printEnumDecl(const AST_Node_enum_declaration* e, std::ostream& os, int ind);
static void printTypedefDecl(const AST_Node_typedef_declaration* tdef, std::ostream& os, int ind);
static void printClassDecl(const AST_Node_class_declaration* c, std::ostream& os, int ind);

static void printLabel(const AST_Node_label* l, std::ostream& os, int ind);

static void printBlock(const AST_Node_block* b, std::ostream& os, int ind);
static void printBlockItem(const AST_Node_block_item* bi, std::ostream& os, int ind);

static void printStmtBody(const AST_Node_statement* s, std::ostream& os, int ind);
[[maybe_unused]] static void printStmtInline(const AST_Node_statement* s, std::ostream& os, int ind);
static void printExp(const AST_Node_exp* e, std::ostream& os, int ind);

// ---------------- types ----------------

static void printType(const AST_Node_type* t, std::ostream& os, int ind) {
  if (!t) { os << Indent{ind} << "Type(?)"; return; }

  // qualifiers
  auto qualifiers = std::string{};
  if (t->is_const)  qualifiers += " const";
  if (t->is_static) qualifiers += " static";
  if (t->is_extern) qualifiers += " extern";

  switch (t->type) {
    case TypePointer: {
      os << Indent{ind} << "Pointer(";
      if (auto p = getPtr<AST_Node_type_pointer>(t->info)) {
        os << "level=" << p->level << ", ref=";
        if (p->referenced) {
          os << "\n";
          printType(p->referenced.get(), os, ind + 2);
          os << "\n" << Indent{ind};
        } else {
          os << "?";
        }
      }
      os << ")";
      return;
    }
    case TypeArray: {
      os << Indent{ind} << "Array(\n";
      if (auto arr = getPtr<AST_Node_type_array>(t->info)) {
        os << Indent{ind+2} << "elem=";
        if (arr->element) {
          printType(arr->element.get(), os, 0);
        } else {
          os << "?";
        }
        os << ",\n" << Indent{ind+2} << "size=";
        if (arr->size) printArraySize(arr->size.get(), os, 0);
        else os << "Unspecified";
        os << "\n";
      }
      os << Indent{ind} << ")";
      return;
    }
    case TypeFunction: {
      os << Indent{ind} << "FunType(\n";
      if (auto ft = getPtr<AST_Node_type_function>(t->info)) {
        os << Indent{ind+2} << "params=[\n";
        for (auto& p : ft->params) {
          if (p) {
            printType(p.get(), os, ind+4);
            os << "\n";
          } else {
            os << Indent{ind+4} << "?\n";
          }
        }
        os << Indent{ind+2} << "],\n";
        os << Indent{ind+2} << "ret=";
        if (ft->ret) printType(ft->ret.get(), os, 0); else os << "?";
        os << "\n";
      }
      os << Indent{ind} << ")";
      return;
    }
    case TypeStructure:
    case TypeUnion:
    case TypeEnum:
    case TypeNamedtype:
    case TypeClass: {
      os << Indent{ind} << typeKindStr(t->type) << "(";
      if (auto tag = std::get_if<std::string>(&t->info)) os << *tag;
      os << ")";
      return;
    }
    default: {
      os << Indent{ind} << typeKindStr(t->type);
      if (!qualifiers.empty()) os << " [" << qualifiers.substr(1) << "]";
      return;
    }
  }
}

static void printArraySize(const AST_Node_array_size* s, std::ostream& os, int ind) {
  if (!s) { os << Indent{ind} << "Unspecified"; return; }
  switch (s->size_type) {
    case ArraySizeConst: {
      if (auto c = getPtr<AST_Node_const>(s->info)) {
        os << "Known(" << constValue(c) << ")";
      } else {
        os << "Known(?)";
      }
      return;
    }
    case ArraySizeVar: {
      os << "VLA(";
      if (auto e = getPtr<AST_Node_exp>(s->info)) {
        os << "\n"; printExp(e, os, ind + 2); os << Indent{ind};
      } else os << "?";
      os << ")";
      return;
    }
    case ArraySizeUnspecified:
      os << "Unspecified"; return;
  }
}

// ---------------- expressions ----------------

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
    case ExprCast: {
      auto* c = getPtr<AST_Node_cast>(e->info);
      os << Indent{ind} << "Cast(\n";
      if (c) {
        os << Indent{ind+2} << "type=";
        if (c->target_type) printType(c->target_type.get(), os, 0); else os << "?";
        os << ",\n";
        os << Indent{ind+2} << "expr=\n";
        printExp(c->expr.get(), os, ind + 4);
      }
      os << Indent{ind} << ")\n";
      return;
    }
    case ExprUnary: {
      auto* u = getPtr<AST_Node_unary_exp>(e->info);
      const char* ustr = "?";
      if (u && u->op) {
        switch (u->op->op) {
          case UnOpNeg:   ustr = "-"; break;
          case UnOpNot:   ustr = "!"; break;
          case UnOpCompl: ustr = "~"; break;
        }
      }
      os << Indent{ind} << "Unary(" << ustr << ",\n";
      if (u) printExp(u->expr.get(), os, ind + 2); else os << Indent{ind+2} << "?\n";
      os << Indent{ind} << ")\n";
      return;
    }
    case ExprPreInc:
    case ExprPreDec:
    case ExprPostInc:
    case ExprPostDec: {
      // These use the "nested expr" variant slot in your AST_Node_exp::info
      auto* inner = getPtr<AST_Node_exp>(e->info);
      const char* kind = (e->expr_type == ExprPreInc)  ? "PreInc"  :
                         (e->expr_type == ExprPreDec)  ? "PreDec"  :
                         (e->expr_type == ExprPostInc) ? "PostInc" : "PostDec";
      os << Indent{ind} << kind << "(\n";
      printExp(inner, os, ind + 2);
      os << Indent{ind} << ")\n";
      return;
    }
    case ExprBinary: {
      auto* be = getPtr<AST_Node_binary_exp>(e->info);
      os << Indent{ind} << "Binary(" << (be && be->op ? binOpStr(be->op->op) : "?") << ", \n";
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
      auto* as = getPtr<AST_Node_assignment>(e->info);
      os << Indent{ind} << "Assign(";
      if (as) os << assignOpStr(as->op); else os << "=";
      os << ",\n";
      if (as) {
        printExp(as->left.get(),  os, ind + 2);
        printExp(as->right.get(), os, ind + 2);
      } else {
        os << Indent{ind+2} << "?\n" << Indent{ind+2} << "?\n";
      }
      os << Indent{ind} << ")\n";
      return;
    }
    case ExprConditional: {
      auto* c = getPtr<AST_Node_conditional>(e->info);
      os << Indent{ind} << "Cond(\n";
      if (c) {
        printExp(c->condition.get(), os, ind + 2);
        printExp(c->true_expr.get(), os, ind + 2);
        printExp(c->false_expr.get(), os, ind + 2);
      }
      os << Indent{ind} << ")\n";
      return;
    }
    case ExprFunctionCall: {
      // NOTE: in your header this is identifier-only; grammar mentions callee=exp which you can support later.
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
    case ExprSubscript: {
      auto* s = getPtr<AST_Node_subscript>(e->info);
      os << Indent{ind} << "Subscript(\n";
      if (s) {
        printExp(s->array.get(), os, ind + 2);
        printExp(s->index.get(), os, ind + 2);
      }
      os << Indent{ind} << ")\n";
      return;
    }
    case ExprSizeof: {
      os << Indent{ind} << "SizeOf(\n";
      if (auto* inner = getPtr<AST_Node_exp>(e->info))
        printExp(inner, os, ind + 2);
      os << Indent{ind} << ")\n";
      return;
    }
    case ExprSizeofType: {
      os << Indent{ind} << "SizeOfT(";
      if (auto* tt = getPtr<AST_Node_type>(e->info)) {
        os << "\n"; printType(tt, os, ind + 2); os << "\n" << Indent{ind};
      } else {
        os << "?";
      }
      os << ")\n";
      return;
    }
    case ExprDot: {
      auto* d = getPtr<AST_Node_dot>(e->info);
      os << Indent{ind} << "Dot(\n";
      if (d) {
        printExp(d->structure.get(), os, ind + 2);
        os << Indent{ind+2} << "member=" << d->member << "\n";
      }
      os << Indent{ind} << ")\n";
      return;
    }
    case ExprArrow: {
      auto* a = getPtr<AST_Node_arrow>(e->info);
      os << Indent{ind} << "Arrow(\n";
      if (a) {
        printExp(a->pointer.get(), os, ind + 2);
        os << Indent{ind+2} << "member=" << a->member << "\n";
      }
      os << Indent{ind} << ")\n";
      return;
    }
    default:
      // Varargs & builtins in your AST:
      if (auto* vs = getPtr<AST_Node_VaStart>(e->info)) {
        os << Indent{ind} << "VaStart(last=" << vs->last_param << ",\n";
        printExp(vs->ap.get(), os, ind + 2);
        os << Indent{ind} << ")\n";
        return;
      }
      if (auto* ve = getPtr<AST_Node_VaEnd>(e->info)) {
        os << Indent{ind} << "VaEnd(\n";
        printExp(ve->ap.get(), os, ind + 2);
        os << Indent{ind} << ")\n";
        return;
      }
      if (auto* va = getPtr<AST_Node_VaArg>(e->info)) {
        os << Indent{ind} << "VaArg(";
        if (va->target_type) { os << "\n"; printType(va->target_type.get(), os, ind + 2); os << "\n" << Indent{ind}; }
        os << ",\n";
        printExp(va->ap.get(), os, ind + 2);
        os << Indent{ind} << ")\n";
        return;
      }
      os << Indent{ind} << "Expr\n";
      return;
  }
}

// ---------------- statements & blocks ----------------

static void printStmtInline(const AST_Node_statement* s, std::ostream& os, int ind) {
  if (!s) { os << Indent{ind} << "?\n"; return; }

  switch (s->stmt_type) {
    case StmtReturn: {
      auto* r = getPtr<AST_Node_return_statement>(s->info);
      os << "Return(";
      if (r && r->expr.has_value())
        printExp(r->expr.value().get(), os, 0);
      os << ")";
      return;
    }
    case StmtExpression: {
      auto* e = getPtr<AST_Node_expression_statement>(s->info);
      os << "Expr(";
      if (e && e->expr) {
        // Inline constants/vars; multiline otherwise
        auto* inner = e->expr.get();
        if (inner && (inner->expr_type == ExprConstant || inner->expr_type == ExprVar)) {
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

static void printLabel(const AST_Node_label* l, std::ostream& os, int ind) {
  if (!l) { os << Indent{ind} << "Label(?)\n"; return; }
  switch (l->label_type) {
    case LabelIdentifier: {
      if (auto s = std::get_if<std::string>(&l->info)) {
        os << Indent{ind} << "UserLabel(" << *s << ")\n";
      } else {
        os << Indent{ind} << "UserLabel(?)\n";
      }
      return;
    }
    case LabelCase: {
      if (auto c = getPtr<AST_Node_const>(l->info)) {
        os << Indent{ind} << "CaseLabel(" << constValue(c) << ")\n";
      } else {
        os << Indent{ind} << "CaseLabel(?)\n";
      }
      return;
    }
    case LabelDefault:
      os << Indent{ind} << "DefaultLabel\n"; return;
  }
}

static void printBlock(const AST_Node_block* b, std::ostream& os, int ind) {
  if (!b) { os << Indent{ind} << "Block(?)"; return; }
  os << "Block(\n";
  for (const auto& it : b->items) {
    printBlockItem(it.get(), os, ind + 2);
  }
  os << Indent{ind} << ")";
}

static void printBlockItem(const AST_Node_block_item* bi, std::ostream& os, int ind) {
  if (!bi) { os << Indent{ind} << "?\n"; return; }
  if (bi->item_type == BlockItemDeclaration) {
    if (auto* d = std::get_if<std::unique_ptr<AST_Node_declaration>>(&bi->info)) {
      printDecl(d->get(), os, ind);
    } else {
      os << Indent{ind} << "Decl\n";
    }
  } else {
    if (auto* s = std::get_if<std::unique_ptr<AST_Node_statement>>(&bi->info)) {
      os << Indent{ind};
      printStmtBody(s->get(), os, ind);
      os << "\n";
    } else {
      os << Indent{ind} << "Stmt\n";
    }
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
    case StmtIf: {
      auto* iff = getPtr<AST_Node_if_statement>(s->info);
      os << "If(\n";
      if (iff) {
        printExp(iff->condition.get(), os, ind + 2);
        os << Indent{ind+2};
        printStmtBody(iff->true_statement.get(), os, ind + 2);
        if (iff->false_statement.has_value()) {
          os << "\n" << Indent{ind+2};
          printStmtBody(iff->false_statement.value().get(), os, ind + 2);
        } else {
          os << "\n" << Indent{ind+2} << "Else(None)";
        }
      }
      os << "\n" << Indent{ind} << ")";
      return;
    }
    case StmtWhile: {
      auto* w = getPtr<AST_Node_while_statement>(s->info);
      os << "While(\n";
      if (w) {
        printExp(w->condition.get(), os, ind + 2);
        os << Indent{ind+2};
        printStmtBody(w->body.get(), os, ind + 2);
      }
      os << "\n" << Indent{ind} << ")";
      return;
    }
    case StmtDoWhile: {
      auto* d = getPtr<AST_Node_do_while_statement>(s->info);
      os << "DoWhile(\n";
      if (d) {
        os << Indent{ind+2};
        printStmtBody(d->body.get(), os, ind + 2);
        os << "\n";
        printExp(d->condition.get(), os, ind + 2);
      }
      os << Indent{ind} << ")";
      return;
    }
    case StmtFor: {
      auto* f = getPtr<AST_Node_for_statement>(s->info);
      os << "For(\n";
      if (f) {
        // init
        os << Indent{ind+2} << "init=";
        if (f->init) {
          auto* fi = f->init.get();
          switch (fi->init_type) {
            case InitExpression: {
              os << "InitExp(\n";
              if (auto* e = getPtr<AST_Node_expression_statement>(fi->info)) {
                if (e && e->expr) printExp(e->expr.get(), os, ind + 4);
              }
              os << Indent{ind+2} << ")";
              break;
            }
            case InitDeclaration: {
              os << "InitDecl(\n";
              if (auto* d = getPtr<AST_Node_declaration>(fi->info)) {
                printDecl(d, os, ind + 4);
                // printDecl already ends with newline
              }
              os << Indent{ind+2} << ")";
              break;
            }
            default: os << "?";
          }
        } else os << "None";
        os << ",\n";
        // condition
        os << Indent{ind+2} << "cond=";
        if (f->condition.has_value() && f->condition.value()) {
          os << "\n"; printExp(f->condition.value()->expr.get(), os, ind+4);
          os << Indent{ind+2};
        } else os << "None";
        os << ",\n";
        // increment
        os << Indent{ind+2} << "post=";
        if (f->increment.has_value() && f->increment.value()) {
          os << "\n"; printExp(f->increment.value()->expr.get(), os, ind+4);
          os << Indent{ind+2};
        } else os << "None";
        os << ",\n";
        // body
        os << Indent{ind+2} << "body=";
        if (f->body) { printStmtBody(f->body.get(), os, ind + 2); }
        else os << "None";
      }
      os << "\n" << Indent{ind} << ")";
      return;
    }
    case StmtSwitch: {
      auto* sw = getPtr<AST_Node_switch_statement>(s->info);
      os << "Switch(\n";
      if (sw) {
        printExp(sw->selector.get(), os, ind + 2);
        os << Indent{ind+2};
        printStmtBody(sw->body.get(), os, ind + 2);
      }
      os << "\n" << Indent{ind} << ")";
      return;
    }
    case StmtBlock: {
      os << "Block(";
      // expand the block unless compact mode requested
      if (auto* b = getPtr<AST_Node_block>(s->info)) {
        if (kCompactMultiStmtBlocks) {
          os << ")";
          return;
        }
        os << "\n";
        for (auto& it : b->items) printBlockItem(it.get(), os, ind + 2);
        os << Indent{ind} << ")";
        return;
      }
      os << ")";
      return;
    }
    case StmtBreak:    os << "Break";    return;
    case StmtContinue: os << "Continue"; return;
    case StmtNull:     os << "Null";     return;
    case StmtLabeled: {
      auto* lab = getPtr<AST_Node_labeled_statement>(s->info);
      os << "Labeled(\n";
      if (lab) {
        // Your AST stores vector<string> not AST_Node_label; we print them raw
        os << Indent{ind+2} << "labels=[";
        for (size_t i=0;i<lab->label.size();++i) {
          if (i) os << ", ";
          os << lab->label[i];
        }
        os << "],\n";
        os << Indent{ind+2};
        printStmtBody(lab->statement.get(), os, ind + 2);
      }
      os << "\n" << Indent{ind} << ")";
      return;
    }
    case StmtGoto: {
      if (auto s_lbl = std::get_if<std::string>(&const_cast<AST_Node_statement*>(s)->info)) {
        os << "Goto(" << *s_lbl << ")";
      } else {
        os << "Goto(?)";
      }
      return;
    }
  }
}

// ---------------- initializers ----------------

static void printInitializer(const AST_Node_initializer* init, std::ostream& os, int ind) {
  if (!init) { os << Indent{ind} << "Init(?)"; return; }
  switch (init->init_type) {
    case InitializerExp: {
      if (auto e = getPtr<AST_Node_exp>(init->info)) {
        os << "\n"; printExp(e, os, ind + 2); os << Indent{ind};
      } else {
        os << " Expr(?)";
      }
      return;
    }
    case InitializerList: {
      os << " { \n";
      if (auto lst = std::get_if<std::vector<std::unique_ptr<AST_Node_initializer>>>(&init->info)) {
        for (auto& sub : *lst) {
          printInitializer(sub.get(), os, ind + 2);
          os << "\n";
        }
      }
      os << Indent{ind} << "}";
      return;
    }
    case InitializerCtor: {
      // Your AST uses InitializerCtor type name but info is vector<initializer> or exp? (in this header it's vector of inits)
      os << " Ctor(";
      if (auto lst = std::get_if<std::vector<std::unique_ptr<AST_Node_initializer>>>(&init->info)) {
        os << "\n";
        for (auto& sub : *lst) { printInitializer(sub.get(), os, ind + 2); os << "\n"; }
        os << Indent{ind};
      }
      os << ")";
      return;
  }
  }
}

// ---------------- declarations ----------------

static void printVarDecl(const AST_Node_variable_declaration* v, std::ostream& os, int ind) {
  if (!v) { os << Indent{ind} << "VarDecl(?)\n"; return; }
  os << Indent{ind} << "VarDecl(name=\"" << v->identifier << "\"";
  if (v->var_type) {
    os << ", type=";
    printType(v->var_type.get(), os, 0);
  }
  if (v->initializer.has_value()) {
    const auto* init = v->initializer.value().get();
    os << ", init=";
    printInitializer(init, os, ind + 2);
  }
  if (v->storage_class != StorageClassNone) {
    os << ", storage=" << storageStr(v->storage_class);
  }
  os << ")\n";
}

static void printFuncDecl(const AST_Node_function_declaration* f, std::ostream& os, int ind) {
  if (!f) { os << Indent{ind} << "Function(?)\n"; return; }

  os << Indent{ind} << "Function(\n";
  os << Indent{ind+2} << "name=\"" << f->identifier << "\"";

  // params (names only in this AST)
  if (!f->params.empty()) {
    os << ",\n" << Indent{ind+2} << "params=[";
    for (size_t i=0;i<f->params.size();++i) {
      if (i) os << ", ";
      os << f->params[i];
    }
    os << "]";
  }

  // type
  if (f->fun_type) {
    os << ",\n" << Indent{ind+2} << "type=";
    printType(f->fun_type.get(), os, 0);
  }

  // storage
  if (f->storage_class != StorageClassNone) {
    os << ",\n" << Indent{ind+2} << "storage=" << storageStr(f->storage_class);
  }

  // body
  if (f->body.has_value()) {
    const auto* b = f->body.value().get();

    // If body has exactly one statement item and it's a statement, print inline pretty form
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
      os << ",\n" << Indent{ind+2} << "body=";
      if (kCompactMultiStmtBlocks) {
        os << "Block()\n";
      } else {
        printBlock(b, os, ind + 2);
        os << "\n";
      }
    }
  }
  os << Indent{ind} << ")\n";
}

static void printStructDecl(const AST_Node_struct_declaration* s, std::ostream& os, int ind) {
  if (!s) { os << Indent{ind} << "Struct\n"; return; }
  os << Indent{ind} << "Struct(";
  if (!s->tag.empty()) os << "name=\"" << s->tag << "\"";
  if (!s->members.empty()) {
    os << (s->tag.empty() ? "" : ",") << " members=[\n";
    for (const auto& m : s->members) {
      if (!m) { os << Indent{ind+2} << "?\n"; continue; }
      os << Indent{ind+2} << "Field(name=\"" << m->identifier << "\"";
      if (m->member_type) {
        os << ", type=";
        printType(m->member_type.get(), os, 0);
      }
      os << ")\n";
    }
    os << Indent{ind} << "]";
  }
  os << ")\n";
}

static void printUnionDecl(const AST_Node_union_declaration* u, std::ostream& os, int ind) {
  if (!u) { os << Indent{ind} << "Union\n"; return; }
  os << Indent{ind} << "Union(";
  if (!u->tag.empty()) os << "name=\"" << u->tag << "\"";
  if (!u->members.empty()) {
    os << (u->tag.empty() ? "" : ",") << " members=[\n";
    for (const auto& m : u->members) {
      if (!m) { os << Indent{ind+2} << "?\n"; continue; }
      os << Indent{ind+2} << "Field(name=\"" << m->identifier << "\"";
      if (m->member_type) {
        os << ", type=";
        printType(m->member_type.get(), os, 0);
      }
      os << ")\n";
    }
    os << Indent{ind} << "]";
  }
  os << ")\n";
}

static void printEnumDecl(const AST_Node_enum_declaration* e, std::ostream& os, int ind) {
  if (!e) { os << Indent{ind} << "Enum\n"; return; }
  os << Indent{ind} << "Enum(";
  if (e->tag.has_value()) os << "name=\"" << *e->tag << "\"";
  if (!e->enumerators.empty()) {
    os << (e->tag.has_value() ? ", " : "") << "values=[\n";
    for (const auto& en : e->enumerators) {
      if (!en) { os << Indent{ind+2} << "?\n"; continue; }
      os << Indent{ind+2} << en->name;
      if (en->value.has_value()) {
        auto* c = en->value.value().get();
        os << " = ";
        if (c) os << constValue(c); else os << "?";
      }
      os << "\n";
    }
    os << Indent{ind} << "]";
  }
  os << ")\n";
}

static void printTypedefDecl(const AST_Node_typedef_declaration* tdef, std::ostream& os, int ind) {
  if (!tdef) { os << Indent{ind} << "Typedef\n"; return; }
  os << Indent{ind} << "Typedef([\n";
  for (const auto& e : tdef->entries) {
    if (!e) { os << Indent{ind+2} << "?\n"; continue; }
    os << Indent{ind+2} << "name=" << e->name << ", type=";
    if (e->alias_type) printType(e->alias_type.get(), os, 0); else os << "?";
    os << "\n";
  }
  os << Indent{ind} << "])\n";
}

static void printClassDecl(const AST_Node_class_declaration* c, std::ostream& os, int ind) {
  if (!c) { os << Indent{ind} << "Class\n"; return; }
  os << Indent{ind} << "Class(name=\"" << c->name << "\"";
  if (!c->members.empty()) {
    os << ", members=[\n";
    for (const auto& m : c->members) {
      if (!m) { os << Indent{ind+2} << "?\n"; continue; }
      switch (m->member_type) {
        case ClassMemberField: {
          if (auto fm = getPtr<AST_Node_field_member>(m->info)) {
            os << Indent{ind+2} << "Field(" << accessStr(fm->access) << ", ";
            if (fm->var) {
              os << "name=\"" << fm->var->identifier << "\"";
              if (fm->var->var_type) {
                os << ", type="; printType(fm->var->var_type.get(), os, 0);
              }
              if (fm->var->initializer.has_value()) {
                os << ", init="; printInitializer(fm->var->initializer.value().get(), os, ind + 4);
              }
            }
            os << ")\n";
          }
          break;
        }
        case ClassMemberMethod: {
          if (auto mm = getPtr<AST_Node_method_member>(m->info)) {
            os << Indent{ind+2} << "Method(" << accessStr(mm->access) << ", ";
            if (mm->fun) {
              os << "name=\"" << mm->fun->identifier << "\"";
              if (mm->fun->fun_type) { os << ", type="; printType(mm->fun->fun_type.get(), os, 0); }
              if (mm->fun->body.has_value()) { os << ", body="; printBlock(mm->fun->body.value().get(), os, ind + 4); }
            }
            os << ")\n";
          }
          break;
        }
        case ClassMemberConstructor: {
          if (auto cm = getPtr<AST_Node_constructor_member>(m->info)) {
            os << Indent{ind+2} << "Ctor(" << accessStr(cm->access) << ", class=" << cm->class_name;
            if (!cm->params.empty()) {
              os << ", params=[";
              for (size_t i=0;i<cm->params.size();++i) { if (i) os << ", "; os << cm->params[i]; }
              os << "]";
            }
            if (cm->ctor_type) { os << ", type="; printType(cm->ctor_type.get(), os, 0); }
            if (cm->body.has_value()) { os << ", body="; printBlock(cm->body.value().get(), os, ind + 4); }
            os << ")\n";
          }
          break;
        }
      }
    }
    os << Indent{ind} << "]";
  }
  os << ")\n";
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
      if (auto* s = getPtr<AST_Node_struct_declaration>(d->info)) printStructDecl(s, os, ind);
      else os << Indent{ind} << "Struct\n";
      return;
    }
    case DeclUnion: {
      if (auto* u = getPtr<AST_Node_union_declaration>(d->info)) printUnionDecl(u, os, ind);
      else os << Indent{ind} << "Union\n";
      return;
    }
    case DeclEnum: {
      if (auto* e = getPtr<AST_Node_enum_declaration>(d->info)) printEnumDecl(e, os, ind);
      else os << Indent{ind} << "Enum\n";
      return;
    }
    case DeclTypedef: {
      if (auto* td = getPtr<AST_Node_typedef_declaration>(d->info)) printTypedefDecl(td, os, ind);
      else os << Indent{ind} << "Typedef\n";
      return;
    }
    case DeclClass: {
      if (auto* c = getPtr<AST_Node_class_declaration>(d->info)) printClassDecl(c, os, ind);
      else os << Indent{ind} << "Class\n";
      return;
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