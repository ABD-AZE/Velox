#include <iostream>
#include <memory>
#include <vector>
#include <string>

#include "ast/ast_variant.hpp"     // your variant-based AST
#include "tools/ast_printer.hpp"   // PrintAST(...)

using std::make_unique;

static std::unique_ptr<AST_Node_type> makePrim(TypeType tt) {
  auto t = make_unique<AST_Node_type>();
  t->type = tt;
  t->is_const = t->is_static = t->is_extern = false;
  t->info = std::monostate{}; // no payload for primitive types
  return t;
}

static std::unique_ptr<AST_Node_exp> makeConstInt(int v) {
  auto c = make_unique<AST_Node_const>();
  c->type = ConstInt;
  c->value.intValue = v;

  auto e = make_unique<AST_Node_exp>();
  e->expr_type = ExprConstant;
  e->info = std::move(c); // variant alt: unique_ptr<AST_Node_const>
  return e;
}

static std::unique_ptr<AST_Node_exp> makeVar(const std::string& name) {
  auto v = make_unique<AST_Node_var>();
  v->identifier = name;

  auto e = make_unique<AST_Node_exp>();
  e->expr_type = ExprVar;
  e->info = std::move(v); // variant alt: unique_ptr<AST_Node_var>
  return e;
}

static std::unique_ptr<AST_Node_exp> makeBinary(BinOpType op, std::unique_ptr<AST_Node_exp> lhs,
                                                std::unique_ptr<AST_Node_exp> rhs) {
  auto bop = make_unique<AST_Node_binary_operator>();
  bop->op = op;

  auto be = make_unique<AST_Node_binary_exp>();
  be->op = std::move(bop);
  be->left = std::move(lhs);
  be->right = std::move(rhs);

  auto e = make_unique<AST_Node_exp>();
  e->expr_type = ExprBinary;
  e->info = std::move(be); // variant alt: unique_ptr<AST_Node_binary_exp>
  return e;
}

int main() {
  auto prog = std::make_unique<AST_Node_program>();

  // ---- decl #1: int x = 42;
  {
    auto vardecl = make_unique<AST_Node_variable_declaration>();
    vardecl->var_type = makePrim(TypeInt);
    vardecl->identifier = "x";
    vardecl->storage_class = StorageClassNone;

    auto init = make_unique<AST_Node_initializer>();
    init->init_type = InitializerExp;
    init->info.expr = makeConstInt(42);
    vardecl->initializer = std::move(init);

    auto d = make_unique<AST_Node_declaration>();
    d->decl_type = DeclVariable;
    d->info = std::move(vardecl); // variant alt: unique_ptr<AST_Node_variable_declaration>
    prog->AddDeclaration(std::move(d));
  }

  // ---- decl #2: int add(int a, int b) { return a + b; }
  {
    // function type: (int,int) -> int
    auto tf = make_unique<AST_Node_type_function>();
    tf->params.push_back(makePrim(TypeInt));
    tf->params.push_back(makePrim(TypeInt));
    tf->ret = makePrim(TypeInt);

    auto funType = make_unique<AST_Node_type>();
    funType->type = TypeFunction;
    funType->is_const = funType->is_static = funType->is_extern = false;
    funType->info = std::move(tf); // variant alt: unique_ptr<AST_Node_type_function>

    auto f = make_unique<AST_Node_function_declaration>();
    f->identifier = "add";
    f->params = {"a", "b"};
    f->fun_type = std::move(funType);
    f->storage_class = StorageClassNone;

    // return a + b;
    auto ret = make_unique<AST_Node_return_statement>();
    ret->expr = makeBinary(BinOpAdd, makeVar("a"), makeVar("b"));

    auto sret = make_unique<AST_Node_statement>();
    sret->stmt_type = StmtReturn;
    sret->info.return_statement = std::move(ret);

    auto bi = make_unique<AST_Node_block_item>();
    bi->item_type = BlockItemStatement;
    bi->info.statement = std::move(sret);

    auto body = make_unique<AST_Node_block>();
    body->items.push_back(std::move(bi));
    f->body = std::move(body);

    auto d = make_unique<AST_Node_declaration>();
    d->decl_type = DeclFunction;
    d->info = std::move(f); // variant alt: unique_ptr<AST_Node_function_declaration>
    prog->AddDeclaration(std::move(d));
  }

  // ---- pretty-print the program
  velox_astpp::PrintAST(prog.get(), std::cout);
  return 0;
}
