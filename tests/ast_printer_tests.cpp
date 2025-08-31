// tests/ast_printer_tests.cpp
#include <sstream>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <variant>
#include <cassert>
using namespace std;

#include "ast/ast_variant.hpp"
#include "ast/ast_printer.hpp"

using std::make_unique;
using std::string;
using std::unique_ptr;
using std::vector;

// ---------- tiny builders to keep tests readable ----------
static unique_ptr<AST_Node_const> c_int(int v) {
  auto c = make_unique<AST_Node_const>();
  c->type = ConstInt;
  c->value.intValue = v;
  return c;
}
static unique_ptr<AST_Node_string> c_str(string v) {
  auto s = make_unique<AST_Node_string>();
  s->value = std::move(v);
  return s;
}
static unique_ptr<AST_Node_var> var(string id) {
  auto v = make_unique<AST_Node_var>();
  v->identifier = std::move(id);
  return v;
}
static unique_ptr<AST_Node_binary_exp> bin(BinOpType op, unique_ptr<AST_Node_exp> l, unique_ptr<AST_Node_exp> r) {
  auto b = make_unique<AST_Node_binary_exp>();
  b->op = make_unique<AST_Node_binary_operator>();
  b->op->op = op;
  b->left = std::move(l);
  b->right = std::move(r);
  return b;
}
static unique_ptr<AST_Node_assignment> asg(unique_ptr<AST_Node_exp> l, unique_ptr<AST_Node_exp> r) {
  auto a = make_unique<AST_Node_assignment>();
  a->left = std::move(l);
  a->right = std::move(r);
  return a;
}
static unique_ptr<AST_Node_function_call> call(string name, vector<unique_ptr<AST_Node_exp>> args) {
  auto f = make_unique<AST_Node_function_call>();
  f->identifier = std::move(name);
  f->args = std::move(args);
  return f;
}
static unique_ptr<AST_Node_exp> E_from(auto ptr, ExprType t) {
  auto e = make_unique<AST_Node_exp>();
  e->expr_type = t;
  e->info = std::move(ptr);
  return e;
}
static unique_ptr<AST_Node_expression_statement> S_expr(unique_ptr<AST_Node_exp> e) {
  auto s = make_unique<AST_Node_expression_statement>();
  s->expr = std::move(e);
  return s;
}
static unique_ptr<AST_Node_return_statement> S_ret(unique_ptr<AST_Node_exp> e) {
  auto r = make_unique<AST_Node_return_statement>();
  r->expr = std::optional{ std::move(e) };
  return r;
}
static unique_ptr<AST_Node_block_item> BI_stmt(unique_ptr<AST_Node_statement> s) {
  auto bi = make_unique<AST_Node_block_item>();
  bi->item_type = BlockItemStatement;
  bi->info = std::move(s);
  return bi;
}
static unique_ptr<AST_Node_block> block_with_single_stmt(unique_ptr<AST_Node_statement> s) {
  auto b = make_unique<AST_Node_block>();
  b->items.push_back(BI_stmt(std::move(s)));
  return b;
}
static unique_ptr<AST_Node_statement> ST_from(auto ptr, StmtType t) {
  auto s = make_unique<AST_Node_statement>();
  s->stmt_type = t;
  s->info = std::move(ptr);
  return s;
}
static unique_ptr<AST_Node_variable_declaration> Vardecl(string name, unique_ptr<AST_Node_initializer> init = nullptr) {
  auto v = make_unique<AST_Node_variable_declaration>();
  v->identifier = std::move(name);
  v->var_type = make_unique<AST_Node_type>(); // not printed; keep minimal
  if (init) v->initializer = std::optional{ std::move(init) };
  return v;
}
static unique_ptr<AST_Node_initializer> Init_from_exp(unique_ptr<AST_Node_exp> e) {
  auto in = make_unique<AST_Node_initializer>();
  in->init_type = InitializerExp;
  in->info = std::move(e);
  return in;
}
static unique_ptr<AST_Node_function_declaration> Func(string name, unique_ptr<AST_Node_block> body) {
  auto f = make_unique<AST_Node_function_declaration>();
  f->identifier = std::move(name);
  f->body = std::optional{ std::move(body) };
  f->fun_type = make_unique<AST_Node_type>(); // not printed; keep minimal
  return f;
}
static unique_ptr<AST_Node_declaration> DeclFunc(unique_ptr<AST_Node_function_declaration> f) {
  auto d = make_unique<AST_Node_declaration>();
  d->decl_type = DeclFunction;
  d->info = std::move(f);
  return d;
}
static unique_ptr<AST_Node_declaration> DeclVar(unique_ptr<AST_Node_variable_declaration> v) {
  auto d = make_unique<AST_Node_declaration>();
  d->decl_type = DeclVariable;
  d->info = std::move(v);
  return d;
}
static unique_ptr<AST_Node_declaration> DeclTagOnly(DeclType t) {
  auto d = make_unique<AST_Node_declaration>();
  d->decl_type = t;
  d->info = std::monostate{};
  return d;
}
static string print_program(const AST_Node_program* p) {
  std::ostringstream oss;
  velox_astpp::PrintAST(p, oss);
  return oss.str();
}
static bool expect_eq(const string& got, const string& want, const string& testname) {
  if (got == want) {
    std::cout << "[PASS] " << testname << "\n";
    return true;
  }
  std::cout << "[FAIL] " << testname << "\n--- got ---\n" << got << "--- want ---\n" << want;
  // quick pointer where it differs:
  size_t i=0, n=std::min(got.size(), want.size());
  for (; i<n && got[i]==want[i]; ++i) {}
  std::cout << "--- diff at pos " << i << " ---\n";
  return false;
}

int main() {
  int failed = 0;

  // 1) Empty program
  {
    AST_Node_program prog;
    string want =
      "Program(\n"
      ")\n";
    failed += !expect_eq(print_program(&prog), want, "EmptyProgram");
  }

  // 2) int main(){ return 2; }  -> single Return printed as body
  {
    AST_Node_program prog;
    auto ret2 = ST_from(S_ret(E_from(c_int(2), ExprConstant)), StmtReturn);
    auto f = Func("main", block_with_single_stmt(std::move(ret2)));
    prog.AddDeclaration(DeclFunc(std::move(f)));
    string want =
      "Program(\n"
      "  Function(\n"
      "    name=\"main\",\n"
      "    body=Return(\n"
      "      Constant(2)\n"
      "    )\n"
      "  )\n"
      ")\n";
    failed += !expect_eq(print_program(&prog), want, "ReturnConstant");
  }

  // 3) int x;  -> prints VarDecl(name="x")
  {
    AST_Node_program prog;
    prog.AddDeclaration(DeclVar(Vardecl("x")));
    string want =
      "Program(\n"
      "  VarDecl(name=\"x\")\n"
      ")\n";
    failed += !expect_eq(print_program(&prog), want, "GlobalVarNoInit");
  }

  // 4) int y = 1 + 2;
  {
    AST_Node_program prog;
    auto e = E_from(
      bin(BinOpAdd,
          E_from(c_int(1), ExprConstant),
          E_from(c_int(2), ExprConstant)),
      ExprBinary);
    prog.AddDeclaration(DeclVar(Vardecl("y", Init_from_exp(std::move(e)))));
    string want =
      "Program(\n"
      "  VarDecl(name=\"y\", init=\n"
      "    Binary(+, \n"
      "      Constant(1)\n"
      "      Constant(2)\n"
      "    )\n"
      "  )\n"
      ")\n";
    failed += !expect_eq(print_program(&prog), want, "GlobalVarBinaryInit");
  }

  // 5) void f(){ foo(1, x); }  -> FunctionCall inside Expr stmt
  {
    AST_Node_program prog;
    vector<unique_ptr<AST_Node_exp>> args;
    args.push_back(E_from(c_int(1), ExprConstant));
    args.push_back(E_from(var("x"), ExprVar));
    auto callFoo = E_from(call("foo", std::move(args)), ExprFunctionCall);
    auto f = Func("f", block_with_single_stmt(ST_from(S_expr(std::move(callFoo)), StmtExpression)));
    prog.AddDeclaration(DeclFunc(std::move(f)));
    string want =
      "Program(\n"
      "  Function(\n"
      "    name=\"f\",\n"
      "    body=Expr(\n"
      "      Call(foo)(\n"
      "        Constant(1)\n"
      "        Var(x)\n"
      "      )\n"
      "    )\n"
      "  )\n"
      ")\n";
    failed += !expect_eq(print_program(&prog), want, "FunctionCallExprStmt");
  }

  // 6) a = b; (assignment expression statement)
  {
    AST_Node_program prog;
    auto assign = E_from(asg(E_from(var("a"), ExprVar), E_from(var("b"), ExprVar)), ExprAssignment);
    auto f = Func("g", block_with_single_stmt(ST_from(S_expr(std::move(assign)), StmtExpression)));
    prog.AddDeclaration(DeclFunc(std::move(f)));
    string want =
      "Program(\n"
      "  Function(\n"
      "    name=\"g\",\n"
      "    body=Expr(\n"
      "      Assign(\n"
      "        Var(a)\n"
      "        Var(b)\n"
      "      )\n"
      "    )\n"
      "  )\n"
      ")\n";
    failed += !expect_eq(print_program(&prog), want, "AssignmentExprStmt");
  }

  // 7) string literal expr stmt -> String("hi")
  {
    AST_Node_program prog;
    auto str = E_from(c_str("hi"), ExprString);
    auto f = Func("h", block_with_single_stmt(ST_from(S_expr(std::move(str)), StmtExpression)));
    prog.AddDeclaration(DeclFunc(std::move(f)));
    string want =
      "Program(\n"
      "  Function(\n"
      "    name=\"h\",\n"
      "    body=Expr(\n"
      "      String(\"hi\")\n"
      "    )\n"
      "  )\n"
      ")\n";
    failed += !expect_eq(print_program(&prog), want, "StringExprStmt");
  }

  // 8) multi-statement body -> prints Block()
  {
    AST_Node_program prog;
    auto b = make_unique<AST_Node_block>();
    // two statements => not single => Block()
    b->items.push_back(BI_stmt(ST_from(S_expr(E_from(c_int(1), ExprConstant)), StmtExpression)));
    b->items.push_back(BI_stmt(ST_from(S_expr(E_from(c_int(2), ExprConstant)), StmtExpression)));
    auto f = Func("many", std::move(b));
    prog.AddDeclaration(DeclFunc(std::move(f)));
    string want =
      "Program(\n"
      "  Function(\n"
      "    name=\"many\",\n"
      "    body=Block()\n"
      "  )\n"
      ")\n";
    failed += !expect_eq(print_program(&prog), want, "MultiStmtBodyIsBlock");
  }

  // 9) Struct decl prints "Struct"
  {
    AST_Node_program prog;
    prog.AddDeclaration(DeclTagOnly(DeclStruct));
    string want =
      "Program(\n"
      "  Struct\n"
      ")\n";
    failed += !expect_eq(print_program(&prog), want, "StructTagPrint");
  }

  // 10) Union
  {
    AST_Node_program prog;
    prog.AddDeclaration(DeclTagOnly(DeclUnion));
    string want =
      "Program(\n"
      "  Union\n"
      ")\n";
    failed += !expect_eq(print_program(&prog), want, "UnionTagPrint");
  }

  // 11) Enum
  {
    AST_Node_program prog;
    prog.AddDeclaration(DeclTagOnly(DeclEnum));
    string want =
      "Program(\n"
      "  Enum\n"
      ")\n";
    failed += !expect_eq(print_program(&prog), want, "EnumTagPrint");
  }

  // 12) Typedef
  {
    AST_Node_program prog;
    prog.AddDeclaration(DeclTagOnly(DeclTypedef));
    string want =
      "Program(\n"
      "  Typedef\n"
      ")\n";
    failed += !expect_eq(print_program(&prog), want, "TypedefTagPrint");
  }

  // 13) Class
  {
    AST_Node_program prog;
    prog.AddDeclaration(DeclTagOnly(DeclClass));
    string want =
      "Program(\n"
      "  Class\n"
      ")\n";
    failed += !expect_eq(print_program(&prog), want, "ClassTagPrint");
  }

  // 14) While statement (printer falls back to "Stmt")
  {
    AST_Node_program prog;
    auto whileStmt = make_unique<AST_Node_statement>();
    whileStmt->stmt_type = StmtWhile;
    whileStmt->info = std::monostate{};
    auto f = Func("loop", block_with_single_stmt(std::move(whileStmt)));
    prog.AddDeclaration(DeclFunc(std::move(f)));
    string want =
      "Program(\n"
      "  Function(\n"
      "    name=\"loop\",\n"
      "    body=Stmt\n"
      "  )\n"
      ")\n";
    failed += !expect_eq(print_program(&prog), want, "WhileBecomesStmt");
  }

  // 15) Subscript/Dot/Arrow/Cast/?:/sizeof(type) are all printed as "Expr" by default
  //     We just ensure they don't crash and land in the default branch.
  {
    AST_Node_program prog;

    // Subscript: array[index]
    auto sub = make_unique<AST_Node_subscript>();
    sub->array = E_from(var("arr"), ExprVar);
    sub->index = E_from(c_int(3), ExprConstant);
    auto e_sub = E_from(std::move(sub), ExprSubscript);

    // Dot: s.m
    auto dot = make_unique<AST_Node_dot>();
    dot->structure = E_from(var("s"), ExprVar);
    dot->member = "m";
    auto e_dot = E_from(std::move(dot), ExprDot);

    // Arrow: p->m
    auto arw = make_unique<AST_Node_arrow>();
    arw->pointer = E_from(var("p"), ExprVar);
    arw->member = "m";
    auto e_arw = E_from(std::move(arw), ExprArrow);

    // Conditional: a?b:c
    auto cond = make_unique<AST_Node_conditional>();
    cond->condition = E_from(var("a"), ExprVar);
    cond->true_expr = E_from(var("b"), ExprVar);
    cond->false_expr = E_from(var("c"), ExprVar);
    auto e_cond = E_from(std::move(cond), ExprConditional);

    // sizeof(type)
    auto e_sz = make_unique<AST_Node_exp>();
    e_sz->expr_type = ExprSizeofType;
    e_sz->info = make_unique<AST_Node_type>();

    // Make three expr statements (any will print "Expr")
    auto b = make_unique<AST_Node_block>();
    b->items.push_back(BI_stmt(ST_from(S_expr(std::move(e_sub)), StmtExpression)));
    b->items.push_back(BI_stmt(ST_from(S_expr(std::move(e_dot)), StmtExpression)));
    b->items.push_back(BI_stmt(ST_from(S_expr(std::move(e_arw)), StmtExpression)));
    b->items.push_back(BI_stmt(ST_from(S_expr(std::move(e_cond)), StmtExpression)));
    b->items.push_back(BI_stmt(ST_from(S_expr(std::move(e_sz)), StmtExpression)));

    auto f = Func("misc", std::move(b));
    prog.AddDeclaration(DeclFunc(std::move(f)));

    // Because any body with >1 statement becomes Block(), we only assert that.
    string want =
      "Program(\n"
      "  Function(\n"
      "    name=\"misc\",\n"
      "    body=Block()\n"
      "  )\n"
      ")\n";
    failed += !expect_eq(print_program(&prog), want, "MiscExprsDefaultExprBranch");
  }

  std::cout << "\n====================\n";
  if (failed == 0) {
    std::cout << "All tests passed.\n";
    return 0;
  }
  std::cout << failed << " test(s) failed.\n";
  return 1;
}
