#pragma once
#include "../ast/ast.hpp"
#include "../parser/parser.hpp"
#include "../symbol_table/symbol_table.hpp"
#include "../token/token.hpp"
#include <memory>
#include <string>
#include <vector>

// Forward declarations
class IRProgramNode;
class IRTopLevelNode;
class IRFunctionNode;
class IRStaticVariableNode;
class IRInstructionNode;
class IRValueNode;

using IRProgramPtr = std::unique_ptr<IRProgramNode>;
using IRTopLevelPtr = std::shared_ptr<IRTopLevelNode>;
using IRFunctionPtr = std::shared_ptr<IRFunctionNode>;
using IRStaticVariablePtr = std::shared_ptr<IRStaticVariableNode>;
using IRInstructionPtr = std::shared_ptr<IRInstructionNode>;
using IRValuePtr = std::shared_ptr<IRValueNode>;

// IR Value types
enum class IRValueType { CONSTANT, VARIABLE, TEMPORARY, ARGS };

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
  std::variant<int, long int, long unsigned int, unsigned int, double, char, unsigned char> value;
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

  static IRValuePtr makeConstant(
      std::variant<int, long int, long unsigned int, unsigned int, double, char, unsigned char>
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

  static IRInstructionPtr makeReturn(IRValuePtr value) {
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
                                   IRValuePtr result) {
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
  std::variant<int, long int, long unsigned int, unsigned int, double>
      initialValue; // default to 0 if uninitialized
  IRStaticVariableNode(
      std::string id, bool isGlobal, Type varType,
      std::variant<int, long int, long unsigned int, unsigned int, double>
          init = 0)
      : identifier(std::move(id)), global(isGlobal), type(varType),
        initialValue(init) {}

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
  IRGenerator(int labelCounter) : tempCounter(), labelCounter(labelCounter) {}

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

  int tempCounter;
  int labelCounter;

  // Helper methods
  std::string generateTempName() {
    return "tmp." + std::to_string(tempCounter++);
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
};

class Valor {
public:
  Valor(int labelcounter) : generator(labelcounter) {}
  ~Valor() = default;

  IRProgramPtr convertToIR(const ASTNodePtr &ast);

private:
  IRGenerator generator;
};
