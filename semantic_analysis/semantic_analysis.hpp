#pragma once
#include "../ast/ast.hpp"
#include "../symbol_table/symbol_table.hpp"
#include <map>
#include <stack>
#include <string>
#include <vector>

extern std::unordered_map<std::string, SymbolTableEntry> global_symbol_table;
// Semantic Analyzer using visitor pattern
class SemanticAnalyzer : public ASTVisitor {
public:
  SemanticAnalyzer();

  // Main entry point
  void analyze(ASTNodePtr &ast);

  // Visitor methods for all AST node types
  void visit(ProgramNode &node) override;
  void visit(FunctionDefinitionNode &node) override;
  void visit(FunDeclNode &node) override;
  void visit(VarDeclNode &node) override;
  void visit(StructDeclarationNode &node) override;
  void visit(MemberDeclarationNode &node) override;
  void visit(BlockNode &node) override;
  void visit(BlockItemNode &node) override;
  void visit(DeclarationNode &node) override;
  void visit(FunctionCallNode &node) override;
  void visit(InitializerNode &node) override;

  // Statement visitors
  void visit(ReturnStatement &node) override;
  void visit(ExpressionStatement &node) override;
  void visit(IfStatement &node) override;
  void visit(CompoundStatement &node) override;
  void visit(NullStatement &node) override;
  void visit(GotoStatement &node) override;
  void visit(LabelStatement &node) override;
  void visit(WhileNode &node) override;
  void visit(DoWhileNode &node) override;
  void visit(ForNode &node) override;
  void visit(BreakNode &node) override;
  void visit(ContinueNode &node) override;
  void visit(ForInit &node) override;
  void visit(InitDecl &node) override;
  void visit(InitExp &node) override;

  // Expression visitors
  void visit(BinaryExpression &node) override;
  void visit(UnaryExpression &node) override;
  void visit(AssignmentExpression &node) override;
  void visit(PostfixExpression &node) override;
  void visit(ConstantExpression &node) override;
  void visit(VariableExpression &node) override;
  void visit(ConditionalExpression &node) override;
  void visit(CastExpression &node) override;
  void visit(DereferenceExpression &node) override;
  void visit(AddressOfExpression &node) override;
  void visit(SubscriptExpression &node) override;
  void visit(StringLiteralExpression &node) override;
  void visit(SizeofExpression &node) override;
  void visit(SizeofTypeExpression &node) override;
  void visit(DotExpression &node) override;
  void visit(ArrowExpression &node) override;

  // Type and declarator visitors (no-ops for variable resolution)
  void visit(Type &node) override;
  void visit(Ident &node) override;
  void visit(DeclaratorNode &node) override;
  void visit(PointerDeclarator &node) override;
  void visit(FunDeclarator &node) override;
  void visit(paraminfo &node) override;
  void visit(AbstractPointer &node) override;
  void visit(AbstractBase &node) override;
  void visit(AbstractArray &node) override;
  void visit(ArrayDeclarator &node) override;

  bool success;
  int label_counter = 0;
  std::vector<std::string> errors;

private:
  // stores the updated variable names for the current scope(the bool represents
  // linkage of the variable), the current scope being the last element in the
  // stack
  std::map<std::string, std::pair<std::string, bool>> identifier_map;
  std::string current_label;
  // Helper methods
  void pushScope();
  void popScope();
  PointerType getCommonPointerType(ExpressionNode *first,
                                   ExpressionNode *second);
  void validateCast(CastExpression &node);
  // Helper function for generating unique variable names
  std::string make_temp(const std::string &var_name);
  // Helper function for generating unique labels
  std::string make_label();
  // Helper function to check if expression is an lvalue
  bool isLvalue(ASTNode *expr);
  ASTNodePtr convertByAssignment(ASTNodePtr exp, Type &targetType);
  // Helper function to make implicit type conversions explicit
  ASTNodePtr convertTo(ASTNodePtr expr, Type &targetType);
  bool isFunctionBlock = false;
  bool inFunctionScope = false;
  bool inFileScope = true;
  bool forInit = false;
  FunDeclNode *currentFunction = nullptr;
  // Scope stack for nested blocks
  std::vector<std::map<std::string, std::pair<std::string, bool>>> scope_stack;
};
