#pragma once
#include <vector>
#include <string>
#include <sstream>
#include "../valor/valor.hpp"

class ASMBase;
class ASMProgram;
class ASMFunction;
class ASMInstruction;
class Operand;
class Immediate;

using ASMBasePtr = std::shared_ptr<ASMBase>;
using ASMProgramPtr = std::shared_ptr<ASMProgram>;
using ASMFunctionPtr = std::shared_ptr<ASMFunction>;
using ASMInstructionPtr = std::shared_ptr<ASMInstruction>;
using OperandPtr = std::shared_ptr<Operand>;

extern bool oneByte;

class Codegen
{
public:
  ASMProgramPtr generateCode(IRProgramPtr &irProgram);

private:
  ASMProgramPtr IRProgramtoASM(const IRProgramPtr &irProgram);
  ASMFunctionPtr IRFunctionToASM(const IRFunctionPtr &irFunction);
  std::vector<ASMInstructionPtr> IRInstructionToASM(const IRInstructionPtr &irInstruction);
  OperandPtr IRValueToOperand(const IRValuePtr &irValue);
  /// replaces pseudoregisters with stack locations, returns the number of bytes used on the stack
  int replacePseudoRegisters(ASMBasePtr ast);
  /* allocatestack based on the return value of replacePseudoRegister
  also fix mov instructions having both src and dst as memory operands
  */
  void finalPass(ASMBasePtr ast);
};

// assembly AST nodes

enum class ASMOpType
{
  MOV,
  UNARY,
  BINARY,
  IDIV,
  CDQ,
  ALLOCATE_STACK,
  RET,
  CMP,
  JMP,
  JMPCC,
  SETCC,
  LABEL, // NOT an instruction, but a marker for labels
};

enum class UnaryOpType
{
  NEG,
  NOT,
};

enum class BinaryOpType
{
  ADD,
  SUB,
  MULT,
  AND,
  OR,
  XOR,
  LEFT_SHIFT,
  RIGHT_SHIFT,
};

enum class ConditionCode
{
  E,
  NE,
  G,
  GE,
  L,
  LE,
};

enum class RegisterType
{
  AX,
  DX,
  R10,
  R11,
  ECX,
};

class ASMBase
{
public:
  virtual ~ASMBase() = default;
};

class ASMProgram : public ASMBase
{
public:
  std::vector<ASMFunctionPtr> functions;
  std::string toString() const;
};

class ASMFunction : public ASMBase
{
public:
  std::string name;
  std::vector<ASMInstructionPtr> instructions;
  std::string toString() const;
};


class Operand : public ASMBase
{
public:
  virtual ~Operand() = default;
  virtual std::string toString() const = 0;
};

class Reg : public Operand
{
public:
  RegisterType name;
  Reg(RegisterType name) : name(name) {}
  std::string toString() const override
  {
    switch (name)
    {
    case RegisterType::AX:{
      if(oneByte){
        return "%al";
      }
      return "%eax";
    }
    case RegisterType::R10:{
      if(oneByte){
        return "%r10b";
      }
      return "%r10d";
    }
    case RegisterType::R11:{
      if(oneByte){
        return "%r11b";
      }
      return "%r11d";
    }
    case RegisterType::DX:{
      if(oneByte){
        return "%dl";
      }
    return "%edx";
    }
    case RegisterType::ECX:{
      if(oneByte){
        return "%cl";
      }
      return "%ecx";
    }
    default:
      return "%unknown";
    }
  }
  static std::shared_ptr<Reg> createRegister(RegisterType r)
  {
    return std::make_shared<Reg>(r);
  }
};

class Immediate : public Operand
{
public:
  int value;
  Immediate(int value) : value(value) {}
  std::string toString() const override
  {
    return "$" + std::to_string(value);
  }
  static std::shared_ptr<Immediate> createImmediate(int value)
  {
    return std::make_shared<Immediate>(value);
  }
};
/// Pseudo operand lets us use an arbitrary identifier as a pseudo register
class Pseudo : public Operand
{
public:
  std::string name;

  Pseudo(std::string name) : name(std::move(name)) {}
  // useless
  std::string toString() const override
  {
    return "";
  }
};

/// -4(%rbp) = Stack(-4)
class Stack : public Operand
{
public:
  int offset;
  Stack(int offset) : offset(offset) {}
  std::string toString() const override
  {
    return std::to_string(-offset) + "(%rbp)";
  }
};

class ASMInstruction : public ASMBase
{
public:
  ASMOpType opType;
  UnaryOpType unaryOpType;
  BinaryOpType binaryOpType;
  std::string label;           // for labels and jumps
  ConditionCode conditionCode; // for conditional jumps and setcc
  OperandPtr dst;
  OperandPtr src1;
  OperandPtr src2;
  std::string toString() const;

  static ASMInstructionPtr createMov(const OperandPtr &dst, const OperandPtr &src)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::MOV;
    instr->dst = dst;
    instr->src1 = src;
    return instr;
  }

  static ASMInstructionPtr createUnary(UnaryOpType unaryOpType, const OperandPtr &src)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::UNARY;
    instr->unaryOpType = unaryOpType;
    instr->src1 = src;
    return instr;
  }

  static ASMInstructionPtr createBinary(BinaryOpType binaryOpType, const OperandPtr &dst, const OperandPtr &src2)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::BINARY;
    instr->binaryOpType = binaryOpType;
    instr->dst = dst;
    instr->src2 = src2;
    return instr;
  }

  static ASMInstructionPtr createIDiv(const OperandPtr &src)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::IDIV;
    instr->src1 = src;
    return instr;
  }

  static ASMInstructionPtr createCDQ()
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::CDQ;
    return instr;
  }

  static ASMInstructionPtr createRet()
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::RET;
    return instr;
  }

  static ASMInstructionPtr createAllocateStack(int bytes)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::ALLOCATE_STACK;
    instr->src1 = Immediate::createImmediate(bytes);
    return instr;
  }

  static ASMInstructionPtr createLabel(const std::string &label)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::LABEL;
    instr->label = label;
    return instr;
  }

  static ASMInstructionPtr createJmp(const std::string &label)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::JMP;
    instr->label = label;
    return instr;
  }

  static ASMInstructionPtr createJmpCC(ConditionCode cc, const std::string &label)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::JMPCC;
    instr->conditionCode = cc;
    instr->label = label;
    return instr;
  }

  static ASMInstructionPtr createSetCC(ConditionCode cc, const OperandPtr &dst)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::SETCC;
    instr->conditionCode = cc;
    instr->dst = dst;
    return instr;
  }

  static ASMInstructionPtr createCmp(const OperandPtr &src1, const OperandPtr &src2)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::CMP;
    instr->src1 = src1;
    instr->src2 = src2;
    return instr;
  }
};
