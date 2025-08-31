#include <memory>
#include <optional>
#include <string>
#include <vector>

enum ConstType {
  ConstInt,
  ConstLong,
  ConstUInt,
  ConstULong,
  ConstDouble,
  ConstChar,
  ConstUChar
};

enum BinOpType {
  BinOpAdd,
  BinOpSub,
  BinOpMul,
  BinOpDiv,
  BinOpMod,
  BinOpAnd,
  BinOpOr,
  BinOpXor,
  BinOpShl,
  BinOpShr
};

enum UnOpType {
  UnOpNeg,
  UnOpNot,
  UnOpPreInc,
  UnOpPreDec,
  UnOpPostInc,
  UnOpPostDec
};

enum TypeType {
  TypeChar,
  TypeSChar,
  TypeUChar,
  TypeInt,
  TypeLong,
  TypeUInt,
  TypeULong,
  TypeDouble,
  TypeVoid,
  TypeFunction,
  TypePointer,
  TypeArray,
  TypeStructure
};

enum ExprType {
  ExprConstant,
  ExprString,
  ExprVar,
  ExprCast,
  ExprUnary,
  ExprBinary,
  ExprAssignment,
  ExprConditional,
  ExprFunctionCall,
  ExprDereference,
  ExprAddrof,
  ExprSubscript,
  ExprSizeof,
  ExprSizeofType,
  ExprDot,
  ExprArrow
};

struct AST_Node_const {
  ConstType type;
  union {
    int intValue;
    long longValue;
    unsigned int uintValue;
    unsigned long ulongValue;
    double doubleValue;
    char charValue;
    unsigned char ucharValue;
  } value;
};

struct AST_Node_binary_operator {
  BinOpType op;
};

struct AST_Node_unary_operator {
  UnOpType op;
};

struct AST_Node_type {
  TypeType type;
  bool is_const;
  bool is_static;
  bool is_extern;
  union {
    std::unique_ptr<AST_Node_type_pointer> pointer;
    std::unique_ptr<AST_Node_type_array> array;
    std::unique_ptr<AST_Node_type_structure> structure;
    std::unique_ptr<AST_Node_type_function> function;
  } info;
};

struct AST_Node_type_pointer {
  std::unique_ptr<AST_Node_type> referenced;
  int level;
};

struct AST_Node_type_array {
  std::unique_ptr<AST_Node_type> element;
  int size;
};

struct AST_Node_type_structure {
  std::string tag;
};

struct AST_Node_type_function {
  std::vector<std::unique_ptr<AST_Node_type>> params;
  std::unique_ptr<AST_Node_type> ret;
};

struct AST_Node_exp {
  ExprType expr_type;
  union {
    std::unique_ptr<AST_Node_constant> constant;
    std::unique_ptr<AST_Node_string> string;
    std::unique_ptr<AST_Node_var> var;
    std::unique_ptr<AST_Node_cast> cast;
    std::unique_ptr<AST_Node_unary_exp> unary_exp;
    std::unique_ptr<AST_Node_binary_exp> binary_exp;
    std::unique_ptr<AST_Node_assignment> assignment;
    std::unique_ptr<AST_Node_conditional> conditional;
    std::unique_ptr<AST_Node_function_call> function_call;
    std::unique_ptr<AST_Node_dereference> dereference;
    std::unique_ptr<AST_Node_addrof> addrof;
    std::unique_ptr<AST_Node_subscript> subscript;
    std::unique_ptr<AST_Node_sizeof_exp> sizeof_exp;
    std::unique_ptr<AST_Node_sizeof_type> sizeof_type;
    std::unique_ptr<AST_Node_dot> dot;
    std::unique_ptr<AST_Node_arrow> arrow;
  } info;
};

struct AST_Node_constant {
  std::unique_ptr<AST_Node_const> constant;
};

struct AST_Node_string {
  std::string value;
};

struct AST_Node_var {
  std::string identifier;
};

struct AST_Node_cast {
  std::unique_ptr<AST_Node_type> target_type;
  std::unique_ptr<AST_Node_exp> expr;
};

struct AST_Node_unary_exp {
  std::unique_ptr<AST_Node_unary_operator> op;
  std::unique_ptr<AST_Node_exp> expr;
};

struct AST_Node_binary_exp {
  std::unique_ptr<AST_Node_binary_operator> op;
  std::unique_ptr<AST_Node_exp> left;
  std::unique_ptr<AST_Node_exp> right;
};

struct AST_Node_assignment {
  std::unique_ptr<AST_Node_exp> left;
  std::unique_ptr<AST_Node_exp> right;
};

struct AST_Node_conditional {
  std::unique_ptr<AST_Node_exp> condition;
  std::unique_ptr<AST_Node_exp> true_expr;
  std::unique_ptr<AST_Node_exp> false_expr;
};

struct AST_Node_function_call {
  std::string identifier;
  std::vector<std::unique_ptr<AST_Node_exp>> args;
};

struct AST_Node_dereference {
  std::unique_ptr<AST_Node_exp> expr;
};

struct AST_Node_addrof {
  std::unique_ptr<AST_Node_exp> expr;
};

struct AST_Node_subscript {
  std::unique_ptr<AST_Node_exp> array;
  std::unique_ptr<AST_Node_exp> index;
};

struct AST_Node_sizeof_exp {
  std::unique_ptr<AST_Node_exp> expr;
};

struct AST_Node_sizeof_type {
  std::unique_ptr<AST_Node_type> type;
};

struct AST_Node_dot {
  std::unique_ptr<AST_Node_exp> structure;
  std::string member;
};

struct AST_Node_arrow {
  std::unique_ptr<AST_Node_exp> pointer;
  std::string member;
};

enum BlockItemType { BlockItemDeclaration, BlockItemStatement };

struct AST_Node_block_item {
  BlockItemType item_type;
  union {
    std::unique_ptr<AST_Node_declaration> declaration;
    std::unique_ptr<AST_Node_statement> statement;
  } info;
};

struct AST_Node_block {
  std::vector<std::unique_ptr<AST_Node_block_item>> items;
};

enum StmtType {
  StmtExpression,
  StmtReturn,
  StmtIf,
  StmtWhile,
  StmtDoWhile,
  StmtFor,
  StmtBreak,
  StmtContinue,
  StmtBlock,
  StmtNull
};

struct AST_Node_statement {
  StmtType stmt_type;
  union {
    std::unique_ptr<AST_Node_expression_statement> expression_statement;
    std::unique_ptr<AST_Node_return_statement> return_statement;
    std::unique_ptr<AST_Node_if_statement> if_statement;
    std::unique_ptr<AST_Node_while_statement> while_statement;
    std::unique_ptr<AST_Node_do_while_statement> do_while_statement;
    std::unique_ptr<AST_Node_for_statement> for_statement;
    std::unique_ptr<AST_Node_block> block;
  } info;
};

struct AST_Node_return_statement {
  std::optional<std::unique_ptr<AST_Node_exp>> expr;
};

struct AST_Node_expression_statement {
  std::unique_ptr<AST_Node_exp> expr;
};

struct AST_Node_if_statement {
  std::unique_ptr<AST_Node_exp> condition;
  std::unique_ptr<AST_Node_statement> true_statement;
  std::optional<std::unique_ptr<AST_Node_statement>> false_statement;
};

struct AST_Node_while_statement {
  std::unique_ptr<AST_Node_exp> condition;
  std::unique_ptr<AST_Node_statement> body;
};

struct AST_Node_do_while_statement {
  std::unique_ptr<AST_Node_statement> body;
  std::unique_ptr<AST_Node_exp> condition;
};

struct AST_Node_for_statement {
  std::unique_ptr<AST_Node_for_init> init;
  std::optional<std::unique_ptr<AST_Node_expression_statement>> condition;
  std::optional<std::unique_ptr<AST_Node_expression_statement>> increment;
  std::unique_ptr<AST_Node_statement> body;
};

enum InitType { InitExpression, InitDeclaration };

struct AST_Node_for_init {
  InitType init_type;
  union {
    std::unique_ptr<AST_Node_expression_statement> expression;
    std::unique_ptr<AST_Node_declaration> declaration;
  } info;
};

enum StroageClass { StorageClassNone, StorageClassStatic, StorageClassExtern };

enum InitializerType { InitializerExp, InitializerList };

struct AST_Node_initializer {
  InitializerType init_type;
  union {
    std::unique_ptr<AST_Node_exp> expr;
    std::vector<std::unique_ptr<AST_Node_initializer>> init_list;
  } info;
};

struct AST_Node_variable_declaration {
  std::unique_ptr<AST_Node_type> var_type;
  std::string identifier;
  std::optional<std::unique_ptr<AST_Node_initializer>> initializer;
  StroageClass storage_class;
};

struct AST_Node_struct_declaration {
  std::string tag;
  std::vector<std::unique_ptr<AST_Node_member_declaration>> members;
};

struct AST_Node_member_declaration {
  std::unique_ptr<AST_Node_type> member_type;
  std::string identifier;
};

struct AST_Node_function_declaration {
  std::string identifier;
  std::vector<std::string> params;
  std::optional<std::unique_ptr<AST_Node_block>> body;
  std::unique_ptr<AST_Node_type> fun_type;
  StroageClass storage_class;
};

enum DeclType { DeclVariable, DeclFunction, DeclStruct };

struct AST_Node_declaration {
  DeclType decl_type;
  union {
    std::unique_ptr<AST_Node_variable_declaration> variable;
    std::unique_ptr<AST_Node_function_declaration> function;
    std::unique_ptr<AST_Node_struct_declaration> structure;
  } info;
};

struct AST_Node_translation_unit {
  std::vector<std::unique_ptr<AST_Node_declaration>> declarations;
};