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
  BinOpShr,
  LogicalAnd,
  LogicalOr,
  Equal,
  NotEqual,
  LessThan,
  GreaterThan,
  LessEqual,
  GreaterEqual
};

enum UnOpType {
  UnOpNeg,
  UnOpNot,
  UnOpCompl
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
  TypeStructure,
  TypeUnion,
  TypeEnum,
  TypeNamedtype,
  TypeClass,
  TypeVaList
};

enum ExprType {
  ExprConstant,
  ExprString,
  ExprVar,
  ExprCast,
  ExprUnary,
  ExprPreInc,
  ExprPreDec,
  ExprPostInc,
  ExprPostDec,
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
    std::string tag; // For named types like struct/union/enum/class
  } info;
};

struct AST_Node_type_pointer {
  std::unique_ptr<AST_Node_type> referenced;
  int level;
};

enum ArraySizeType { ArraySizeConst, ArraySizeVar, ArraySizeUnspecified };

struct AST_Node_array_size {
  ArraySizeType size_type;
  union {
    std::unique_ptr<AST_Node_const> const_size;
    std::unique_ptr<AST_Node_exp> var_size;
  } info;
};

struct AST_Node_type_array {
  std::unique_ptr<AST_Node_type> element;
  std::unique_ptr<AST_Node_array_size> size;
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
    std::unique_ptr<AST_Node_const> constant;
    std::unique_ptr<AST_Node_string> string;
    std::unique_ptr<AST_Node_var> var;
    std::unique_ptr<AST_Node_cast> cast;
    std::unique_ptr<AST_Node_unary_exp> unary_exp;
    std::unique_ptr<AST_Node_exp> exp; // For pre/post inc/dec, dereference, addrof, sizeofexp
    std::unique_ptr<AST_Node_binary_exp> binary_exp;
    std::unique_ptr<AST_Node_assignment> assignment;
    std::unique_ptr<AST_Node_conditional> conditional;
    std::unique_ptr<AST_Node_function_call> function_call;
    std::unique_ptr<AST_Node_subscript> subscript;
    std::unique_ptr<AST_Node_type> sizeof_type; // For sizeof(type)
    std::unique_ptr<AST_Node_VaStart> va_start;
    std::unique_ptr<AST_Node_VaEnd> va_end;
    std::unique_ptr<AST_Node_VaArg> va_arg;
    std::unique_ptr<AST_Node_dot> dot;
    std::unique_ptr<AST_Node_arrow> arrow;
  } info;
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

enum AssignOpType { Assign, AddAssign, SubAssign, MulAssign, DivAssign, ModAssign, AndAssign, OrAssign, XorAssign, ShlAssign, ShrAssign };

struct AST_Node_assignment {
  std::unique_ptr<AST_Node_exp> left;
  AssignOpType op;
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

struct AST_Node_VaStart {
  std::unique_ptr<AST_Node_exp> ap;
  std::string last_param;
};

struct AST_Node_VaEnd {
  std::unique_ptr<AST_Node_exp> ap;
};

struct AST_Node_VaArg {
  std::unique_ptr<AST_Node_exp> ap;
  std::unique_ptr<AST_Node_type> target_type;
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
  StmtNull,
  StmtLabeled,
  StmtSwitch,
  StmtGoto
};

enum LabelType { LabelIdentifier, LabelCase, LabelDefault };

struct AST_Node_label {
  LabelType label_type;
  union {
    std::string identifier; // For LabelIdentifier
    std::unique_ptr<AST_Node_const> case_value; // For LabelCase
  } info;
};

struct AST_Node_labeled_statement {
  std::vector<std::string> label;
  std::unique_ptr<AST_Node_statement> statement;
};

struct AST_Node_switch_statement {
  std::unique_ptr<AST_Node_exp> selector;
  std::unique_ptr<AST_Node_statement> body;
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
    std::unique_ptr<AST_Node_labeled_statement> labeled_statement;
    std::unique_ptr<AST_Node_switch_statement> switch_statement;
    std::string goto_label; // For goto statement
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

enum InitializerType { InitializerExp, InitializerList, InitializerCtor };

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

/*
### `class_declaration`
```
class_declaration = (identifier name, class_member* members)

class_member = Field(field_member) | Method(method_member) | Ctor(constructor_member)

field_member = (variable_declaration var, access_spec access)

method_member =
  (function_declaration fun,
   access_spec access)

constructor_member =
  (identifier class_name,
   identifier* params,             // names
   type ctor_type,                 // FunType(param_types, Void, false)
   block? body,
   access_spec access)

access_spec = Public | Private    //Access checking is semantic check not a parsing concern
```
*/

enum AccessSpec { Public, Private };

struct AST_Node_field_member {
  std::unique_ptr<AST_Node_variable_declaration> var;
  AccessSpec access; // "public" or "private"
};

struct AST_Node_method_member {
  std::unique_ptr<AST_Node_function_declaration> fun;
  AccessSpec access; // "public" or "private"
};

struct AST_Node_constructor_member {
  std::string class_name;
  std::vector<std::string> params; // names
  std::unique_ptr<AST_Node_type> ctor_type; // FunType(param_types, Void, false)
  std::optional<std::unique_ptr<AST_Node_block>> body;
  AccessSpec access; // "public" or "private"
};

enum ClassMemberType { ClassMemberField, ClassMemberMethod, ClassMemberConstructor };

struct AST_Node_class_member {
  ClassMemberType member_type;
  union {
    std::unique_ptr<AST_Node_field_member> field;
    std::unique_ptr<AST_Node_method_member> method;
    std::unique_ptr<AST_Node_constructor_member> constructor;
  } info;
};

struct AST_Node_class_declaration {
  std::string name;
  std::vector<std::unique_ptr<AST_Node_class_member>> members;
};

enum DeclType { DeclVariable, DeclFunction, DeclStruct, DeclUnion, DeclEnum, DeclTypedef, DeclClass };

struct AST_Node_declaration {
  DeclType decl_type;
  union {
    std::unique_ptr<AST_Node_variable_declaration> variable;
    std::unique_ptr<AST_Node_function_declaration> function;
    std::unique_ptr<AST_Node_struct_declaration> structure;
    std::unique_ptr<AST_Node_union_declaration> union_decl;
    std::unique_ptr<AST_Node_enum_declaration> enum_decl;
    std::unique_ptr<AST_Node_typedef_declaration> typedef_decl;
    std::unique_ptr<AST_Node_class_declaration> class_decl;
  } info;
};

struct AST_Node_program {
  std::vector<std::unique_ptr<AST_Node_declaration>> declarations;
};