#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <variant>

// ===== Enums (unchanged) =====
enum ConstType {
  ConstInt, ConstLong, ConstUInt, ConstULong, ConstDouble, ConstChar, ConstUChar
};

enum BinOpType {
  BinOpAdd, BinOpSub, BinOpMul, BinOpDiv, BinOpMod, BinOpAnd, BinOpOr, BinOpXor,
  BinOpShl, BinOpShr, LogicalAnd, LogicalOr, Equal, NotEqual, LessThan, GreaterThan,
  LessEqual, GreaterEqual
};

enum UnOpType { UnOpNeg, UnOpNot, UnOpCompl };

enum TypeType {
  TypeChar, TypeSChar, TypeUChar, TypeInt, TypeLong, TypeUInt, TypeULong, TypeDouble,
  TypeVoid, TypeFunction, TypePointer, TypeArray, TypeStructure, TypeUnion, TypeEnum,
  TypeNamedtype, TypeClass, TypeVaList
};

enum ExprType {
  ExprConstant, ExprString, ExprVar, ExprCast, ExprUnary, ExprPreInc, ExprPreDec,
  ExprPostInc, ExprPostDec, ExprBinary, ExprAssignment, ExprConditional, ExprFunctionCall,
  ExprDereference, ExprAddrof, ExprSubscript, ExprSizeof, ExprSizeofType, ExprDot, ExprArrow
};

// ===== Forward declarations (kept; order matters for unique_ptr to incomplete types) =====
struct AST_Node_exp;
struct AST_Node_type;
struct AST_Node_type_pointer;
struct AST_Node_type_array;
struct AST_Node_type_structure;
struct AST_Node_type_function;
struct AST_Node_array_size;

struct AST_Node_const;
struct AST_Node_string;
struct AST_Node_var;
struct AST_Node_cast;
struct AST_Node_unary_exp;
struct AST_Node_binary_exp;
struct AST_Node_assignment;
struct AST_Node_conditional;
struct AST_Node_function_call;
struct AST_Node_subscript;
struct AST_Node_VaStart;
struct AST_Node_VaEnd;
struct AST_Node_VaArg;
struct AST_Node_dot;
struct AST_Node_arrow;

struct AST_Node_declaration;
struct AST_Node_statement;
struct AST_Node_expression_statement;
struct AST_Node_return_statement;
struct AST_Node_if_statement;
struct AST_Node_while_statement;
struct AST_Node_until_statement;
struct AST_Node_do_while_statement;
struct AST_Node_for_statement;
struct AST_Node_block;
struct AST_Node_block_item;
struct AST_Node_labeled_statement;
struct AST_Node_switch_statement;
struct AST_Node_for_init;

struct AST_Node_member_declaration;
struct AST_Node_struct_declaration;
struct AST_Node_union_declaration;
struct AST_Node_enum_declaration;
struct AST_Node_enumerator;
struct AST_Node_typedef_entry;
struct AST_Node_typedef_declaration;

struct AST_Node_label;

struct AST_Node_field_member;
struct AST_Node_method_member;
struct AST_Node_constructor_member;
struct AST_Node_class_member;
struct AST_Node_class_declaration;

struct AST_Node_program;

// ===== Leaf/simple nodes (unchanged) =====
struct AST_Node_const {
  ConstType type{};
  union {
    int intValue;
    long longValue;
    unsigned int uintValue;
    unsigned long ulongValue;
    double doubleValue;
    char charValue;
    unsigned char ucharValue;
  } value{};
};

struct AST_Node_binary_operator { BinOpType op{}; };
struct AST_Node_unary_operator { UnOpType op{}; };

// ===== Types (variant replaces union) =====
struct AST_Node_type {
  TypeType type{};
  bool is_const{false};
  bool is_static{false};
  bool is_extern{false};

  using TypeInfo = std::variant<
    std::monostate,
    std::unique_ptr<AST_Node_type_pointer>,
    std::unique_ptr<AST_Node_type_array>,
    std::unique_ptr<AST_Node_type_structure>,
    std::unique_ptr<AST_Node_type_function>,
    std::string // tag (struct/union/enum/class/typedef name)
  >;

  TypeInfo info{};
};

struct AST_Node_type_pointer {
  std::unique_ptr<AST_Node_type> referenced;
  int level{0};
};

enum ArraySizeType { ArraySizeConst, ArraySizeVar, ArraySizeUnspecified };

struct AST_Node_array_size {
  ArraySizeType size_type{ArraySizeUnspecified};
  using SizeInfo = std::variant<
    std::monostate,                       // Unspecified
    std::unique_ptr<AST_Node_const>,      // Known (const size)
    std::unique_ptr<AST_Node_exp>         // VLA (variable length)
  >;
  SizeInfo info{};
};

struct AST_Node_type_array {
  std::unique_ptr<AST_Node_type> element;
  std::unique_ptr<AST_Node_array_size> size; // may hold monostate
};

struct AST_Node_type_structure { std::string tag; };

struct AST_Node_type_function {
  std::vector<std::unique_ptr<AST_Node_type>> params;
  std::unique_ptr<AST_Node_type> ret;
};

// ===== Expressions (variant replaces union) =====
struct AST_Node_string { std::string value; };
struct AST_Node_var { std::string identifier; };

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

enum AssignOpType {
  Assign, AddAssign, SubAssign, MulAssign, DivAssign, ModAssign,
  AndAssign, OrAssign, XorAssign, ShlAssign, ShrAssign
};

struct AST_Node_assignment {
  std::unique_ptr<AST_Node_exp> left;
  AssignOpType op{Assign};
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

struct AST_Node_subscript {
  std::unique_ptr<AST_Node_exp> array;
  std::unique_ptr<AST_Node_exp> index;
};

struct AST_Node_dot {
  std::unique_ptr<AST_Node_exp> structure;
  std::string member;
};

struct AST_Node_arrow {
  std::unique_ptr<AST_Node_exp> pointer;
  std::string member;
};

struct AST_Node_VaStart { std::unique_ptr<AST_Node_exp> ap; std::string last_param; };
struct AST_Node_VaEnd   { std::unique_ptr<AST_Node_exp> ap; };
struct AST_Node_VaArg   { std::unique_ptr<AST_Node_exp> ap; std::unique_ptr<AST_Node_type> target_type; };

struct AST_Node_exp {
  ExprType expr_type{};

  using ExpInfo = std::variant<
    std::monostate,
    std::unique_ptr<AST_Node_const>,
    std::unique_ptr<AST_Node_string>,
    std::unique_ptr<AST_Node_var>,
    std::unique_ptr<AST_Node_cast>,
    std::unique_ptr<AST_Node_unary_exp>,
    std::unique_ptr<AST_Node_exp>,          // nested expr (pre/post inc/dec etc.)
    std::unique_ptr<AST_Node_binary_exp>,
    std::unique_ptr<AST_Node_assignment>,
    std::unique_ptr<AST_Node_conditional>,
    std::unique_ptr<AST_Node_function_call>,
    std::unique_ptr<AST_Node_subscript>,
    std::unique_ptr<AST_Node_type>,         // sizeof(type)
    std::unique_ptr<AST_Node_VaStart>,
    std::unique_ptr<AST_Node_VaEnd>,
    std::unique_ptr<AST_Node_VaArg>,
    std::unique_ptr<AST_Node_dot>,
    std::unique_ptr<AST_Node_arrow>
  >;

  ExpInfo info{};
};

// ===== Blocks & statements (variant replaces union) =====
enum BlockItemType { BlockItemDeclaration, BlockItemStatement };

struct AST_Node_block_item {
  BlockItemType item_type{BlockItemStatement};
  using BlockItemInfo = std::variant<
    std::monostate,
    std::unique_ptr<AST_Node_declaration>,
    std::unique_ptr<AST_Node_statement>
  >;
  BlockItemInfo info{};
};

struct AST_Node_block {
  std::vector<std::unique_ptr<AST_Node_block_item>> items;
};

enum StmtType {
  StmtExpression, StmtReturn, StmtIf, StmtWhile, StmtDoWhile, StmtFor, StmtUntil,
  StmtBreak, StmtContinue, StmtBlock, StmtNull, StmtLabeled, StmtSwitch, StmtGoto
};

enum LabelType { LabelIdentifier, LabelCase, LabelDefault };

struct AST_Node_label {
  LabelType label_type{LabelIdentifier};
  using LabelInfo = std::variant<
    std::monostate,
    std::string,                         // identifier
    std::unique_ptr<AST_Node_const>      // case value
  >;
  LabelInfo info{};
};

struct AST_Node_labeled_statement {
  std::vector<std::string> label;
  std::unique_ptr<AST_Node_statement> statement;
};

struct AST_Node_switch_statement {
  std::unique_ptr<AST_Node_exp> selector;
  std::unique_ptr<AST_Node_statement> body;
};

struct AST_Node_return_statement { std::optional<std::unique_ptr<AST_Node_exp>> expr; };
struct AST_Node_expression_statement { std::unique_ptr<AST_Node_exp> expr; };

struct AST_Node_if_statement {
  std::unique_ptr<AST_Node_exp> condition;
  std::unique_ptr<AST_Node_statement> true_statement;
  std::optional<std::unique_ptr<AST_Node_statement>> false_statement;
};

struct AST_Node_while_statement {
  std::unique_ptr<AST_Node_exp> condition;
  std::unique_ptr<AST_Node_statement> body;
};

struct AST_Node_until_statement {
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
  InitType init_type{InitExpression};
  using ForInitInfo = std::variant<
    std::monostate,
    std::unique_ptr<AST_Node_expression_statement>,
    std::unique_ptr<AST_Node_declaration>
  >;
  ForInitInfo info{};
};

// ===== Initializers (variant replaces union) =====
enum StroageClass { StorageClassNone, StorageClassStatic, StorageClassExtern };
enum InitializerType { InitializerExp, InitializerList, InitializerCtor };

struct AST_Node_initializer {
  InitializerType init_type{InitializerExp};
  using InitializerInfo = std::variant<
    std::monostate,
    std::unique_ptr<AST_Node_exp>,
    std::vector<std::unique_ptr<AST_Node_initializer>>
  >;
  InitializerInfo info{};
};

// ===== Declarations (unchanged shapes + variant unions) =====
struct AST_Node_variable_declaration {
  std::unique_ptr<AST_Node_type> var_type;
  std::string identifier;
  std::optional<std::unique_ptr<AST_Node_initializer>> initializer;
  StroageClass storage_class{StorageClassNone};
};

struct AST_Node_member_declaration {
  std::unique_ptr<AST_Node_type> member_type;
  std::string identifier;
};

struct AST_Node_struct_declaration {
  std::string tag;
  std::vector<std::unique_ptr<AST_Node_member_declaration>> members;
};

struct AST_Node_function_declaration {
  std::string identifier;
  std::vector<std::string> params;
  std::optional<std::unique_ptr<AST_Node_block>> body;
  std::unique_ptr<AST_Node_type> fun_type;
  StroageClass storage_class{StorageClassNone};
};

struct AST_Node_union_declaration {
  std::string tag;
  std::vector<std::unique_ptr<AST_Node_member_declaration>> members;
};

struct AST_Node_enumerator {
  std::string name;
  std::optional<std::unique_ptr<AST_Node_const>> value;
};

struct AST_Node_enum_declaration {
  std::optional<std::string> tag;
  std::vector<std::unique_ptr<AST_Node_enumerator>> enumerators;
};

struct AST_Node_typedef_entry {
  std::string name;
  std::unique_ptr<AST_Node_type> alias_type;
};

struct AST_Node_typedef_declaration {
  std::vector<std::unique_ptr<AST_Node_typedef_entry>> entries;
};

// ===== Classes (variant replaces union) =====
enum AccessSpec { Public, Private };

struct AST_Node_field_member {
  std::unique_ptr<AST_Node_variable_declaration> var;
  AccessSpec access{Public};
};

struct AST_Node_method_member {
  std::unique_ptr<AST_Node_function_declaration> fun;
  AccessSpec access{Public};
};

struct AST_Node_constructor_member {
  std::string class_name;
  std::vector<std::string> params; // names
  std::unique_ptr<AST_Node_type> ctor_type; // FunType(param_types, Void, false)
  std::optional<std::unique_ptr<AST_Node_block>> body;
  AccessSpec access{Public};
};

enum ClassMemberType { ClassMemberField, ClassMemberMethod, ClassMemberConstructor };

struct AST_Node_class_member {
  ClassMemberType member_type{ClassMemberField};
  using ClassMemberInfo = std::variant<
    std::monostate,
    std::unique_ptr<AST_Node_field_member>,
    std::unique_ptr<AST_Node_method_member>,
    std::unique_ptr<AST_Node_constructor_member>
  >;
  ClassMemberInfo info{};
};

struct AST_Node_class_declaration {
  std::string name;
  std::vector<std::unique_ptr<AST_Node_class_member>> members;
};

// ===== Top-level declaration (variant replaces union) =====
enum DeclType { DeclVariable, DeclFunction, DeclStruct, DeclUnion, DeclEnum, DeclTypedef, DeclClass };

struct AST_Node_declaration {
  DeclType decl_type{DeclVariable};
  using DeclInfo = std::variant<
    std::monostate,
    std::unique_ptr<AST_Node_variable_declaration>,
    std::unique_ptr<AST_Node_function_declaration>,
    std::unique_ptr<AST_Node_struct_declaration>,
    std::unique_ptr<AST_Node_union_declaration>,
    std::unique_ptr<AST_Node_enum_declaration>,
    std::unique_ptr<AST_Node_typedef_declaration>,
    std::unique_ptr<AST_Node_class_declaration>
  >;
  DeclInfo info{};
};

struct AST_Node_program {
  std::vector<std::unique_ptr<AST_Node_declaration>> declarations;
  void AddDeclaration(std::unique_ptr<AST_Node_declaration> decl) {
    declarations.push_back(std::move(decl));
  }
};

struct AST_Node_statement {
  StmtType stmt_type{StmtNull};
  using StmtInfo = std::variant<
    std::monostate,
    std::unique_ptr<AST_Node_expression_statement>,
    std::unique_ptr<AST_Node_return_statement>,
    std::unique_ptr<AST_Node_if_statement>,
    std::unique_ptr<AST_Node_while_statement>,
    std::unique_ptr<AST_Node_until_statement>,
    std::unique_ptr<AST_Node_do_while_statement>,
    std::unique_ptr<AST_Node_for_statement>,
    std::unique_ptr<AST_Node_block>,
    std::unique_ptr<AST_Node_labeled_statement>,
    std::unique_ptr<AST_Node_switch_statement>,
    std::string // goto label
  >;
  StmtInfo info{};
};
