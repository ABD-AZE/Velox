#pragma once
#include "../valor/valor.hpp"
#include <functional>
#include <sstream>
#include <string>
#include <vector>

#define TAB "    "

class ASMBase;
class ASMProgram;
class ASMFunction;
class ASMTopLevel;
class ASMStaticVariable;
class ASMInstruction;
class Operand;
class Immediate;
class ClassifiedParams;

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
  std::vector<ASMInstructionPtr>
  IRInstructionToASM(const IRInstructionPtr &irInstruction);
  OperandPtr IRValueToOperand(const IRValuePtr &irValue);
  ClassifiedParams classifyParameters(const std::vector<IRValuePtr> &values);
  /// replaces pseudoregisters with stack locations, returns the number of bytes
  /// used on the stack
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
  DIV,
  CDQ,
  RET,
  CMP,
  JMP,
  JMPCC,
  SETCC,
  LABEL, // NOT an instruction, but a marker for labels
  PUSH,  // arg : operand
  CALL,  // arg: identifier
  MOVSX,
  MOVZEROEXTEND,
  CVTTSD2SI, // double to int/long
  CVTTSI2SD, // int/long to double
  LEA,       // load effective address
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
  UNSIGNED_RIGHT_SHIFT,
  DIVDOUBLE,
};

enum class ConditionCode
{
  E,
  NE,
  G,
  GE,
  L,
  LE,
  A,
  AE,
  B,
  BE,
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
  SP,
  BP,
  XMM0,
  XMM1,
  XMM2,
  XMM3,
  XMM4,
  XMM5,
  XMM6,
  XMM7,
  XMM8,
  XMM9,
  XMM10,
  XMM11,
  XMM12,
  XMM13,
  XMM14,
  XMM15,
};

// Helper function to classify parameters/arguments into register and stack
// categories
struct ClassifiedParams
{
  std::vector<std::pair<AssemblyType, OperandPtr>> intRegArgs;
  std::vector<OperandPtr> doubleRegArgs;
  std::vector<std::pair<AssemblyType, OperandPtr>> stackArgs;
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
  int alignment;
  std::vector<StaticInit> init;
  bool constant = false;
  static std::shared_ptr<ASMStaticVariable>
  createStaticVariable(const std::string &name, bool global, int alignment,
                       std::vector<StaticInit> init, bool constant)
  {
    auto var = std::make_shared<ASMStaticVariable>();
    var->name = name;
    var->global = global;
    var->alignment = alignment;
    var->init = init;
    var->constant = constant;
    return var;
  }

  // create a static constant (non-global)
  static std::shared_ptr<ASMStaticVariable>
  createStaticConstant(const std::string &name, int alignment,
                       std::vector<StaticInit> init)
  {
    return createStaticVariable(name, false, alignment, init, true);
  }

  std::string toString() const override
  {
    std::stringstream ss;
    if (global)
    {
      ss << TAB << ".globl " << name << "\n";
    }

    // Helper lambda to recursively print static initializers
    std::function<void(const StaticInit &)> printStaticInit;
    printStaticInit = [&](const StaticInit &staticInit)
    {
      switch (staticInit.kind)
      {
      case StaticInitKind::INT_INIT:
        ss << TAB << ".long " << std::get<int>(staticInit.data) << "\n";
        break;
      case StaticInitKind::UINT_INIT:
        ss << TAB << ".long " << std::get<unsigned int>(staticInit.data) << "\n";
        break;
      case StaticInitKind::LONG_INIT:
        ss << TAB << ".quad " << std::get<long>(staticInit.data) << "\n";
        break;
      case StaticInitKind::ULONG_INIT:
        ss << TAB << ".quad " << std::get<unsigned long>(staticInit.data) << "\n";
        break;
      case StaticInitKind::CHAR_INIT:
        ss << TAB << ".byte " << static_cast<int>(std::get<char>(staticInit.data)) << "\n";
        break;
      case StaticInitKind::UCHAR_INIT:
        ss << TAB << ".byte " << static_cast<int>(std::get<unsigned char>(staticInit.data)) << "\n";
        break;
      case StaticInitKind::DOUBLE_INIT:
      {
        union
        {
          double d;
          uint64_t u;
        } converter;
        converter.d = std::get<double>(staticInit.data);
        ss << TAB << ".quad " << converter.u << "\n";
        break;
      }
      case StaticInitKind::ZERO_INIT:
        ss << TAB << ".zero " << std::get<int>(staticInit.data) << "\n";
        break;
      case StaticInitKind::STRING_INIT:
      {
        const auto &strInit = std::get<StringStaticInit>(staticInit.data);
        ss << TAB << ".ascii \"";
        for (char c : strInit.value)
        {
          if (c == '\\')
            ss << "\\\\";
          else if (c == '"')
            ss << "\\\"";
          else if (c == '\n')
            ss << "\\n";
          else if (c == '\t')
            ss << "\\t";
          else if (c == '\r')
            ss << "\\r";
          else if (c >= 32 && c <= 126)
            ss << c;
          else
            ss << "\\x" << std::hex << (int)(unsigned char)c << std::dec;
        }
        ss << "\"\n";
        if (strInit.null_terminated)
        {
          ss << TAB << ".byte 0\n";
        }
        break;
      }
      case StaticInitKind::POINTER_INIT:
      {
        const auto &ptrInit = std::get<PointerStaticInit>(staticInit.data);
        ss << TAB << ".quad " << ptrInit.name << "\n";
        break;
      }
      case StaticInitKind::COMPOUND:
      {
        const auto &compound = std::get<CompoundStaticInit>(staticInit.data);
        for (const auto &element : compound.initializers)
        {
          printStaticInit(element);
        }
        break;
      }
      }
    };

    if (constant)
    {
      ss << TAB << ".section .rodata\n";
      ss << TAB << ".align " << alignment << "\n";
      ss << name << ":\n";
      for (const auto &staticInit : init)
      {
        printStaticInit(staticInit);
      }
      return ss.str();
    }

    // Handle first element for non-constant variables
    if (!init.empty())
    {
      const auto &firstInit = init[0];
      if (firstInit.kind == StaticInitKind::ZERO_INIT)
      {
        ss << TAB << ".bss\n";
        ss << TAB << ".align " << alignment << "\n";
        ss << name << ":\n";
        ss << TAB << ".zero " << std::get<int>(firstInit.data) << "\n";
      }
      else
      {
        ss << TAB << ".data\n";
        ss << TAB << ".align " << alignment << "\n";
        ss << name << ":\n";
        for (const auto &staticInit : init)
        {
          printStaticInit(staticInit);
        }
      }
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
    case RegisterType::AX:
    {
      if (oneByte)
      {
        return "%al";
      }
      else if (eightByte)
      {
        return "%rax";
      }
      return "%eax";
    }
    case RegisterType::R10:
    {
      if (oneByte)
      {
        return "%r10b";
      }
      else if (eightByte)
      {
        return "%r10";
      }
      return "%r10d";
    }
    case RegisterType::R11:
    {
      if (oneByte)
      {
        return "%r11b";
      }
      else if (eightByte)
      {
        return "%r11";
      }
      return "%r11d";
    }
    case RegisterType::DX:
    {
      if (oneByte)
      {
        return "%dl";
      }
      else if (eightByte)
      {
        return "%rdx";
      }
      return "%edx";
    }
    case RegisterType::CX:
    {
      if (oneByte)
      {
        return "%cl";
      }
      else if (eightByte)
      {
        return "%rcx";
      }
      return "%ecx";
    }
    case RegisterType::DI:
    {
      if (oneByte)
      {
        return "%dil";
      }
      else if (eightByte)
      {
        return "%rdi";
      }
      return "%edi";
    }
    case RegisterType::SI:
    {
      if (oneByte)
      {
        return "%sil";
      }
      else if (eightByte)
      {
        return "%rsi";
      }
      return "%esi";
    }
    case RegisterType::R8:
    {
      if (oneByte)
      {
        return "%r8b";
      }
      else if (eightByte)
      {
        return "%r8";
      }
      return "%r8d";
    }
    case RegisterType::R9:
    {
      if (oneByte)
      {
        return "%r9b";
      }
      else if (eightByte)
      {
        return "%r9";
      }
      return "%r9d";
    }
    case RegisterType::SP:
    {
      if (oneByte)
      {
        return "%spl";
      }
      else if (eightByte)
      {
        return "%rsp";
      }
      return "%esp";
    }
    case RegisterType::BP:
    {
      if (oneByte)
      {
        return "%bpl";
      }
      else if (eightByte)
      {
        return "%rbp";
      }
      return "%ebp";
    }
    case RegisterType::XMM0:
      return "%xmm0";
    case RegisterType::XMM1:
      return "%xmm1";
    case RegisterType::XMM2:
      return "%xmm2";
    case RegisterType::XMM3:
      return "%xmm3";
    case RegisterType::XMM4:
      return "%xmm4";
    case RegisterType::XMM5:
      return "%xmm5";
    case RegisterType::XMM6:
      return "%xmm6";
    case RegisterType::XMM7:
      return "%xmm7";
    case RegisterType::XMM8:
      return "%xmm8";
    case RegisterType::XMM9:
      return "%xmm9";
    case RegisterType::XMM10:
      return "%xmm10";
    case RegisterType::XMM11:
      return "%xmm11";
    case RegisterType::XMM12:
      return "%xmm12";
    case RegisterType::XMM13:
      return "%xmm13";
    case RegisterType::XMM14:
      return "%xmm14";
    case RegisterType::XMM15:
      return "%xmm15";
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
  unsigned long value;
  Immediate(unsigned long value) : value(value) {}
  std::string toString() const override { return "$" + std::to_string(value); }
  static std::shared_ptr<Immediate> createImmediate(unsigned long value)
  {
    return std::make_shared<Immediate>(value);
  }
};
/// Pseudo operand lets us use an arbitrary identifier as a pseudo register.
/// Tacky(Var) = Pseudo irrespective of storage duration
class Pseudo : public Operand
{
public:
  std::string name;

  Pseudo(std::string name) : name(std::move(name)) {}
  // useless
  std::string toString() const override { return ""; }

  static std::shared_ptr<Pseudo> createPseudo(const std::string &name)
  {
    return std::make_shared<Pseudo>(name);
  }
};

/// -4(%rbp) = Memory(4)
class Memory : public Operand
{
public:
  std::shared_ptr<Reg> reg;
  int offset;
  Memory(std::shared_ptr<Reg> reg, int offset) : reg(reg), offset(offset) {}
  std::string toString() const override
  {
    auto eb = eightByte;
    auto ob = oneByte;
    oneByte = 0;
    eightByte = 1;
    std::string result = std::to_string(-offset) + "(" + reg->toString() + ")";
    eightByte = eb;
    oneByte = ob;
    return result;
  }
  static std::shared_ptr<Memory> createMemory(RegisterType reg, int offset)
  {
    return std::make_shared<Memory>(Reg::createRegister(reg), offset);
  }
};

class Indexed : public Operand
{
public:
  std::shared_ptr<Reg> base;
  std::shared_ptr<Reg> index;
  int scale;

  Indexed(std::shared_ptr<Reg> base, std::shared_ptr<Reg> index, int scale)
      : base(base), index(index), scale(scale) {}
  std::string toString() const override
  {
    auto eb = eightByte;
    auto ob = oneByte;
    oneByte = 0;
    eightByte = 1;
    std::string result = "(" + base->toString() + "," + index->toString() + "," +
                         std::to_string(scale) + ")";
    eightByte = eb;
    oneByte = ob;
    return result;
  }
  static std::shared_ptr<Indexed>
  createIndexed(RegisterType base, RegisterType index, int scale)
  {
    return std::make_shared<Indexed>(Reg::createRegister(base),
                                     Reg::createRegister(index), scale);
  }
};

class PseudoMem : public Operand
{
public:
  std::string name;
  int offset; // offset into the pseudo memory
  PseudoMem(std::string name, int offset) : name(std::move(name)), offset(offset) {}
  // useless
  std::shared_ptr<PseudoMem> static createPseudoMem(const std::string &name, int offset)
  {
    return std::make_shared<PseudoMem>(name, offset);
  }
  std::string toString() const override { return ""; }
};

class Data : public Operand
{
public:
  std::string name;
  Data(std::string name) : name(std::move(name)) {}
  std::string toString() const override { return name + "(%rip)"; }
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
  AssemblyType assemblyType;
  std::string toString() const;

  static ASMInstructionPtr createMov(const OperandPtr &dst,
                                     const OperandPtr &src,
                                     AssemblyType assemblyType)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::MOV;
    instr->dst = dst;
    instr->src1 = src;
    instr->assemblyType = assemblyType;
    return instr;
  }

  static ASMInstructionPtr createUnary(UnaryOpType unaryOpType,
                                       const OperandPtr &src,
                                       AssemblyType assemblyType)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::UNARY;
    instr->unaryOpType = unaryOpType;
    instr->src1 = src;
    instr->assemblyType = assemblyType;
    return instr;
  }

  static ASMInstructionPtr createBinary(BinaryOpType binaryOpType,
                                        const OperandPtr &dst,
                                        const OperandPtr &src2,
                                        AssemblyType assemblyType)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::BINARY;
    instr->binaryOpType = binaryOpType;
    instr->dst = dst;
    instr->src2 = src2;
    instr->assemblyType = assemblyType;
    return instr;
  }

  static ASMInstructionPtr createIDiv(const OperandPtr &src,
                                      AssemblyType assemblyType)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::IDIV;
    instr->src1 = src;
    instr->assemblyType = assemblyType;
    return instr;
  }

  static ASMInstructionPtr createCDQ(AssemblyType assemblyType)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::CDQ;
    instr->assemblyType = assemblyType;
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
    return createBinary(
        BinaryOpType::SUB, Reg::createRegister(RegisterType::SP),
        Immediate::createImmediate(bytes), AssemblyType::QUAD_WORD);
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

  static ASMInstructionPtr createJmpCC(ConditionCode cc,
                                       const std::string &label)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::JMPCC;
    instr->conditionCode = cc;
    instr->label = label;
    return instr;
  }

  static ASMInstructionPtr createSetCC(ConditionCode cc,
                                       const OperandPtr &dst)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::SETCC;
    instr->conditionCode = cc;
    instr->dst = dst;
    return instr;
  }

  static ASMInstructionPtr createCmp(const OperandPtr &src1,
                                     const OperandPtr &src2,
                                     AssemblyType assemblyType)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::CMP;
    instr->src1 = src1;
    instr->src2 = src2;
    instr->assemblyType = assemblyType;
    return instr;
  }

  static ASMInstructionPtr createDeallocateStack(int bytes)
  {
    return createBinary(
        BinaryOpType::ADD, Reg::createRegister(RegisterType::SP),
        Immediate::createImmediate(bytes), AssemblyType::QUAD_WORD);
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

  static ASMInstructionPtr createMovsx(const OperandPtr &dst,
                                       const OperandPtr &src)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::MOVSX;
    instr->dst = dst;
    instr->src1 = src;
    return instr;
  }

  static ASMInstructionPtr createMovZeroExtend(const OperandPtr &dst,
                                               const OperandPtr &src)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::MOVZEROEXTEND;
    instr->dst = dst;
    instr->src1 = src;
    return instr;
  }

  static ASMInstructionPtr createDiv(const OperandPtr &src,
                                     AssemblyType assemblyType)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::DIV;
    instr->src1 = src;
    instr->assemblyType = assemblyType;
    return instr;
  }

  static ASMInstructionPtr createCvttsd2si(const OperandPtr &dst,
                                           const OperandPtr &src,
                                           AssemblyType assemblyType)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::CVTTSD2SI;
    instr->dst = dst;
    instr->src1 = src;
    instr->assemblyType = assemblyType;
    return instr;
  }

  static ASMInstructionPtr createCvttsi2sd(const OperandPtr &dst,
                                           const OperandPtr &src,
                                           AssemblyType assemblyType)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::CVTTSI2SD;
    instr->dst = dst;
    instr->src1 = src;
    instr->assemblyType = assemblyType;
    return instr;
  }

  static ASMInstructionPtr createLea(const OperandPtr &dst,
                                     const OperandPtr &src)
  {
    auto instr = std::make_shared<ASMInstruction>();
    instr->opType = ASMOpType::LEA;
    instr->dst = dst;
    instr->src1 = src;
    return instr;
  }
};
