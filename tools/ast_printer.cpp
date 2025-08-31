#include "tools/ast_printer.hpp"
#include "ast/ast_variant.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <variant>

namespace velox_astpp {

namespace {

// ---------- small helpers ---------------------------------------------------

struct Indent { int n; };
std::ostream& operator<<(std::ostream& os, const Indent& i) {
  for (int k = 0; k < i.n; ++k) os << ' ';
  return os;
}

// Pull a T* out of a variant that stores std::unique_ptr<T>
template <typename T, typename V>
const T* vget(const V& var) {
  if (auto p = std::get_if<std::unique_ptr<T>>(&var)) return p->get();
  return nullptr;
}

// Get std::string* out of a variant, if present
template <typename V>
const std::string* vget_str(const V& var) {
  return std::get_if<std::string>(&var);
}

std::string escape_str(const std::string& s) {
  std::string out; out.reserve(s.size()+4);
  for (unsigned char c : s) {
    switch (c) {
      case '\\': out += "\\\\"; break;
      case '\"': out += "\\\""; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default:
        if (c < 32) { out += "\\x"; char buf[3]; sprintf(buf,"%02X", c); out += buf; }
        else out += char(c);
    }
  }
  return out;
}

// ---------- to_string helpers (unchanged enums) -----------------------------

const char* to_string(ConstType v) {
  switch (v) {
    case ConstInt: return "ConstInt";
    case ConstLong: return "ConstLong";
    case ConstUInt: return "ConstUInt";
    case ConstULong: return "ConstULong";
    case ConstDouble: return "ConstDouble";
    case ConstChar: return "ConstChar";
    case ConstUChar: return "ConstUChar";
  }
  return "?Const";
}

const char* to_string(BinOpType v) {
  switch (v) {
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
  return "?binop";
}

const char* to_string(UnOpType v) {
  switch (v) {
    case UnOpNeg: return "-";
    case UnOpNot: return "!";
    case UnOpCompl: return "~";
  }
  return "?unop";
}

const char* to_string(TypeType t) {
  switch (t) {
    case TypeChar: return "char";
    case TypeSChar: return "signed char";
    case TypeUChar: return "unsigned char";
    case TypeInt: return "int";
    case TypeLong: return "long";
    case TypeUInt: return "unsigned int";
    case TypeULong: return "unsigned long";
    case TypeDouble: return "double";
    case TypeVoid: return "void";
    case TypeFunction: return "fun";
    case TypePointer: return "ptr";
    case TypeArray: return "array";
    case TypeStructure: return "struct";
    case TypeUnion: return "union";
    case TypeEnum: return "enum";
    case TypeNamedtype: return "typedef-name";
    case TypeClass: return "class";
    case TypeVaList: return "va_list";
  }
  return "?type";
}

const char* to_string(AssignOpType a) {
  switch (a) {
    case Assign: return "=";
    case AddAssign: return "+=";
    case SubAssign: return "-=";
    case MulAssign: return "*=";
    case DivAssign: return "/=";
    case ModAssign: return "%=";
    case AndAssign: return "&=";
    case OrAssign: return "|=";
    case XorAssign: return "^=";
    case ShlAssign: return "<<=";
    case ShrAssign: return ">>=";
  }
  return "?=";
}

const char* to_string(StroageClass s) {
  switch (s) {
    case StorageClassNone: return "none";
    case StorageClassStatic: return "static";
    case StorageClassExtern: return "extern";
  }
  return "?sc";
}

const char* to_string(BlockItemType t) {
  switch (t) {
    case BlockItemDeclaration: return "decl";
    case BlockItemStatement:   return "stmt";
  }
  return "?blockitem";
}

const char* to_string(StmtType t) {
  switch (t) {
    case StmtExpression: return "expr-stmt";
    case StmtReturn: return "return";
    case StmtIf: return "if";
    case StmtWhile: return "while";
    case StmtDoWhile: return "do-while";
    case StmtFor: return "for";
    case StmtBreak: return "break";
    case StmtContinue: return "continue";
    case StmtBlock: return "block";
    case StmtNull: return "null";
    case StmtLabeled: return "labeled";
    case StmtSwitch: return "switch";
    case StmtGoto: return "goto";
  }
  return "?stmt";
}

const char* to_string(LabelType t) {
  switch (t) {
    case LabelIdentifier: return "label";
    case LabelCase: return "case";
    case LabelDefault: return "default";
  }
  return "?label";
}

const char* to_string(InitializerType t) {
  switch (t) {
    case InitializerExp: return "init-exp";
    case InitializerList: return "init-list";
    case InitializerCtor: return "init-ctor";
  }
  return "?init";
}

const char* to_string(DeclType t) {
  switch (t) {
    case DeclVariable: return "var-decl";
    case DeclFunction: return "fun-decl";
    case DeclStruct: return "struct-decl";
    case DeclUnion: return "union-decl";
    case DeclEnum: return "enum-decl";
    case DeclTypedef: return "typedef-decl";
    case DeclClass: return "class-decl";
  }
  return "?decl";
}

// ---------- forward decls for recursive printers ---------------------------

void printType(const AST_Node_type* t, std::ostream& os, int ind);
void printArraySize(const AST_Node_array_size* s, std::ostream& os, int ind);
void printInitializer(const AST_Node_initializer* init, std::ostream& os, int ind);
void printExp(const AST_Node_exp* e, std::ostream& os, int ind);
void printStmt(const AST_Node_statement* s, std::ostream& os, int ind);
void printBlock(const AST_Node_block* b, std::ostream& os, int ind);
void printDecl(const AST_Node_declaration* d, std::ostream& os, int ind);

// ---------- type printers ---------------------------------------------------

void printType(const AST_Node_type* t, std::ostream& os, int ind) {
  if (!t) { os << Indent{ind} << "(type null)\n"; return; }
  os << Indent{ind} << "(type " << to_string(t->type);
  if (t->is_const)  os << " const";
  if (t->is_static) os << " static";
  if (t->is_extern) os << " extern";
  os << ")\n";

  switch (t->type) {
    case TypePointer: {
      if (auto p = vget<AST_Node_type_pointer>(t->info)) {
        os << Indent{ind} << "↳ pointer-level=" << p->level << "\n";
        printType(p->referenced.get(), os, ind+2);
      }
      break;
    }
    case TypeArray: {
      if (auto a = vget<AST_Node_type_array>(t->info)) {
        os << Indent{ind} << "↳ array\n";
        printType(a->element.get(), os, ind+2);
        printArraySize(a->size.get(), os, ind+2);
      }
      break;
    }
    case TypeFunction: {
      if (auto f = vget<AST_Node_type_function>(t->info)) {
        os << Indent{ind} << "↳ function-type\n";
        os << Indent{ind+2} << "params:\n";
        for (auto& p : f->params) printType(p.get(), os, ind+4);
        os << Indent{ind+2} << "return:\n";
        printType(f->ret.get(), os, ind+4);
      }
      break;
    }
    case TypeStructure:
    case TypeUnion:
    case TypeEnum:
    case TypeClass:
    case TypeNamedtype: {
      if (auto s = vget_str(t->info)) {
        os << Indent{ind} << "↳ tag/name: " << *s << "\n";
      } else {
        os << Indent{ind} << "↳ tag/name: ?\n";
      }
      break;
    }
    default: break;
  }
}

void printArraySize(const AST_Node_array_size* s, std::ostream& os, int ind) {
  if (!s) { os << Indent{ind} << "(array-size unspecified)\n"; return; }

  switch (s->size_type) {
    case ArraySizeUnspecified:
      os << Indent{ind} << "(array-size ?)\n"; break;
    case ArraySizeConst: {
      os << Indent{ind} << "(array-size const ";
      if (auto c = vget<AST_Node_const>(s->info)) {
        switch (c->type) {
          case ConstInt: os << c->value.intValue; break;
          case ConstLong: os << c->value.longValue; break;
          case ConstUInt: os << c->value.uintValue; break;
          case ConstULong: os << c->value.ulongValue; break;
          default: os << "?"; break;
        }
      } else os << "?";
      os << ")\n";
      break;
    }
    case ArraySizeVar: {
      os << Indent{ind} << "(array-size vla)\n";
      if (auto e = vget<AST_Node_exp>(s->info)) printExp(e, os, ind+2);
      break;
    }
  }
}

// ---------- expressions -----------------------------------------------------

void printConst(const AST_Node_const* c, std::ostream& os, int ind) {
  if (!c) { os << Indent{ind} << "(const null)\n"; return; }
  os << Indent{ind} << "(const " << to_string(c->type) << " ";
  switch (c->type) {
    case ConstInt: os << c->value.intValue; break;
    case ConstLong: os << c->value.longValue; break;
    case ConstUInt: os << c->value.uintValue; break;
    case ConstULong: os << c->value.ulongValue; break;
    case ConstDouble: os << c->value.doubleValue; break;
    case ConstChar: os << (int)c->value.charValue; break;
    case ConstUChar: os << (unsigned)c->value.ucharValue; break;
  }
  os << ")\n";
}

void printExp(const AST_Node_exp* e, std::ostream& os, int ind) {
  if (!e) { os << Indent{ind} << "(expr null)\n"; return; }

  switch (e->expr_type) {
    case ExprConstant: {
      printConst(vget<AST_Node_const>(e->info), os, ind);
      return;
    }
    case ExprString: {
      auto s = vget<AST_Node_string>(e->info);
      os << Indent{ind} << "(string \"" << (s ? escape_str(s->value) : std::string("?")) << "\")\n";
      return;
    }
    case ExprVar: {
      auto v = vget<AST_Node_var>(e->info);
      os << Indent{ind} << "(var " << (v ? v->identifier : std::string("?")) << ")\n";
      return;
    }
    case ExprCast: {
      auto c = vget<AST_Node_cast>(e->info);
      os << Indent{ind} << "(cast)\n";
      if (c) {
        os << Indent{ind+2} << "type:\n";
        printType(c->target_type.get(), os, ind+4);
        os << Indent{ind+2} << "expr:\n";
        printExp(c->expr.get(), os, ind+4);
      }
      return;
    }
    case ExprUnary:
    case ExprPreInc:
    case ExprPreDec:
    case ExprPostInc:
    case ExprPostDec: {
      os << Indent{ind} << "(unary ";
      if (e->expr_type == ExprUnary) {
        if (auto u = vget<AST_Node_unary_exp>(e->info)) {
          if (u->op) os << to_string(u->op->op);
        }
      } else if (e->expr_type == ExprPreInc) os << "++ (pre)";
      else if (e->expr_type == ExprPreDec) os << "-- (pre)";
      else if (e->expr_type == ExprPostInc) os << "++ (post)";
      else if (e->expr_type == ExprPostDec) os << "-- (post)";
      os << ")\n";
      const AST_Node_exp* sub = nullptr;
      if (e->expr_type == ExprUnary) {
        if (auto u = vget<AST_Node_unary_exp>(e->info)) sub = u->expr.get();
      } else {
        sub = vget<AST_Node_exp>(e->info);
      }
      printExp(sub, os, ind+2);
      return;
    }
    case ExprBinary: {
      auto b = vget<AST_Node_binary_exp>(e->info);
      os << Indent{ind} << "(binary)\n";
      if (b) {
        os << Indent{ind+2} << "op: " << (b->op ? to_string(b->op->op) : "?") << "\n";
        os << Indent{ind+2} << "lhs:\n"; printExp(b->left.get(), os, ind+4);
        os << Indent{ind+2} << "rhs:\n"; printExp(b->right.get(), os, ind+4);
      }
      return;
    }
    case ExprAssignment: {
      auto a = vget<AST_Node_assignment>(e->info);
      os << Indent{ind} << "(assign " << (a ? to_string(a->op) : "?") << ")\n";
      if (a) {
        os << Indent{ind+2} << "lhs:\n"; printExp(a->left.get(), os, ind+4);
        os << Indent{ind+2} << "rhs:\n"; printExp(a->right.get(), os, ind+4);
      }
      return;
    }
    case ExprConditional: {
      auto c = vget<AST_Node_conditional>(e->info);
      os << Indent{ind} << "(conditional)\n";
      if (c) {
        os << Indent{ind+2} << "cond:\n"; printExp(c->condition.get(), os, ind+4);
        os << Indent{ind+2} << "then:\n"; printExp(c->true_expr.get(), os, ind+4);
        os << Indent{ind+2} << "else:\n"; printExp(c->false_expr.get(), os, ind+4);
      }
      return;
    }
    case ExprFunctionCall: {
      auto fc = vget<AST_Node_function_call>(e->info);
      os << Indent{ind} << "(call " << (fc ? fc->identifier : std::string("?")) << ")\n";
      if (fc) {
        for (size_t i=0;i<fc->args.size();++i) {
          os << Indent{ind+2} << "arg["<<i<<"]:\n";
          printExp(fc->args[i].get(), os, ind+4);
        }
      }
      return;
    }
    case ExprDereference: {
      os << Indent{ind} << "(* dereference)\n";
      printExp(vget<AST_Node_exp>(e->info), os, ind+2);
      return;
    }
    case ExprAddrof: {
      os << Indent{ind} << "(& addrof)\n";
      printExp(vget<AST_Node_exp>(e->info), os, ind+2);
      return;
    }
    case ExprSubscript: {
      auto s = vget<AST_Node_subscript>(e->info);
      os << Indent{ind} << "(subscript)\n";
      if (s) {
        os << Indent{ind+2} << "array:\n"; printExp(s->array.get(), os, ind+4);
        os << Indent{ind+2} << "index:\n"; printExp(s->index.get(), os, ind+4);
      }
      return;
    }
    case ExprSizeof: {
      os << Indent{ind} << "(sizeof expr)\n";
      printExp(vget<AST_Node_exp>(e->info), os, ind+2);
      return;
    }
    case ExprSizeofType: {
      os << Indent{ind} << "(sizeof type)\n";
      printType(vget<AST_Node_type>(e->info), os, ind+2);
      return;
    }
    case ExprDot: {
      auto d = vget<AST_Node_dot>(e->info);
      os << Indent{ind} << "(dot)\n";
      if (d) {
        printExp(d->structure.get(), os, ind+2);
        os << Indent{ind+2} << ". " << d->member << "\n";
      }
      return;
    }
    case ExprArrow: {
      auto a = vget<AST_Node_arrow>(e->info);
      os << Indent{ind} << "(arrow)\n";
      if (a) {
        printExp(a->pointer.get(), os, ind+2);
        os << Indent{ind+2} << "-> " << a->member << "\n";
      }
      return;
    }
  }
}

// ---------- initializer -----------------------------------------------------

void printInitializer(const AST_Node_initializer* init, std::ostream& os, int ind) {
  if (!init) { os << Indent{ind} << "(initializer null)\n"; return; }
  os << Indent{ind} << "(" << to_string(init->init_type) << ")\n";
  switch (init->init_type) {
    case InitializerExp: {
      if (auto e = vget<AST_Node_exp>(init->info)) printExp(e, os, ind+2);
      break;
    }
    case InitializerList:
    case InitializerCtor: {
      if (auto vec = std::get_if<std::vector<std::unique_ptr<AST_Node_initializer>>>(&init->info)) {
        for (size_t i=0;i<vec->size();++i) {
          os << Indent{ind+2} << ((init->init_type==InitializerCtor) ? "ctor-arg[" : "elem[") << i << "]:\n";
          printInitializer((*vec)[i].get(), os, ind+4);
        }
      }
      break;
    }
  }
}

// ---------- blocks & statements --------------------------------------------

void printBlock(const AST_Node_block* b, std::ostream& os, int ind);

void printDecl(const AST_Node_declaration* d, std::ostream& os, int ind);

void printBlockItem(const AST_Node_block_item* it, std::ostream& os, int ind) {
  if (!it) { os << Indent{ind} << "(block-item null)\n"; return; }
  os << Indent{ind} << "(block-item " << to_string(it->item_type) << ")\n";
  if (it->item_type == BlockItemDeclaration) {
    if (auto p = vget<AST_Node_declaration>(it->info)) printDecl(p, os, ind+2);
  } else {
    if (auto p = vget<AST_Node_statement>(it->info)) printStmt(p, os, ind+2);
  }
}

void printBlock(const AST_Node_block* b, std::ostream& os, int ind) {
  if (!b) { os << Indent{ind} << "(block null)\n"; return; }
  os << Indent{ind} << "(block items=" << b->items.size() << ")\n";
  for (auto& it : b->items) printBlockItem(it.get(), os, ind+2);
}

void printStmt(const AST_Node_statement* s, std::ostream& os, int ind) {
  if (!s) { os << Indent{ind} << "(stmt null)\n"; return; }
  os << Indent{ind} << "(stmt " << to_string(s->stmt_type) << ")\n";
  switch (s->stmt_type) {
    case StmtExpression: {
      if (auto es = vget<AST_Node_expression_statement>(s->info)) {
        if (es->expr) printExp(es->expr.get(), os, ind+2);
      }
      break;
    }
    case StmtReturn: {
      if (auto rs = vget<AST_Node_return_statement>(s->info)) {
        if (rs->expr.has_value() && rs->expr.value()) {
          printExp(rs->expr.value().get(), os, ind+2);
        }
      }
      break;
    }
    case StmtIf: {
      if (auto is = vget<AST_Node_if_statement>(s->info)) {
        os << Indent{ind+2} << "cond:\n";
        printExp(is->condition.get(), os, ind+4);
        os << Indent{ind+2} << "then:\n";
        printStmt(is->true_statement.get(), os, ind+4);
        if (is->false_statement.has_value() && is->false_statement.value()) {
          os << Indent{ind+2} << "else:\n";
          printStmt(is->false_statement.value().get(), os, ind+4);
        }
      }
      break;
    }
    case StmtWhile: {
      if (auto ws = vget<AST_Node_while_statement>(s->info)) {
        os << Indent{ind+2} << "cond:\n";
        printExp(ws->condition.get(), os, ind+4);
        os << Indent{ind+2} << "body:\n";
        printStmt(ws->body.get(), os, ind+4);
      }
      break;
    }
    case StmtDoWhile: {
      if (auto ds = vget<AST_Node_do_while_statement>(s->info)) {
        os << Indent{ind+2} << "body:\n";
        printStmt(ds->body.get(), os, ind+4);
        os << Indent{ind+2} << "cond:\n";
        printExp(ds->condition.get(), os, ind+4);
      }
      break;
    }
    case StmtFor: {
      if (auto fs = vget<AST_Node_for_statement>(s->info)) {
        os << Indent{ind+2} << "init:\n";
        if (fs->init) {
          auto* fi = fs->init.get();
          if (fi->init_type == InitDeclaration) {
            if (auto d = vget<AST_Node_declaration>(fi->info)) printDecl(d, os, ind+4);
          } else if (fi->init_type == InitExpression) {
            if (auto exs = vget<AST_Node_expression_statement>(fi->info)) {
              if (exs->expr) printExp(exs->expr.get(), os, ind+4);
            }
          }
        }
        if (fs->condition.has_value() && fs->condition.value()) {
          os << Indent{ind+2} << "cond:\n";
          if (fs->condition.value()->expr)
            printExp(fs->condition.value()->expr.get(), os, ind+4);
        }
        if (fs->increment.has_value() && fs->increment.value()) {
          os << Indent{ind+2} << "post:\n";
          if (fs->increment.value()->expr)
            printExp(fs->increment.value()->expr.get(), os, ind+4);
        }
        os << Indent{ind+2} << "body:\n";
        printStmt(fs->body.get(), os, ind+4);
      }
      break;
    }
    case StmtBreak:
    case StmtContinue:
      // leaf
      break;
    case StmtBlock: {
      if (auto b = vget<AST_Node_block>(s->info)) printBlock(b, os, ind+2);
      break;
    }
    case StmtNull:
      // empty ;
      break;
    case StmtLabeled: {
      if (auto ls = vget<AST_Node_labeled_statement>(s->info)) {
        for (auto& name : ls->label) os << Indent{ind+2} << "label: " << name << "\n";
        os << Indent{ind+2} << "stmt:\n";
        printStmt(ls->statement.get(), os, ind+4);
      }
      break;
    }
    case StmtSwitch: {
      if (auto ss = vget<AST_Node_switch_statement>(s->info)) {
        os << Indent{ind+2} << "selector:\n";
        printExp(ss->selector.get(), os, ind+4);
        os << Indent{ind+2} << "body:\n";
        printStmt(ss->body.get(), os, ind+4);
      }
      break;
    }
    case StmtGoto: {
      if (auto label = vget_str(s->info))
        os << Indent{ind+2} << "target: " << *label << "\n";
      else
        os << Indent{ind+2} << "target: ?\n";
      break;
    }
  }
}

// ---------- declarations ----------------------------------------------------

void printType(const AST_Node_type* t, std::ostream& os, int ind);
void printInitializer(const AST_Node_initializer* init, std::ostream& os, int ind);

void printMemberDecl(const AST_Node_member_declaration* m, std::ostream& os, int ind) {
  if (!m) { os << Indent{ind} << "(member null)\n"; return; }
  os << Indent{ind} << "(member " << m->identifier << ")\n";
  printType(m->member_type.get(), os, ind+2);
}

void printStructDecl(const AST_Node_struct_declaration* s, std::ostream& os, int ind) {
  if (!s) { os << Indent{ind} << "(struct null)\n"; return; }
  os << Indent{ind} << "(struct " << s->tag << ")\n";
  for (auto& m : s->members) printMemberDecl(m.get(), os, ind+2);
}

void printUnionDecl(const AST_Node_union_declaration* u, std::ostream& os, int ind) {
  if (!u) { os << Indent{ind} << "(union null)\n"; return; }
  os << Indent{ind} << "(union " << u->tag << ")\n";
  for (auto& m : u->members) printMemberDecl(m.get(), os, ind+2);
}

void printEnumDecl(const AST_Node_enum_declaration* e, std::ostream& os, int ind) {
  if (!e) { os << Indent{ind} << "(enum null)\n"; return; }
  os << Indent{ind} << "(enum";
  if (e->tag.has_value()) os << " " << *e->tag;
  os << ")\n";
  for (auto& en : e->enumerators) {
    os << Indent{ind+2} << "enumerator " << en->name;
    if (en->value.has_value() && en->value.value()) {
      os << " = ";
      auto* c = en->value.value().get();
      switch (c->type) {
        case ConstInt: os << c->value.intValue; break;
        case ConstLong: os << c->value.longValue; break;
        case ConstUInt: os << c->value.uintValue; break;
        case ConstULong: os << c->value.ulongValue; break;
        default: os << "?";
      }
    }
    os << "\n";
  }
}

void printVarDecl(const AST_Node_variable_declaration* v, std::ostream& os, int ind) {
  if (!v) { os << Indent{ind} << "(var-decl null)\n"; return; }
  os << Indent{ind} << "(var " << v->identifier << " storage=" << to_string(v->storage_class) << ")\n";
  os << Indent{ind+2} << "type:\n";  printType(v->var_type.get(), os, ind+4);
  if (v->initializer.has_value() && v->initializer.value()) {
    os << Indent{ind+2} << "init:\n";
    printInitializer(v->initializer.value().get(), os, ind+4);
  }
}

void printFuncDecl(const AST_Node_function_declaration* f, std::ostream& os, int ind) {
  if (!f) { os << Indent{ind} << "(fun-decl null)\n"; return; }
  os << Indent{ind} << "(function " << f->identifier << " storage=" << to_string(f->storage_class) << ")\n";
  os << Indent{ind+2} << "params:";
  if (f->params.empty()) os << " (none)\n";
  else {
    os << "\n";
    for (size_t i=0;i<f->params.size();++i)
      os << Indent{ind+4} << "- " << f->params[i] << "\n";
  }
  os << Indent{ind+2} << "type:\n";
  printType(f->fun_type.get(), os, ind+4);
  if (f->body.has_value() && f->body.value()) {
    os << Indent{ind+2} << "body:\n";
    printBlock(f->body.value().get(), os, ind+4);
  }
}

void printTypedefDecl(const AST_Node_typedef_declaration* t, std::ostream& os, int ind) {
  if (!t) { os << Indent{ind} << "(typedef null)\n"; return; }
  os << Indent{ind} << "(typedef)\n";
  for (auto& e : t->entries) {
    os << Indent{ind+2} << e->name << " :=\n";
    printType(e->alias_type.get(), os, ind+4);
  }
}

void printClassDecl(const AST_Node_class_declaration* c, std::ostream& os, int ind) {
  if (!c) { os << Indent{ind} << "(class null)\n"; return; }
  os << Indent{ind} << "(class " << c->name << ")\n";
  for (auto& m : c->members) {
    if (!m) continue;
    switch (m->member_type) {
      case ClassMemberField: {
        auto fld = vget<AST_Node_field_member>(m->info);
        os << Indent{ind+2} << "(field access=" << ((fld && fld->access==Public) ? "public" : "private") << ")\n";
        if (fld) printVarDecl(fld->var.get(), os, ind+4);
        break;
      }
      case ClassMemberMethod: {
        auto md = vget<AST_Node_method_member>(m->info);
        os << Indent{ind+2} << "(method access=" << ((md && md->access==Public) ? "public" : "private") << ")\n";
        if (md) printFuncDecl(md->fun.get(), os, ind+4);
        break;
      }
      case ClassMemberConstructor: {
        auto ct = vget<AST_Node_constructor_member>(m->info);
        os << Indent{ind+2} << "(ctor access=" << ((ct && ct->access==Public) ? "public" : "private") << ")\n";
        if (ct) {
          os << Indent{ind+4} << "name: " << ct->class_name << "\n";
          os << Indent{ind+4} << "params:";
          if (ct->params.empty()) os << " (none)\n";
          else {
            os << "\n";
            for (auto& p : ct->params) os << Indent{ind+6} << "- " << p << "\n";
          }
          os << Indent{ind+4} << "type:\n";
          printType(ct->ctor_type.get(), os, ind+6);
          if (ct->body.has_value() && ct->body.value()) {
            os << Indent{ind+4} << "body:\n";
            printBlock(ct->body.value().get(), os, ind+6);
          }
        }
        break;
      }
    }
  }
}

void printDecl(const AST_Node_declaration* d, std::ostream& os, int ind) {
  if (!d) { os << Indent{ind} << "(decl null)\n"; return; }
  os << Indent{ind} << "(declaration " << to_string(d->decl_type) << ")\n";
  switch (d->decl_type) {
    case DeclVariable:  if (auto p = vget<AST_Node_variable_declaration>(d->info))  printVarDecl(p, os, ind+2); break;
    case DeclFunction:  if (auto p = vget<AST_Node_function_declaration>(d->info))  printFuncDecl(p, os, ind+2); break;
    case DeclStruct:    if (auto p = vget<AST_Node_struct_declaration>(d->info))    printStructDecl(p, os, ind+2); break;
    case DeclUnion:     if (auto p = vget<AST_Node_union_declaration>(d->info))     printUnionDecl(p, os, ind+2); break;
    case DeclEnum:      if (auto p = vget<AST_Node_enum_declaration>(d->info))      printEnumDecl(p, os, ind+2); break;
    case DeclTypedef:   if (auto p = vget<AST_Node_typedef_declaration>(d->info))   printTypedefDecl(p, os, ind+2); break;
    case DeclClass:     if (auto p = vget<AST_Node_class_declaration>(d->info))     printClassDecl(p, os, ind+2); break;
  }
}

} // namespace

// ---------- public entry ----------------------------------------------------

void PrintAST(const AST_Node_program* root, std::ostream& os) {
  if (!root) { os << "(program null)\n"; return; }
  os << "(program declarations=" << root->declarations.size() << ")\n";
  for (auto& d : root->declarations) printDecl(d.get(), os, 2);
}

} // namespace velox_astpp
