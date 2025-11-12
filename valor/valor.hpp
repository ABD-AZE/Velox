#pragma once
#include "../ast/ast.hpp"
#include "../parser/parser.hpp"
#include "../symbol_table/symbol_table.hpp"
#include "../token/token.hpp"
#include <cstring>
#include <memory>
#include <string>
#include <vector>

// Forward declarations
class IRProgramNode;
class IRTopLevelNode;
class IRFunctionNode;
class IRStaticVariableNode;
class IRStaticConstantNode;
class IRInstructionNode;
class IRValueNode;

using IRProgramPtr = std::unique_ptr<IRProgramNode>;
using IRTopLevelPtr = std::shared_ptr<IRTopLevelNode>;
using IRFunctionPtr = std::shared_ptr<IRFunctionNode>;
using IRStaticVariablePtr = std::shared_ptr<IRStaticVariableNode>;
using IRStaticConstantPtr = std::shared_ptr<IRStaticConstantNode>;
using IRInstructionPtr = std::shared_ptr<IRInstructionNode>;
using IRValuePtr = std::shared_ptr<IRValueNode>;

// Expression result types (for handling lvalue conversion)
enum class ExpResultType { PLAIN_OPERAND, DEREFERENCED_POINTER };

struct ExpResult {
  ExpResultType type;
  IRValuePtr value;

  static ExpResult makePlainOperand(IRValuePtr val) {
    return {ExpResultType::PLAIN_OPERAND, std::move(val)};
  }

  static ExpResult makeDereferencedPointer(IRValuePtr ptr) {
    return {ExpResultType::DEREFERENCED_POINTER, std::move(ptr)};
  }
};

// IR Value types
enum class IRValueType { CONSTANT, VARIABLE, TEMPORARY, ARGS };

// Static initializer types
enum class StaticInitKind {
  INT_INIT,     // int
  LONG_INIT,    // long
  UINT_INIT,    // unsigned int
  ULONG_INIT,   // unsigned long
  CHAR_INIT,    // char
  UCHAR_INIT,   // unsigned char
  DOUBLE_INIT,  // double
  ZERO_INIT,    // Zero-initialized (for tentative definitions)
  STRING_INIT,  // ASCII string initializer
  POINTER_INIT, // Pointer to static object
  COMPOUND      // Compound initializer (list of static_init)
};

// Forward declaration for recursive structure
struct StaticInit;

// Compound initializer
struct CompoundStaticInit {
  std::vector<StaticInit> initializers;
};

// String initializer
struct StringStaticInit {
  std::string value;
  bool null_terminated;
};

// Pointer initializer
struct PointerStaticInit {
  std::string name; // Name of the static object being pointed to
};

// Static initializer variant
struct StaticInit {
  StaticInitKind kind;
  std::variant<int,               // INT_INIT and ZERO_INIT
               long int,          // LONG_INIT
               long unsigned int, // ULONG_INIT
               unsigned int,      // UINT_INIT
               char,              // CHAR_INIT
               unsigned char,     // UCHAR_INIT
               double,            // DOUBLE_INIT
               StringStaticInit,  // STRING_INIT
               PointerStaticInit, // POINTER_INIT
               CompoundStaticInit // COMPOUND
               >
      data;

  // Factory methods
  static StaticInit makeIntInit(int value) {
    StaticInit init;
    init.kind = StaticInitKind::INT_INIT;
    init.data = value;
    return init;
  }

  static StaticInit makeLongInit(long value) {
    StaticInit init;
    init.kind = StaticInitKind::LONG_INIT;
    init.data = (value);
    return init;
  }

  static StaticInit makeUIntInit(unsigned int value) {
    StaticInit init;
    init.kind = StaticInitKind::UINT_INIT;
    init.data = (value);
    return init;
  }

  static StaticInit makeULongInit(unsigned long value) {
    StaticInit init;
    init.kind = StaticInitKind::ULONG_INIT;
    init.data = (value);
    return init;
  }

  static StaticInit makeCharInit(char value) {
    StaticInit init;
    init.kind = StaticInitKind::CHAR_INIT;
    init.data = (value);
    return init;
  }

  static StaticInit makeUCharInit(unsigned char value) {
    StaticInit init;
    init.kind = StaticInitKind::UCHAR_INIT;
    init.data = (value);
    return init;
  }

  static StaticInit makeDoubleInit(double value) {
    StaticInit init;
    init.kind = StaticInitKind::DOUBLE_INIT;
    init.data = value;
    return init;
  }

  static StaticInit makeZeroInit(int numBytes) {
    StaticInit init;
    init.kind = StaticInitKind::ZERO_INIT;
    init.data = numBytes;
    return init;
  }

  static StaticInit makeStringInit(std::string value, bool null_terminated) {
    StaticInit init;
    init.kind = StaticInitKind::STRING_INIT;
    init.data = StringStaticInit{std::move(value), null_terminated};
    return init;
  }

  static StaticInit makePointerInit(std::string name) {
    StaticInit init;
    init.kind = StaticInitKind::POINTER_INIT;
    init.data = PointerStaticInit{std::move(name)};
    return init;
  }

  static StaticInit makeCompound(std::vector<StaticInit> inits) {
    StaticInit init;
    init.kind = StaticInitKind::COMPOUND;
    init.data = CompoundStaticInit{std::move(inits)};
    return init;
  }

  // Generic factory method for variant types (for backward compatibility)
  static StaticInit
  makeInitial(std::variant<int, long int, long unsigned int, unsigned int, char,
                           unsigned char, double>
                  value) {
    return std::visit(
        [](auto &&val) -> StaticInit {
          using T = std::decay_t<decltype(val)>;
          if constexpr (std::is_same_v<T, int>) {
            return makeIntInit(val);
          } else if constexpr (std::is_same_v<T, long int>) {
            return makeLongInit(val);
          } else if constexpr (std::is_same_v<T, unsigned int>) {
            return makeUIntInit(val);
          } else if constexpr (std::is_same_v<T, long unsigned int>) {
            return makeULongInit(val);
          } else if constexpr (std::is_same_v<T, char>) {
            return makeCharInit(val);
          } else if constexpr (std::is_same_v<T, unsigned char>) {
            return makeUCharInit(val);
          } else if constexpr (std::is_same_v<T, double>) {
            return makeDoubleInit(val);
          }
        },
        value);
  }
};

struct StaticInitComparator {
  bool operator()(const StaticInit &a, const StaticInit &b) const {
    // First compare by kind
    if (a.kind != b.kind) {
      return a.kind < b.kind;
    }

    // Same kind, now compare values
    switch (a.kind) {
    case StaticInitKind::INT_INIT:
      return std::get<int>(a.data) < std::get<int>(b.data);

    case StaticInitKind::LONG_INIT:
      return std::get<long>(a.data) < std::get<long>(b.data);

    case StaticInitKind::UINT_INIT:
      return std::get<unsigned int>(a.data) < std::get<unsigned int>(b.data);

    case StaticInitKind::ULONG_INIT:
      return std::get<unsigned long>(a.data) < std::get<unsigned long>(b.data);

    case StaticInitKind::DOUBLE_INIT: {
      // For doubles, use memcmp to distinguish between 0.0 and -0.0
      double val_a = std::get<double>(a.data);
      double val_b = std::get<double>(b.data);

      // Use bit representation to compare (distinguishes +0.0 from -0.0)
      uint64_t bits_a, bits_b;
      std::memcpy(&bits_a, &val_a, sizeof(double));
      std::memcpy(&bits_b, &val_b, sizeof(double));

      return bits_a < bits_b;
    }

    case StaticInitKind::ZERO_INIT:
      // All zero initializers are equal
      return false;

    case StaticInitKind::CHAR_INIT:
      return std::get<char>(a.data) < std::get<char>(b.data);

    case StaticInitKind::UCHAR_INIT:
      return std::get<unsigned char>(a.data) < std::get<unsigned char>(b.data);

    case StaticInitKind::STRING_INIT: {
      auto &str_a = std::get<StringStaticInit>(a.data);
      auto &str_b = std::get<StringStaticInit>(b.data);
      if (str_a.value != str_b.value) {
        return str_a.value < str_b.value;
      }
      return str_a.null_terminated < str_b.null_terminated;
    }

    case StaticInitKind::POINTER_INIT: {
      auto &ptr_a = std::get<PointerStaticInit>(a.data);
      auto &ptr_b = std::get<PointerStaticInit>(b.data);
      return ptr_a.name < ptr_b.name;
    }

    case StaticInitKind::COMPOUND: {
      auto &comp_a = std::get<CompoundStaticInit>(a.data);
      auto &comp_b = std::get<CompoundStaticInit>(b.data);
      if (comp_a.initializers.size() != comp_b.initializers.size()) {
        return comp_a.initializers.size() < comp_b.initializers.size();
      }
      size_t min_size =
          std::min(comp_a.initializers.size(), comp_b.initializers.size());
      for (size_t i = 0; i < min_size; ++i) {
        if (StaticInitComparator()(comp_a.initializers[i],
                                   comp_b.initializers[i])) {
          return true;
        } else if (StaticInitComparator()(comp_b.initializers[i],
                                          comp_a.initializers[i])) {
          return false;
        }
      }
      return false;
    }
    default:
      return false;
    }
  }
};

// IR Instruction types
enum class IROpType {
  // Unary operations
  COMPLEMENT,
  NEGATE,
  NOT,
  // Type conversion operations
  SIGN_EXTEND,
  TRUNCATE,
  ZERO_EXTEND, // for unsigned types
  DOUBLE_TO_LONG,
  DOUBLE_TO_ULONG,
  LONG_TO_DOUBLE,
  ULONG_TO_DOUBLE,
  // Pointer operations
  GET_ADDRESS,
  LOAD,
  STORE,
  ADD_PTR,
  COPY_TO_OFFSET,
  // Binary operations
  ADD,
  SUBTRACT,
  MULTIPLY,
  DIVIDE,
  REMAINDER,
  AND,
  OR,
  XOR,
  LEFT_SHIFT,
  RIGHT_SHIFT,
  EQUAL,
  NOT_EQUAL,
  LESS_THAN,
  LESS_EQUAL,
  GREATER_THAN,
  GREATER_EQUAL,
  LOGICAL_AND,
  LOGICAL_OR,
  // Control flow
  RETURN,
  COPY,
  JUMP,
  JUMP_IF_ZERO,
  JUMP_IF_NOT_ZERO,
  // Function calls
  CALL,
  LABEL
};

class IRValueNode {
public:
  IRValueType type;
  TypeKind constType;
  std::variant<int, long int, long unsigned int, unsigned int, double, char,
               unsigned char>
      value;
  std::string name;
  std::vector<IRValuePtr> args;
  // Constructors
  // copy ctr
  IRValueNode(IRValuePtr other) {
    type = other->type;
    value = other->value;
    name = other->name;
    args = other->args;
  }

  IRValueNode() = default;

  static IRValuePtr
  makeConstant(std::variant<int, long int, long unsigned int, unsigned int,
                            double, char, unsigned char>
                   value) {
    auto val = std::make_shared<IRValueNode>();
    val->type = IRValueType::CONSTANT;
    val->value = value;
    val->constType = std::visit(
        [&](auto &&val) {
          using T = std::decay_t<decltype(val)>;
          if constexpr (std::is_same_v<T, int>)
            return TypeKind::INT;
          else if constexpr (std::is_same_v<T, long int>)
            return TypeKind::LONG;
          else if constexpr (std::is_same_v<T, unsigned int>)
            return TypeKind::UINT;
          else if constexpr (std::is_same_v<T, long unsigned int>)
            return TypeKind::ULONG;
          else if constexpr (std::is_same_v<T, double>)
            return TypeKind::DOUBLE;
          else
            return TypeKind::ERROR;
        },
        val->value);
    return val;
  }

  static IRValuePtr makeVariable(const std::string &varName) {
    auto val = std::make_shared<IRValueNode>();
    val->type = IRValueType::VARIABLE;
    val->name = varName;
    return val;
  }

  static IRValuePtr makeTemporary(const std::string &tempName) {
    auto val = std::make_shared<IRValueNode>();
    val->type = IRValueType::TEMPORARY;
    val->name = tempName;
    return val;
  }

  static IRValuePtr makeArgs(const std::vector<IRValuePtr> &arguments) {
    auto val = std::make_shared<IRValueNode>();
    val->type = IRValueType::ARGS;
    val->args = arguments;
    return val;
  }

  std::string toString() const {
    switch (type) {
    case IRValueType::CONSTANT:
      return std::visit([](auto &&arg) { return std::to_string(arg); }, value);
    case IRValueType::VARIABLE:
    case IRValueType::TEMPORARY:
      return name;
    case IRValueType::ARGS: {
      std::string argList = "[";
      for (size_t i = 0; i < args.size(); i++) {
        argList += args[i]->toString();
        if (i != args.size() - 1) {
          argList += ", ";
        }
      }
      argList += "]";
      return argList;
    }
    }
    return "";
  }
};

class IRInstructionNode {
public:
  IROpType opType;
  IRValuePtr dst;    // Destination (can be null for some operations)
  IRValuePtr src1;   // First operand
  IRValuePtr src2;   // Second operand (null for unary operations)
  std::string label; // For labels and jumps
  int scale;         // For AddPtr instruction (size of referenced type)
  int offset;        // For CopyToOffset instruction

  /*
  for function call:
  result = call(fn,[arg1,arg2,arg3,...])

  here src1 = fn
  src2 points to list of arguments
  */

  // Factory methods for different instruction types
  static IRInstructionPtr makeUnary(IROpType op, IRValuePtr dst,
                                    IRValuePtr src) {
    auto inst = std::make_shared<IRInstructionNode>();
    inst->opType = op;
    inst->dst = std::move(dst);
    inst->src1 = std::move(src);
    return inst;
  }

  static IRInstructionPtr makeBinary(IROpType op, IRValuePtr dst,
                                     IRValuePtr src1, IRValuePtr src2) {
    auto inst = std::make_shared<IRInstructionNode>();
    inst->opType = op;
    inst->dst = std::move(dst);
    inst->src1 = std::move(src1);
    inst->src2 = std::move(src2);
    return inst;
  }

  static IRInstructionPtr makeSignExtend(IRValuePtr src, IRValuePtr dst) {
    auto inst = std::make_shared<IRInstructionNode>();
    inst->opType = IROpType::SIGN_EXTEND;
    inst->src1 = std::move(src);
    inst->dst = std::move(dst);
    return inst;
  }

  static IRInstructionPtr makeTruncate(IRValuePtr src, IRValuePtr dst) {
    auto inst = std::make_shared<IRInstructionNode>();
    inst->opType = IROpType::TRUNCATE;
    inst->src1 = std::move(src);
    inst->dst = std::move(dst);
    return inst;
  }

  static IRInstructionPtr makeZeroExtend(IRValuePtr src, IRValuePtr dst) {
    auto inst = std::make_shared<IRInstructionNode>();
    inst->opType = IROpType::ZERO_EXTEND;
    inst->src1 = std::move(src);
    inst->dst = std::move(dst);
    return inst;
  }

  static IRInstructionPtr makeDoubleToLong(IRValuePtr src, IRValuePtr dst) {
    auto inst = std::make_shared<IRInstructionNode>();
    inst->opType = IROpType::DOUBLE_TO_LONG;
    inst->src1 = std::move(src);
    inst->dst = std::move(dst);
    return inst;
  }

  static IRInstructionPtr makeDoubleToULong(IRValuePtr src, IRValuePtr dst) {
    auto inst = std::make_shared<IRInstructionNode>();
    inst->opType = IROpType::DOUBLE_TO_ULONG;
    inst->src1 = std::move(src);
    inst->dst = std::move(dst);
    return inst;
  }

  static IRInstructionPtr makeLongToDouble(IRValuePtr src, IRValuePtr dst) {
    auto inst = std::make_shared<IRInstructionNode>();
    inst->opType = IROpType::LONG_TO_DOUBLE;
    inst->src1 = std::move(src);
    inst->dst = std::move(dst);
    return inst;
  }

  static IRInstructionPtr makeULongToDouble(IRValuePtr src, IRValuePtr dst) {
    auto inst = std::make_shared<IRInstructionNode>();
    inst->opType = IROpType::ULONG_TO_DOUBLE;
    inst->src1 = std::move(src);
    inst->dst = std::move(dst);
    return inst;
  }

  static IRInstructionPtr makeGetAddress(IRValuePtr src, IRValuePtr dst) {
    auto inst = std::make_shared<IRInstructionNode>();
    inst->opType = IROpType::GET_ADDRESS;
    inst->src1 = std::move(src);
    inst->dst = std::move(dst);
    return inst;
  }

  static IRInstructionPtr makeLoad(IRValuePtr src_ptr, IRValuePtr dst) {
    auto inst = std::make_shared<IRInstructionNode>();
    inst->opType = IROpType::LOAD;
    inst->src1 = std::move(src_ptr);
    inst->dst = std::move(dst);
    return inst;
  }

  static IRInstructionPtr makeStore(IRValuePtr src, IRValuePtr dst_ptr) {
    auto inst = std::make_shared<IRInstructionNode>();
    inst->opType = IROpType::STORE;
    inst->src1 = std::move(src);
    inst->dst = std::move(dst_ptr);
    return inst;
  }

  static IRInstructionPtr makeAddPtr(IRValuePtr ptr, IRValuePtr index,
                                     int scale, IRValuePtr dst) {
    auto inst = std::make_shared<IRInstructionNode>();
    inst->opType = IROpType::ADD_PTR;
    inst->src1 = std::move(ptr);
    inst->src2 = std::move(index);
    inst->scale = scale;
    inst->dst = std::move(dst);
    return inst;
  }

  static IRInstructionPtr
  makeCopyToOffset(IRValuePtr src, const std::string &dstName, int offset) {
    auto inst = std::make_shared<IRInstructionNode>();
    inst->opType = IROpType::COPY_TO_OFFSET;
    inst->src1 = std::move(src);
    inst->label = dstName; // Reusing label field for destination name
    inst->offset = offset;
    return inst;
  }

  static IRInstructionPtr makeReturn(IRValuePtr value = nullptr) {
    auto inst = std::make_shared<IRInstructionNode>();
    inst->opType = IROpType::RETURN;
    inst->src1 = std::move(value);
    return inst;
  }

  static IRInstructionPtr makeLabel(const std::string &labelName) {
    auto inst = std::make_shared<IRInstructionNode>();
    inst->opType = IROpType::LABEL;
    inst->label = labelName;
    return inst;
  }

  static IRInstructionPtr makeCopy(IRValuePtr src, IRValuePtr dst) {
    auto inst = std::make_shared<IRInstructionNode>();
    inst->opType = IROpType::COPY;
    inst->src1 = std::move(src);
    inst->dst = std::move(dst);
    return inst;
  }

  static IRInstructionPtr makeJump(const std::string &target) {
    auto inst = std::make_shared<IRInstructionNode>();
    inst->opType = IROpType::JUMP;
    inst->label = target;
    return inst;
  }

  static IRInstructionPtr makeJumpIfZero(IRValuePtr condition,
                                         const std::string &target) {
    auto inst = std::make_shared<IRInstructionNode>();
    inst->opType = IROpType::JUMP_IF_ZERO;
    inst->src1 = std::move(condition);
    inst->label = target;
    return inst;
  }

  static IRInstructionPtr makeJumpIfNotZero(IRValuePtr condition,
                                            const std::string &target) {
    auto inst = std::make_shared<IRInstructionNode>();
    inst->opType = IROpType::JUMP_IF_NOT_ZERO;
    inst->src1 = std::move(condition);
    inst->label = target;
    return inst;
  }

  static IRInstructionPtr makeCall(IRValuePtr function, IRValuePtr arguments,
                                   IRValuePtr result = nullptr) {
    auto inst = std::make_shared<IRInstructionNode>();
    inst->opType = IROpType::CALL;
    inst->src1 = std::move(function);
    inst->src2 = std::move(arguments);
    inst->dst = std::move(result);
    return inst;
  }

  std::string toString() const;
};

// Base class for top-level IR nodes (functions and static variables)
class IRTopLevelNode {
public:
  virtual ~IRTopLevelNode() = default;
  virtual std::string toString() const = 0;
};

class IRFunctionNode : public IRTopLevelNode {
public:
  std::string identifier;
  bool global;
  std::vector<std::string> parameters;
  std::vector<IRInstructionPtr> instructions;

  IRFunctionNode(std::string id, bool isGlobal = true)
      : identifier(std::move(id)), global(isGlobal) {}

  void addInstruction(IRInstructionPtr instruction) {
    instructions.push_back(std::move(instruction));
  }

  std::string toString() const override;
};

// Represents both external and local static variables
class IRStaticVariableNode : public IRTopLevelNode {
public:
  std::string identifier;
  bool global;
  Type type;
  std::vector<StaticInit> init_list;

  IRStaticVariableNode(std::string id, bool isGlobal, Type varType,
                       std::vector<StaticInit> inits = {})
      : identifier(std::move(id)), global(isGlobal), type(varType),
        init_list(std::move(inits)) {}

  std::string toString() const override;
};

// Represents constant strings (read-only data)
class IRStaticConstantNode : public IRTopLevelNode {
public:
  std::string identifier;
  Type type;
  std::vector<StaticInit> init_list;

  IRStaticConstantNode(std::string id, Type constType,
                       std::vector<StaticInit> inits = {})
      : identifier(std::move(id)), type(constType),
        init_list(std::move(inits)) {}

  std::string toString() const override;
};

class IRProgramNode {
public:
  std::vector<IRTopLevelPtr> topLevelItems;

  void addTopLevel(IRTopLevelPtr item) {
    topLevelItems.push_back(std::move(item));
  }

  void addFunction(IRFunctionPtr function) {
    topLevelItems.push_back(std::static_pointer_cast<IRTopLevelNode>(function));
  }

  void addStaticVariable(IRStaticVariablePtr variable) {
    topLevelItems.push_back(std::static_pointer_cast<IRTopLevelNode>(variable));
  }

  std::string toString() const;
};

// IR Generator using visitor pattern
class IRGenerator : public ASTVisitor {
public:
  IRGenerator(int labelCounter)
      : tempCounter(), labelCounter(labelCounter), stringCounter(0) {}

  IRProgramPtr generateIR(const ASTNodePtr &ast);

  // Visitor methods
  void visit(Type &node) override;
  void visit(ProgramNode &node) override;
  void visit(FunctionDefinitionNode &node) override;
  void visit(BlockItemNode &node) override;
  void visit(DeclarationNode &node) override;
  void visit(BlockNode &node) override;
  void visit(FunDeclNode &node) override;
  void visit(VarDeclNode &node) override;
  void visit(StructDeclarationNode &node) override;
  void visit(MemberDeclarationNode &node) override;
  void visit(FunctionCallNode &node) override;
  void visit(Ident &node) override;
  void visit(DeclaratorNode &node) override;
  void visit(PointerDeclarator &node) override;
  void visit(FunDeclarator &node) override;
  void visit(paraminfo &node) override;
  void visit(AbstractPointer &node) override;
  void visit(AbstractBase &node) override;

  // Statement visitors
  void visit(ReturnStatement &node) override;
  void visit(NullStatement &node) override;
  void visit(ExpressionStatement &node) override;
  void visit(IfStatement &node) override;
  void visit(GotoStatement &node) override;
  void visit(LabelStatement &node) override;
  void visit(CompoundStatement &node) override;

  // Expression visitors
  void visit(BinaryExpression &node) override;
  void visit(UnaryExpression &node) override;
  void visit(ConstantExpression &node) override;
  void visit(VariableExpression &node) override;
  void visit(AssignmentExpression &node) override;
  void visit(PostfixExpression &node) override;
  void visit(ConditionalExpression &node) override;
  void visit(CastExpression &node) override;
  void visit(DereferenceExpression &node) override;
  void visit(AddressOfExpression &node) override;
  void visit(StringLiteralExpression &node) override;
  void visit(SizeofExpression &node) override;
  void visit(SizeofTypeExpression &node) override;
  void visit(DotExpression &node) override;
  void visit(ArrowExpression &node) override;

  // Loop visitors
  void visit(ForInit &node) override;
  void visit(InitDecl &node) override;
  void visit(InitExp &node) override;
  void visit(BreakNode &node) override;
  void visit(ContinueNode &node) override;
  void visit(WhileNode &node) override;
  void visit(DoWhileNode &node) override;
  void visit(ForNode &node) override;
  void visit(ArrayDeclarator &node) override;
  void visit(AbstractArray &node) override;
  void visit(InitializerNode &node) override;
  void visit(SubscriptExpression &node) override;

private:
  IRProgramPtr program;
  std::shared_ptr<IRFunctionNode> currentFunction;
  std::shared_ptr<IRValueNode> currentValue; // For expression results
  ExpResult currentExpResult; // For tracking expression results (plain vs
                              // dereferenced)
  bool needsLvalueConversion; // Flag to control lvalue conversion

  int tempCounter;
  int labelCounter;
  int stringCounter;

  // Helper methods
  std::string generateTempName() {
    return "tmp." + std::to_string(tempCounter++);
  }

  std::string generateStringName() {
    return "string." + std::to_string(stringCounter++);
  }

  std::string generateLabelName() {
    return "label." + std::to_string(labelCounter++);
  }

  // Convert symbol table entries to static variables
  void convertSymbolTableToIR();

  IROpType tokenTypeToBinaryIR(TokenType tokenType);
  IROpType tokenTypeToUnaryIR(TokenType tokenType);
  IRValuePtr createTemporary();

  // Helper function to create temporary variables with type tracking
  IRValuePtr makeTackyVariable(Type varType);

  // Helper functions for lvalue conversion
  IRValuePtr convertExpResult(const ExpResult &result, const Type &exprType);

  // Helper function to calculate type size in bytes
  int getTypeSize(const Type &type);

  // Helper function to process compound initializers
  void processCompoundInitializer(InitializerNode *init,
                                  const std::string &varName,
                                  const Type &varType, int baseOffset);

  // Helper function to recursively pad arrays with zeros
  void padArrayWithZeros(const std::string &varName, const Type &arrayType,
                         int baseOffset);

  // Helper function to create zero-initialized StaticInit for array types
  StaticInit createZeroStaticInit(const Type &type);

  // Helper function to convert InitializerNode to StaticInit
  StaticInit convertToStaticInit(InitializerNode *init,
                                 const Type *arrayType = nullptr);
};

class Valor {
public:
  Valor(int labelcounter) : generator(labelcounter) {}
  ~Valor() = default;

  IRProgramPtr convertToIR(const ASTNodePtr &ast);

private:
  IRGenerator generator;
};
