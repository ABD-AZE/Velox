#pragma once
#include <vector>
#include <string>
#include <sstream>
#include "../valor/valor.hpp"

#define TAB "    "

class ASMBase;
class ASMProgram;
class ASMFunction;
class ASMTopLevel;
class ASMStaticVariable;
class ASMInstruction;
class Operand;
class Immediate;

using ASMBasePtr = std::shared_ptr<ASMBase>;
using ASMProgramPtr = std::shared_ptr<ASMProgram>;
using ASMTopLevelPtr = std::shared_ptr<ASMTopLevel>;
using ASMStaticVariablePtr = std::shared_ptr<ASMStaticVariable>;
using ASMFunctionPtr = std::shared_ptr<ASMFunction>;
using ASMInstructionPtr = std::shared_ptr<ASMInstruction>;
using OperandPtr = std::shared_ptr<Operand>;

extern bool oneByte;
extern bool eightByte;
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
  DEALLOCATE_STACK, // arg: int 
  PUSH, // arg : operand
  CALL,  // arg: identifier
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
  CX,
  DX,
  DI,
  SI,
  R8,
  R9,
  R10,
  R11,
};

class ASMBase
{
public:
  virtual ~ASMBase() = default;
};

class ASMProgram : public ASMBase
{
public:
  std::vector<ASMTopLevelPtr> topLevelItems;
  std::string toString() const;
};

class ASMTopLevel : public ASMBase
{
public:
  virtual std::string toString() const = 0;
  virtual ~ASMTopLevel() = default;
};

class ASMFunction : public ASMTopLevel
{
public:
  std::string name;
  std::vector<ASMInstructionPtr> instructions;
  bool global;
  int stackSize = 0; // in bytes
  std::string toString() const;
};

class ASMStaticVariable : public ASMTopLevel
{
public:
  std::string name;
  bool global;
  int init;
  static std::shared_ptr<ASMStaticVariable> createStaticVariable(const std::string &name, bool global, int init)
  {
    auto var = std::make_shared<ASMStaticVariable>();
    var->name = name;
    var->global = global;
    var->init = init;
    return var;
  }
  std::string toString() const override
  {
    std::stringstream ss;
    if (global)
    {
      ss << TAB << ".globl " << name << "\n";
    }
    if (init){
      ss << TAB << ".data\n";
      ss << TAB << ".align 4\n";
      ss << name << ":\n";
      ss << TAB << ".long " << init << "\n";
    } else{
      ss << TAB << ".bss\n";
      ss << TAB << ".align 4\n";
      ss << name << ":\n";
      ss << TAB << ".zero 4\n";
    }
    return ss.str();
  }
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
    // default return val is 4 byte register name
    case RegisterType::AX:{
      if(oneByte){
        return "%al";
      } else if (eightByte){
        return "%rax";
      }
      return "%eax";
    }
    case RegisterType::R10:{
      if(oneByte){
        return "%r10b";
      } else if (eightByte){
        return "%r10";
      }
      return "%r10d";
    }
    case RegisterType::R11:{
      if(oneByte){
        return "%r11b";
      } else if (eightByte){
        return "%r11";
      }
      return "%r11d";
    }
    case RegisterType::DX:{
      if(oneByte){
        return "%dl";
      } else if (eightByte){
        return "%rdx";
      }
    return "%edx";
    }
    case RegisterType::CX:{
      if(oneByte){
        return "%cl";
      } else if (eightByte){  
        return "%rcx";
      }
      return "%ecx";
    }
    case RegisterType::DI:{
      if(oneByte){
        return "%dil";
      } else if (eightByte){
        return "%rdi";
      }
      return "%edi";
    }
    case RegisterType::SI:{
      if(oneByte){
        return "%sil";
      } else if (eightByte){
        return "%rsi";
      }
      return "%esi";
    } 
    case RegisterType::R8:{
      if(oneByte){
        return "%r8b";
      } else if (eightByte){
        return "%r8";
      }
      return "%r8d";
    }
    case RegisterType::R9:{
      if(oneByte){
        return "%r9b";
      } else if (eightByte){
        return "%r9";
      }
      return "%r9d";
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
/// Pseudo operand lets us use an arbitrary identifier as a pseudo register. Tacky(Var) = Pseudo irrespective of storage duration
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

  static std::shared_ptr<Pseudo> createPseudo(const std::string &name)
  {
    return std::make_shared<Pseudo>(name);
  }
};

/// -4(%rbp) = Stack(4)
class Stack : public Operand
{
public:
  int offset;
  Stack(int offset) : offset(offset) {}
  std::string toString() const override
  {
    return std::to_string(-offset) + "(%rbp)";
  }
  static std::shared_ptr<Stack> createStack(int offset)
  {
    return std::make_shared<Stack>(offset);
  }
};

class Data : public Operand
{
public:
  std::string name;
  Data(std::string name): name(std::move(name)) {}
  std::string toString() const override
  {
    return name + "(%rip)";
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

  static ASMInstructionPtr createDeallocateStack(int bytes)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::DEALLOCATE_STACK;
    instr->src1 = Immediate::createImmediate(bytes);
    return instr;
  }

  static ASMInstructionPtr createPush(const OperandPtr &src)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::PUSH;
    instr->src1 = src;
    return instr;
  }

  static ASMInstructionPtr createCall(const std::string &label)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::CALL;
    instr->label = label;
    return instr;
  }
};
