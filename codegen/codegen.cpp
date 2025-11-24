#include "codegen.hpp"
#include <cassert>
std::unordered_map<std::string, int> offset_table;

std::vector<ASMStaticVariablePtr>
    static_constants; // used for constants declared locally

// -0.0 and 0.0 will be treated differently
std::map<StaticInit, std::string, StaticInitComparator>
    static_init_map; // map to avoid duplicate static inits

int local_label_counter = 0;

/// @return label of the static constant which has been generated previously
std::string generateStaticConstant(StaticInit staticInit, int alignment)
{
  if (static_init_map.find(staticInit) == static_init_map.end())
  {
    std::string label = ".LC" + std::to_string(static_constants.size());
    static_constants.push_back(
        ASMStaticVariable::createStaticConstant(label, alignment, {staticInit}));
    static_init_map[staticInit] = label;
  }
  return static_init_map[staticInit];
}

// default is 4 bytes
bool eightByte = 0;
bool oneByte = 0;

void modifySymbolTable()
{
  for (auto [name, entry] : global_symbol_table)
  {
    global_symbol_table[global_symbol_table[name].name] = entry;
  }
}

AssemblyType getAssemblyType(IRValuePtr irValue)
{
  if (global_symbol_table.find(irValue->name) != global_symbol_table.end())
  {
    return global_symbol_table[irValue->name].assemblyType;
  }
  else
  {
    if (irValue->constType == TypeKind::INT ||
        irValue->constType == TypeKind::UINT)
      return AssemblyType::LONG_WORD;
    else if (irValue->constType == TypeKind::DOUBLE)
    {
      return AssemblyType::DOUBLE_WORD;
    }
    assert(irValue->constType == TypeKind::LONG || irValue->constType == TypeKind::ULONG || irValue->constType == TypeKind::POINTER);
    return AssemblyType::QUAD_WORD; // LONG, ULONG, POINTER
  }
}

bool isUnsigned(IRValuePtr irValue)
{
  if (global_symbol_table.find(irValue->name) != global_symbol_table.end())
  {
    TypeKind kind = global_symbol_table[irValue->name].type.kind;
    return (kind == TypeKind::ULONG || kind == TypeKind::UINT ||
            kind == TypeKind::UCHAR || kind == TypeKind::DOUBLE || kind == TypeKind::POINTER);
  }
  else
  {
    TypeKind kind = irValue->constType;
    return (kind == TypeKind::ULONG || kind == TypeKind::UINT ||
            kind == TypeKind::UCHAR || kind == TypeKind::DOUBLE || kind == TypeKind::POINTER);
  }
  return false;
}

bool isMemoryAddress(const OperandPtr &operand)
{
  return (std::dynamic_pointer_cast<Memory>(operand) != nullptr ||
          std::dynamic_pointer_cast<Data>(operand) != nullptr || std::dynamic_pointer_cast<Indexed>(operand) != nullptr);
}

// Helper function to calculate type size in bytes
int getTypeSize(const Type &type)
{ 
  switch (type.kind)
  {
  case TypeKind::INT:
  case TypeKind::UINT:
    return 4;
  case TypeKind::LONG:
  case TypeKind::ULONG:
  case TypeKind::DOUBLE:
  case TypeKind::POINTER:
    return 8;
  case TypeKind::CHAR:
  case TypeKind::UCHAR:
  case TypeKind::SCHAR:
    return 1;
  case TypeKind::ARRAY:
  {
    const auto &arrayType = std::get<ArrayType>(type.data);
    return getTypeSize(*arrayType.element) * arrayType.size;
  }
  default:
    return 0;
  }
}

std::string ASMProgram::toString() const
{
  std::stringstream ss;
  for (const auto &topLevel : topLevelItems)
  {
    ss << topLevel->toString() << "\n";
  }
  // disable stack execution
  ss << TAB << ".section .note.GNU-stack,\"\",@progbits\n";
  return ss.str();
}

std::string ASMFunction::toString() const
{
  std::stringstream ss;
  if (global)
    ss << TAB << ".globl " << name << "\n";
  ss << TAB << ".text" << "\n";
  ss << name << ":\n";
  ss << TAB << "pushq %rbp\n";
  ss << TAB << "movq %rsp, %rbp\n";
  for (const auto &instruction : instructions)
  {
    ss << TAB << instruction->toString() << "\n";
  }
  return ss.str();
}

ConditionCode binOptoConditionCode(IROpType binOp, bool isUnsigned)
{
  if (isUnsigned)
  {
    switch (binOp)
    {
    case IROpType::GREATER_THAN:
      return ConditionCode::A;
    case IROpType::GREATER_EQUAL:
      return ConditionCode::AE;
    case IROpType::LESS_THAN:
      return ConditionCode::B;
    case IROpType::LESS_EQUAL:
      return ConditionCode::BE;
    case IROpType::EQUAL:
      return ConditionCode::E;
    case IROpType::NOT_EQUAL:
      return ConditionCode::NE;
    default:
      throw std::runtime_error("Invalid binary operation for condition code");
    }
  }
  switch (binOp)
  {
  case IROpType::GREATER_THAN:
    return ConditionCode::G;
  case IROpType::GREATER_EQUAL:
    return ConditionCode::GE;
  case IROpType::LESS_THAN:
    return ConditionCode::L;
  case IROpType::LESS_EQUAL:
    return ConditionCode::LE;
  case IROpType::EQUAL:
    return ConditionCode::E;
  case IROpType::NOT_EQUAL:
    return ConditionCode::NE;
  default:
    throw std::runtime_error("Invalid binary operation for condition code");
  }
}

std::string ASMInstruction::toString() const
{
  std::stringstream ss;
  if (assemblyType == AssemblyType::QUAD_WORD)
  {
    eightByte = 1;
  }
  switch (opType)
  {
  case ASMOpType::MOV:
    if (assemblyType == AssemblyType::QUAD_WORD)
      ss << "movq " << src1->toString() << ", " << dst->toString();
    else if (assemblyType == AssemblyType::DOUBLE_WORD)
      ss << "movsd " << src1->toString() << ", " << dst->toString();
    else
      ss << "movl " << src1->toString() << ", " << dst->toString();
    break;
  case ASMOpType::UNARY:
    switch (unaryOpType)
    {
    case UnaryOpType::NEG:
      if (assemblyType == AssemblyType::QUAD_WORD)
        ss << "negq " << src1->toString();
      else
        ss << "negl " << src1->toString();
      break;
    case UnaryOpType::NOT:
      if (assemblyType == AssemblyType::QUAD_WORD)
        ss << "notq " << src1->toString();
      else
        ss << "notl " << src1->toString();
      break;
    }
    break;
  case ASMOpType::RET:
    ss << "movq %rbp, %rsp\n";
    ss << TAB << "popq %rbp\n";
    ss << TAB << "ret";
    break;
  case ASMOpType::BINARY:
    switch (binaryOpType)
    {
    case BinaryOpType::ADD:
      if (assemblyType == AssemblyType::QUAD_WORD)
        ss << "addq " << src2->toString() << ", " << dst->toString();
      else if (assemblyType == AssemblyType::DOUBLE_WORD)
        ss << "addsd " << src2->toString() << ", " << dst->toString();
      else
        ss << "addl " << src2->toString() << ", " << dst->toString();
      break;
    case BinaryOpType::SUB:
      if (assemblyType == AssemblyType::QUAD_WORD)
        ss << "subq " << src2->toString() << ", " << dst->toString();
      else if (assemblyType == AssemblyType::DOUBLE_WORD)
        ss << "subsd " << src2->toString() << ", " << dst->toString();
      else
        ss << "subl " << src2->toString() << ", " << dst->toString();
      break;
    case BinaryOpType::MULT:
      if (assemblyType == AssemblyType::QUAD_WORD)
        ss << "imulq " << src2->toString() << ", " << dst->toString();
      else if (assemblyType == AssemblyType::DOUBLE_WORD)
        ss << "mulsd " << src2->toString() << ", " << dst->toString();
      else
        ss << "imull " << src2->toString() << ", " << dst->toString();
      break;
    case BinaryOpType::DIVDOUBLE:
      ss << "divsd " << src2->toString() << ", " << dst->toString();
      break;
    case BinaryOpType::AND:
      if (assemblyType == AssemblyType::QUAD_WORD)
        ss << "andq " << src2->toString() << ", " << dst->toString();
      else
        ss << "andl " << src2->toString() << ", " << dst->toString();
      break;
    case BinaryOpType::OR:
      if (assemblyType == AssemblyType::QUAD_WORD)
        ss << "orq " << src2->toString() << ", " << dst->toString();
      else
        ss << "orl " << src2->toString() << ", " << dst->toString();
      break;
    case BinaryOpType::XOR:
      if (assemblyType == AssemblyType::QUAD_WORD)
        ss << "xorq " << src2->toString() << ", " << dst->toString();
      else if (assemblyType == AssemblyType::DOUBLE_WORD)
        ss << "xorpd " << src2->toString() << ", " << dst->toString();
      else
        ss << "xorl " << src2->toString() << ", " << dst->toString();
      break;
    case BinaryOpType::LEFT_SHIFT:
      oneByte = 1;
      if (assemblyType == AssemblyType::QUAD_WORD)
      {
        std::string src2_str = src2->toString();
        oneByte = 0;
        ss << "salq " << src2_str << ", " << dst->toString();
      }
      else
      {
        std::string src2_str = src2->toString();
        oneByte = 0;
        ss << "sall " << src2_str << ", " << dst->toString();
      }
      break;
    case BinaryOpType::RIGHT_SHIFT:
      oneByte = 1;
      if (assemblyType == AssemblyType::QUAD_WORD)
      {
        std::string src2_str = src2->toString();
        oneByte = 0;
        ss << "sarq " << src2_str << ", " << dst->toString();
      }
      else
      {
        std::string src2_str = src2->toString();
        oneByte = 0;
        ss << "sarl " << src2_str << ", " << dst->toString();
      }
      break;
    case BinaryOpType::UNSIGNED_RIGHT_SHIFT:
      oneByte = 1;
      if (assemblyType == AssemblyType::QUAD_WORD)
      {
        std::string src2_str = src2->toString();
        oneByte = 0;
        ss << "shrq " << src2_str << ", " << dst->toString();
      }
      else
      {
        std::string src2_str = src2->toString();
        oneByte = 0;
        ss << "shrl " << src2_str << ", " << dst->toString();
      }
      break;
    }
    break;
  case ASMOpType::IDIV:
    if (assemblyType == AssemblyType::QUAD_WORD)
      ss << "idivq " << src1->toString();
    else
      ss << "idivl " << src1->toString();
    break;
  case ASMOpType::DIV:
    if (assemblyType == AssemblyType::QUAD_WORD)
      ss << "divq " << src1->toString();
    else
      ss << "divl " << src1->toString();
    break;
  case ASMOpType::CDQ:
    if (assemblyType == AssemblyType::QUAD_WORD)
      ss << "cqo";
    else
      ss << "cdq";
    break;
  case ASMOpType::CMP:
    if (assemblyType == AssemblyType::QUAD_WORD)
      ss << "cmpq " << src2->toString() << ", " << src1->toString();
    else if (assemblyType == AssemblyType::DOUBLE_WORD)
      ss << "comisd " << src2->toString() << ", " << src1->toString();
    else
      ss << "cmpl " << src2->toString() << ", " << src1->toString();
    break;
  case ASMOpType::JMP:
    ss << "jmp " << ".L" << label;
    break;
  case ASMOpType::JMPCC:
    switch (conditionCode)
    {
    case ConditionCode::E:
      ss << "je " << ".L" << label;
      break;
    case ConditionCode::NE:
      ss << "jne " << ".L" << label;
      break;
    case ConditionCode::G:
      ss << "jg " << ".L" << label;
      break;
    case ConditionCode::GE:
      ss << "jge " << ".L" << label;
      break;
    case ConditionCode::L:
      ss << "jl " << ".L" << label;
      break;
    case ConditionCode::LE:
      ss << "jle " << ".L" << label;
      break;
    case ConditionCode::A:
      ss << "ja " << ".L" << label;
      break;
    case ConditionCode::AE:
      ss << "jae " << ".L" << label;
      break;
    case ConditionCode::B:
      ss << "jb " << ".L" << label;
      break;
    case ConditionCode::BE:
      ss << "jbe " << ".L" << label;
    }
    break;
  case ASMOpType::SETCC:
    oneByte = 1;
    switch (conditionCode)
    {
    case ConditionCode::E:
      ss << "sete " << dst->toString();
      break;
    case ConditionCode::NE:
      ss << "setne " << dst->toString();
      break;
    case ConditionCode::G:
      ss << "setg " << dst->toString();
      break;
    case ConditionCode::GE:
      ss << "setge " << dst->toString();
      break;
    case ConditionCode::L:
      ss << "setl " << dst->toString();
      break;
    case ConditionCode::LE:
      ss << "setle " << dst->toString();
      break;
    case ConditionCode::A:
      ss << "seta " << dst->toString();
      break;
    case ConditionCode::AE:
      ss << "setae " << dst->toString();
      break;
    case ConditionCode::B:
      ss << "setb " << dst->toString();
      break;
    case ConditionCode::BE:
      ss << "setbe " << dst->toString();
      break;
    }
    oneByte = 0;
    break;
  case ASMOpType::LABEL:
    ss << ".L" << label << ":";
    break;
  case ASMOpType::PUSH:
    eightByte = 1;
    ss << "pushq " << src1->toString();
    eightByte = 0;
    break;
  case ASMOpType::CALL:
    if (global_symbol_table[label].initType == InitType::INITIALIZED)
    {
      ss << "call " << label;
    }
    else
    {
      ss << "call " << label << "@PLT";
    }
    break;
  case ASMOpType::MOVSX:
    ss << "movslq ";
    eightByte = 0;
    ss << src1->toString() << ", ";
    eightByte = 1;
    ss << dst->toString();
    if (assemblyType != AssemblyType::QUAD_WORD)
    {
      eightByte = 0;
    }
    break;
  case ASMOpType::CVTTSD2SI:
    if (assemblyType == AssemblyType::QUAD_WORD)
      ss << "cvttsd2siq " << src1->toString() << ", " << dst->toString();
    else
      ss << "cvttsd2sil " << src1->toString() << ", " << dst->toString();
    break;
  case ASMOpType::CVTTSI2SD:
    if (assemblyType == AssemblyType::QUAD_WORD)
      ss << "cvtsi2sdq " << src1->toString() << ", " << dst->toString();
    else
      ss << "cvtsi2sdl " << src1->toString() << ", " << dst->toString();
    break;
  case ASMOpType::LEA:
    eightByte = 1;
    ss << "leaq " << src1->toString() << ", " << dst->toString();
    eightByte = 0;
    break;
  default:
    break;
  }
  if (assemblyType == AssemblyType::QUAD_WORD)
  {
    eightByte = 0;
  }
  return ss.str();
}

ASMProgramPtr Codegen::generateCode(IRProgramPtr &irProgram)
{
  modifySymbolTable();
  auto asmProgram = IRProgramtoASM(irProgram);
  return asmProgram;
}

ASMProgramPtr Codegen::IRProgramtoASM(const IRProgramPtr &irProgram)
{
  auto asmProgram = std::make_shared<ASMProgram>();
  for (const auto &irTopLevel : irProgram->topLevelItems)
  {
    if (auto irFunction =
            std::dynamic_pointer_cast<IRFunctionNode>(irTopLevel))
    {
      auto asmFunction = IRFunctionToASM(irFunction);
      asmProgram->topLevelItems.push_back(asmFunction);
    }
    else if (auto irStaticVar =
                 std::dynamic_pointer_cast<IRStaticVariableNode>(
                     irTopLevel))
    {
      auto init = irStaticVar->init_list;
      auto typekind = irStaticVar->type.kind;
      int alignment;

      if (typekind == TypeKind::ARRAY)
      {
        // Get element type and size
        ArrayType arr = std::get<ArrayType>(irStaticVar->type.data);
        int elementSize;
        int elementAlignment;
        int numElements = arr.size;
        if (arr.element->kind == TypeKind::UCHAR || arr.element->kind == TypeKind::CHAR)
        {
          elementSize = 1;
          elementAlignment = 1;
        }
        else if (arr.element->kind == TypeKind::INT || arr.element->kind == TypeKind::UINT)
        {
          elementSize = 4;
          elementAlignment = 4;
        }
        else if (arr.element->kind == TypeKind::LONG || arr.element->kind == TypeKind::ULONG ||
                 arr.element->kind == TypeKind::DOUBLE || arr.element->kind == TypeKind::POINTER)
        {
          elementSize = 8;
          elementAlignment = 8;
        }
        else if (arr.element->kind == TypeKind::ARRAY)
        {
          // Nested array - recursively get the scalar element type
          Type* scalarType = arr.element.get();
          while (scalarType->kind == TypeKind::ARRAY)
          {
            numElements *= std::get<ArrayType>(scalarType->data).size;
            scalarType = std::get<ArrayType>(scalarType->data).element.get();
          }
          
          // Determine scalar element size and alignment
          if (scalarType->kind == TypeKind::UCHAR || scalarType->kind == TypeKind::CHAR)
          {
            elementSize = 1;
            elementAlignment = 1;
          }
          else if (scalarType->kind == TypeKind::INT || scalarType->kind == TypeKind::UINT)
          {
            elementSize = 4;
            elementAlignment = 4;
          }
          else if (scalarType->kind == TypeKind::LONG || scalarType->kind == TypeKind::ULONG ||
                   scalarType->kind == TypeKind::DOUBLE || scalarType->kind == TypeKind::POINTER)
          {
            elementSize = 8;
            elementAlignment = 8;
          }
        }
        else {
          assert(false && "Unsupported array element type for static variable");
        }

        int totalSize = elementSize * numElements;

        // Arrays that are 16 bytes or larger use 16-byte alignment
        if (totalSize >= 16)
        {
          alignment = 16;
        }
        else
        {
          alignment = elementAlignment;
        }
      }
      else if (typekind == TypeKind::UCHAR || typekind == TypeKind::CHAR)
      {
        alignment = 1;
      }
      else if (typekind == TypeKind::INT || typekind == TypeKind::UINT)
      {
        alignment = 4;
      }
      else if (typekind == TypeKind::LONG || typekind == TypeKind::ULONG ||
               typekind == TypeKind::DOUBLE || typekind == TypeKind::POINTER)
      {
        alignment = 8;
      }
      auto asmStaticVar = ASMStaticVariable::createStaticVariable(
          irStaticVar->identifier, irStaticVar->global, alignment, init, false);
      asmProgram->topLevelItems.push_back(asmStaticVar);
    }
  }
  asmProgram->topLevelItems.insert(asmProgram->topLevelItems.begin(),
                                   static_constants.begin(),
                                   static_constants.end());
  return asmProgram;
}

ASMFunctionPtr Codegen::IRFunctionToASM(const IRFunctionPtr &irFunction)
{
  auto asmFunction = std::make_shared<ASMFunction>();
  asmFunction->name = irFunction->identifier;
  asmFunction->global = irFunction->global;

  // Set up parameters using classify_parameters
  auto intRegisters = std::vector<RegisterType>{
      RegisterType::DI, RegisterType::SI, RegisterType::DX,
      RegisterType::CX, RegisterType::R8, RegisterType::R9};

  auto doubleRegisters = std::vector<RegisterType>{
      RegisterType::XMM0, RegisterType::XMM1, RegisterType::XMM2,
      RegisterType::XMM3, RegisterType::XMM4, RegisterType::XMM5,
      RegisterType::XMM6, RegisterType::XMM7};

  // Create IRValuePtrs for parameters
  std::vector<IRValuePtr> paramValues;
  for (const auto &paramName : irFunction->parameters)
  {
    auto paramValue = IRValueNode::makeVariable(paramName);
    paramValues.push_back(paramValue);
  }

  // Classify parameters
  ClassifiedParams classified = classifyParameters(paramValues);

  // Copy parameters from general-purpose registers
  size_t regIndex = 0;
  for (const auto &[paramType, param] : classified.intRegArgs)
  {
    auto reg = intRegisters[regIndex];
    asmFunction->instructions.push_back(
        ASMInstruction::createMov(param, Reg::createRegister(reg), paramType));
    regIndex++;
  }

  // Copy parameters from XMM registers
  regIndex = 0;
  for (const auto &param : classified.doubleRegArgs)
  {
    auto reg = doubleRegisters[regIndex];
    asmFunction->instructions.push_back(ASMInstruction::createMov(
        param, Reg::createRegister(reg), AssemblyType::DOUBLE_WORD));
    regIndex++;
  }

  // Copy parameters from the stack
  int offset = -16; // Parameters start at 16(%rbp)
  for (const auto &[paramType, param] : classified.stackArgs)
  {
    asmFunction->instructions.push_back(ASMInstruction::createMov(
        param, Memory::createMemory(RegisterType::BP, offset), paramType));
    offset -= 8;
  }

  for (const auto &irInstruction : irFunction->instructions)
  {
    auto instrs = IRInstructionToASM(irInstruction);
    for (auto instr : instrs)
    {
      asmFunction->instructions.push_back(instr);
    }
  }
  int stackOffset = replacePseudoRegisters(asmFunction);
  finalPass(asmFunction);
  // allocate stack size in multiple of 16
  auto allocateStackInstr =
      ASMInstruction::createAllocateStack(((stackOffset + 15) / 16) * 16);
  asmFunction->instructions.insert(asmFunction->instructions.begin(),
                                   allocateStackInstr);
  asmFunction->stackSize = stackOffset;
  return asmFunction;
}

ClassifiedParams
Codegen::classifyParameters(const std::vector<IRValuePtr> &values)
{
  ClassifiedParams result;

  for (const auto &v : values)
  {
    OperandPtr operand = IRValueToOperand(v);
    AssemblyType assemblyType = getAssemblyType(v);

    auto typedOperand = std::make_pair(assemblyType, operand);

    if (assemblyType == AssemblyType::DOUBLE_WORD)
    {
      // Check if we can pass in XMM register (max 8)
      if (result.doubleRegArgs.size() < 8)
      {
        result.doubleRegArgs.push_back(operand);
      }
      else
      {
        result.stackArgs.push_back(typedOperand);
      }
    }
    else
    {
      // Check if we can pass in general-purpose register (max 6)
      if (result.intRegArgs.size() < 6)
      {
        result.intRegArgs.push_back(typedOperand);
      }
      else
      {
        result.stackArgs.push_back(typedOperand);
      }
    }
  }

  return result;
}

std::vector<ASMInstructionPtr>
Codegen::IRInstructionToASM(const IRInstructionPtr &irInstruction)
{
  std::vector<ASMInstructionPtr> asmInstructions;
  switch (irInstruction->opType)
  {
  case IROpType::NEGATE:
  {
    // First, move src to dst
    AssemblyType type = getAssemblyType(irInstruction->dst);
    if (type == AssemblyType::DOUBLE_WORD)
    {
      std::string zero =
          generateStaticConstant(StaticInit::makeDoubleInit(-0.0), 16);
      asmInstructions.push_back(ASMInstruction::createMov(
          IRValueToOperand(irInstruction->dst),
          IRValueToOperand(irInstruction->src1), AssemblyType::DOUBLE_WORD));
      asmInstructions.push_back(ASMInstruction::createBinary(
          BinaryOpType::XOR, IRValueToOperand(irInstruction->dst),
          std::make_shared<Data>(zero), AssemblyType::DOUBLE_WORD));
      break;
    }
    asmInstructions.push_back(
        ASMInstruction::createMov(IRValueToOperand(irInstruction->dst),
                                  IRValueToOperand(irInstruction->src1), type));

    // Then, negate the dst
    asmInstructions.push_back(ASMInstruction::createUnary(
        UnaryOpType::NEG, IRValueToOperand(irInstruction->dst), type));
    break;
  }
  case IROpType::COMPLEMENT:
  {
    // First, move src to dst
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(
        ASMInstruction::createMov(IRValueToOperand(irInstruction->dst),
                                  IRValueToOperand(irInstruction->src1), type));

    // Then, complement the dst
    asmInstructions.push_back(ASMInstruction::createUnary(
        UnaryOpType::NOT, IRValueToOperand(irInstruction->dst), type));
    break;
  }
  case IROpType::RETURN:
  {
    AssemblyType type = getAssemblyType(irInstruction->src1);
    if (type == AssemblyType::DOUBLE_WORD)
    {
      asmInstructions.push_back(ASMInstruction::createMov(
          Reg::createRegister(RegisterType::XMM0),
          IRValueToOperand(irInstruction->src1), type));
      asmInstructions.push_back(ASMInstruction::createRet());
    }
    else
    {
      asmInstructions.push_back(ASMInstruction::createMov(
          Reg::createRegister(RegisterType::AX),
          IRValueToOperand(irInstruction->src1), type));
      asmInstructions.push_back(ASMInstruction::createRet());
    }
    break;
  }
  case IROpType::ADD:
  {
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(
        ASMInstruction::createMov(IRValueToOperand(irInstruction->dst),
                                  IRValueToOperand(irInstruction->src1), type));
    asmInstructions.push_back(ASMInstruction::createBinary(
        BinaryOpType::ADD, IRValueToOperand(irInstruction->dst),
        IRValueToOperand(irInstruction->src2), type));
    break;
  }
  case IROpType::SUBTRACT:
  {
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(
        ASMInstruction::createMov(IRValueToOperand(irInstruction->dst),
                                  IRValueToOperand(irInstruction->src1), type));
    asmInstructions.push_back(ASMInstruction::createBinary(
        BinaryOpType::SUB, IRValueToOperand(irInstruction->dst),
        IRValueToOperand(irInstruction->src2), type));
    break;
  }
  case IROpType::MULTIPLY:
  {
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(
        ASMInstruction::createMov(IRValueToOperand(irInstruction->dst),
                                  IRValueToOperand(irInstruction->src1), type));
    asmInstructions.push_back(ASMInstruction::createBinary(
        BinaryOpType::MULT, IRValueToOperand(irInstruction->dst),
        IRValueToOperand(irInstruction->src2), type));
    break;
  }
  /*
    division using idiv instruction
    dividend is in rax
    divisor is the source operand
    quotient is stored in rax
    remainder is stored in rdx
  */
  case IROpType::DIVIDE:
  {
    bool isUnsigned = global_symbol_table[irInstruction->dst->name].type.kind ==
                          TypeKind::ULONG ||
                      global_symbol_table[irInstruction->dst->name].type.kind ==
                          TypeKind::UINT;

    bool isDouble = global_symbol_table[irInstruction->dst->name].type.kind ==
                    TypeKind::DOUBLE;
    if (isDouble)
    {
      // for double division, use SSE instructions
      asmInstructions.push_back(ASMInstruction::createMov(
          IRValueToOperand(irInstruction->dst),
          IRValueToOperand(irInstruction->src1), AssemblyType::DOUBLE_WORD));
      asmInstructions.push_back(ASMInstruction::createBinary(
          BinaryOpType::DIVDOUBLE, IRValueToOperand(irInstruction->dst),
          IRValueToOperand(irInstruction->src2), AssemblyType::DOUBLE_WORD));
    }
    else if (isUnsigned)
    {
      AssemblyType type = getAssemblyType(irInstruction->dst);
      asmInstructions.push_back(ASMInstruction::createMov(
          Reg::createRegister(RegisterType::AX),
          IRValueToOperand(irInstruction->src1), type));
      asmInstructions.push_back(
          ASMInstruction::createMov(Reg::createRegister(RegisterType::DX),
                                    Immediate::createImmediate(0), type));

      asmInstructions.push_back(ASMInstruction::createDiv(
          IRValueToOperand(irInstruction->src2), type));
      asmInstructions.push_back(ASMInstruction::createMov(
          IRValueToOperand(irInstruction->dst),
          Reg::createRegister(RegisterType::AX), type));
    }
    else
    {
      AssemblyType type = getAssemblyType(irInstruction->dst);
      asmInstructions.push_back(ASMInstruction::createMov(
          Reg::createRegister(RegisterType::AX),
          IRValueToOperand(irInstruction->src1), type));
      asmInstructions.push_back(ASMInstruction::createCDQ(type));

      asmInstructions.push_back(ASMInstruction::createIDiv(
          IRValueToOperand(irInstruction->src2), type));
      asmInstructions.push_back(ASMInstruction::createMov(
          IRValueToOperand(irInstruction->dst),
          Reg::createRegister(RegisterType::AX), type));
    }
    break;
  }
  case IROpType::REMAINDER:
  {
    bool isUnsigned = global_symbol_table[irInstruction->dst->name].type.kind ==
                          TypeKind::ULONG ||
                      global_symbol_table[irInstruction->dst->name].type.kind ==
                          TypeKind::UINT;
    if (isUnsigned)
    {
      AssemblyType type = getAssemblyType(irInstruction->dst);
      asmInstructions.push_back(ASMInstruction::createMov(
          Reg::createRegister(RegisterType::AX),
          IRValueToOperand(irInstruction->src1), type));
      asmInstructions.push_back(
          ASMInstruction::createMov(Reg::createRegister(RegisterType::DX),
                                    Immediate::createImmediate(0), type));
      asmInstructions.push_back(ASMInstruction::createDiv(
          IRValueToOperand(irInstruction->src2), type));
      asmInstructions.push_back(ASMInstruction::createMov(
          IRValueToOperand(irInstruction->dst),
          Reg::createRegister(RegisterType::DX), type));
    }
    else
    {
      AssemblyType type = getAssemblyType(irInstruction->dst);
      asmInstructions.push_back(ASMInstruction::createMov(
          Reg::createRegister(RegisterType::AX),
          IRValueToOperand(irInstruction->src1), type));
      asmInstructions.push_back(ASMInstruction::createCDQ(type));

      asmInstructions.push_back(ASMInstruction::createIDiv(
          IRValueToOperand(irInstruction->src2), type));
      asmInstructions.push_back(ASMInstruction::createMov(
          IRValueToOperand(irInstruction->dst),
          Reg::createRegister(RegisterType::DX), type));
    }
    break;
  }
  case IROpType::AND:
  {
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(
        ASMInstruction::createMov(IRValueToOperand(irInstruction->dst),
                                  IRValueToOperand(irInstruction->src1), type));
    asmInstructions.push_back(ASMInstruction::createBinary(
        BinaryOpType::AND, IRValueToOperand(irInstruction->dst),
        IRValueToOperand(irInstruction->src2), type));
    break;
  }
  case IROpType::OR:
  {
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(
        ASMInstruction::createMov(IRValueToOperand(irInstruction->dst),
                                  IRValueToOperand(irInstruction->src1), type));
    asmInstructions.push_back(ASMInstruction::createBinary(
        BinaryOpType::OR, IRValueToOperand(irInstruction->dst),
        IRValueToOperand(irInstruction->src2), type));
    break;
  }
  case IROpType::XOR:
  {
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(
        ASMInstruction::createMov(IRValueToOperand(irInstruction->dst),
                                  IRValueToOperand(irInstruction->src1), type));
    asmInstructions.push_back(ASMInstruction::createBinary(
        BinaryOpType::XOR, IRValueToOperand(irInstruction->dst),
        IRValueToOperand(irInstruction->src2), type));
    break;
  }
  case IROpType::LEFT_SHIFT:
  {
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(
        ASMInstruction::createMov(IRValueToOperand(irInstruction->dst),
                                  IRValueToOperand(irInstruction->src1), type));
    asmInstructions.push_back(ASMInstruction::createBinary(
        BinaryOpType::LEFT_SHIFT, IRValueToOperand(irInstruction->dst),
        IRValueToOperand(irInstruction->src2), type));
    break;
  }
  case IROpType::RIGHT_SHIFT:
  {
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(
        ASMInstruction::createMov(IRValueToOperand(irInstruction->dst),
                                  IRValueToOperand(irInstruction->src1), type));
    asmInstructions.push_back(ASMInstruction::createBinary(
        BinaryOpType::RIGHT_SHIFT, IRValueToOperand(irInstruction->dst),
        IRValueToOperand(irInstruction->src2), type));
    break;
  }
  case IROpType::EQUAL:
  case IROpType::NOT_EQUAL:
  case IROpType::GREATER_THAN:
  case IROpType::GREATER_EQUAL:
  case IROpType::LESS_THAN:
  case IROpType::LESS_EQUAL:
  {
    asmInstructions.push_back(
        ASMInstruction::createCmp(IRValueToOperand(irInstruction->src1),
                                  IRValueToOperand(irInstruction->src2),
                                  getAssemblyType(irInstruction->src1)));
    asmInstructions.push_back(ASMInstruction::createMov(
        IRValueToOperand(irInstruction->dst), Immediate::createImmediate(0),
        AssemblyType::LONG_WORD));
    asmInstructions.push_back(ASMInstruction::createSetCC(
        binOptoConditionCode(irInstruction->opType,
                             isUnsigned(irInstruction->src1)),
        IRValueToOperand(irInstruction->dst)));
    break;
  }
  case IROpType::LABEL:
  {
    asmInstructions.push_back(
        ASMInstruction::createLabel(irInstruction->label));
    break;
  }
  case IROpType::JUMP:
  {
    asmInstructions.push_back(ASMInstruction::createJmp(irInstruction->label));
    break;
  }
  case IROpType::JUMP_IF_ZERO:
  {
    AssemblyType type = getAssemblyType(irInstruction->src1);
    if (type == AssemblyType::DOUBLE_WORD)
    {
      // for double zero comparison
      asmInstructions.push_back(ASMInstruction::createBinary(
          BinaryOpType::XOR, Reg::createRegister(RegisterType::XMM0),
          Reg::createRegister(RegisterType::XMM0), AssemblyType::DOUBLE_WORD));
      asmInstructions.push_back(ASMInstruction::createCmp(
          IRValueToOperand(irInstruction->src1),
          Reg::createRegister(RegisterType::XMM0), type));
      asmInstructions.push_back(
          ASMInstruction::createJmpCC(ConditionCode::E, irInstruction->label));
      break;
    }
    asmInstructions.push_back(
        ASMInstruction::createCmp(IRValueToOperand(irInstruction->src1),
                                  Immediate::createImmediate(0), type));
    asmInstructions.push_back(
        ASMInstruction::createJmpCC(ConditionCode::E, irInstruction->label));
    break;
  }
  case IROpType::JUMP_IF_NOT_ZERO:
  {
    AssemblyType type = getAssemblyType(irInstruction->src1);
    if (type == AssemblyType::DOUBLE_WORD)
    {
      // for double not zero comparison
      asmInstructions.push_back(ASMInstruction::createBinary(
          BinaryOpType::XOR, Reg::createRegister(RegisterType::XMM0),
          Reg::createRegister(RegisterType::XMM0), AssemblyType::DOUBLE_WORD));
      asmInstructions.push_back(ASMInstruction::createCmp(
          IRValueToOperand(irInstruction->src1),
          Reg::createRegister(RegisterType::XMM0), type));
      asmInstructions.push_back(
          ASMInstruction::createJmpCC(ConditionCode::NE, irInstruction->label));
      break;
    }
    asmInstructions.push_back(
        ASMInstruction::createCmp(IRValueToOperand(irInstruction->src1),
                                  Immediate::createImmediate(0), type));
    asmInstructions.push_back(
        ASMInstruction::createJmpCC(ConditionCode::NE, irInstruction->label));
    break;
  }
  case IROpType::NOT:
  {
    AssemblyType type = getAssemblyType(irInstruction->src1);
    if (type == AssemblyType::DOUBLE_WORD)
    {
      // for double not zero comparison
      asmInstructions.push_back(ASMInstruction::createBinary(
          BinaryOpType::XOR, Reg::createRegister(RegisterType::XMM0),
          Reg::createRegister(RegisterType::XMM0), AssemblyType::DOUBLE_WORD));
      asmInstructions.push_back(ASMInstruction::createCmp(
          IRValueToOperand(irInstruction->src1),
          Reg::createRegister(RegisterType::XMM0), type));
      asmInstructions.push_back(ASMInstruction::createMov(
          IRValueToOperand(irInstruction->dst), Immediate::createImmediate(0),
          AssemblyType::LONG_WORD));
      asmInstructions.push_back(ASMInstruction::createSetCC(
          ConditionCode::E, IRValueToOperand(irInstruction->dst)));
      break;
    }
    asmInstructions.push_back(
        ASMInstruction::createCmp(IRValueToOperand(irInstruction->src1),
                                  Immediate::createImmediate(0), type));
    asmInstructions.push_back(ASMInstruction::createMov(
        IRValueToOperand(irInstruction->dst), Immediate::createImmediate(0),
        AssemblyType::LONG_WORD));
    asmInstructions.push_back(ASMInstruction::createSetCC(
        ConditionCode::E, IRValueToOperand(irInstruction->dst)));
    break;
  }
  case IROpType::COPY:
  {
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(
        ASMInstruction::createMov(IRValueToOperand(irInstruction->dst),
                                  IRValueToOperand(irInstruction->src1), type));
    break;
  }
  case IROpType::CALL:
  {
    auto intRegisters = std::vector<RegisterType>{
        RegisterType::DI, RegisterType::SI, RegisterType::DX,
        RegisterType::CX, RegisterType::R8, RegisterType::R9};

    auto doubleRegisters = std::vector<RegisterType>{
        RegisterType::XMM0, RegisterType::XMM1, RegisterType::XMM2,
        RegisterType::XMM3, RegisterType::XMM4, RegisterType::XMM5,
        RegisterType::XMM6, RegisterType::XMM7};

    // Classify arguments
    ClassifiedParams classified = classifyParameters(irInstruction->src2->args);

    // Calculate stack padding for 16-byte alignment
    int stackPadding = 0;
    if (classified.stackArgs.size() % 2 != 0)
    {
      stackPadding = 8;
    }

    // Allocate stack padding if needed
    if (stackPadding != 0)
    {
      auto allocInstr = ASMInstruction::createBinary(
          BinaryOpType::SUB, Reg::createRegister(RegisterType::SP),
          Immediate::createImmediate(stackPadding), AssemblyType::QUAD_WORD);
      asmInstructions.push_back(allocInstr);
    }

    // Pass arguments in general-purpose registers
    for (size_t i = 0; i < classified.intRegArgs.size(); ++i)
    {
      auto [assemblyType, assemblyArg] = classified.intRegArgs[i];
      auto movInstr = ASMInstruction::createMov(
          Reg::createRegister(intRegisters[i]), assemblyArg, assemblyType);
      asmInstructions.push_back(movInstr);
    }

    // Pass arguments in XMM registers
    for (size_t i = 0; i < classified.doubleRegArgs.size(); ++i)
    {
      auto assemblyArg = classified.doubleRegArgs[i];
      auto movInstr =
          ASMInstruction::createMov(Reg::createRegister(doubleRegisters[i]),
                                    assemblyArg, AssemblyType::DOUBLE_WORD);
      asmInstructions.push_back(movInstr);
    }

    // Pass arguments on stack in reverse order
    for (auto it = classified.stackArgs.rbegin();
         it != classified.stackArgs.rend(); ++it)
    {
      auto [assemblyType, assemblyArg] = *it;

      // Check if operand is a register or immediate, or if it's quadword/double
      bool isRegOrImm =
          (std::dynamic_pointer_cast<Reg>(assemblyArg) != nullptr) ||
          (std::dynamic_pointer_cast<Immediate>(assemblyArg) != nullptr);
      bool isQuadOrDouble = (assemblyType == AssemblyType::QUAD_WORD) ||
                            (assemblyType == AssemblyType::DOUBLE_WORD);

      if (isRegOrImm || isQuadOrDouble)
      {
        // Can push directly
        auto pushInstr = ASMInstruction::createPush(assemblyArg);
        asmInstructions.push_back(pushInstr);
      }
      else
      {
        // Need to move to AX first, then push
        auto movInstr = ASMInstruction::createMov(
            Reg::createRegister(RegisterType::AX), assemblyArg, assemblyType);
        asmInstructions.push_back(movInstr);

        auto pushInstr =
            ASMInstruction::createPush(Reg::createRegister(RegisterType::AX));
        asmInstructions.push_back(pushInstr);
      }
    }

    // Emit call instruction
    auto callInstr = ASMInstruction::createCall(irInstruction->src1->name);
    asmInstructions.push_back(callInstr);

    // Deallocate stack (remove args + padding)
    int bytesToRemove = 8 * classified.stackArgs.size() + stackPadding;
    if (bytesToRemove != 0)
    {
      auto deallocInstr = ASMInstruction::createBinary(
          BinaryOpType::ADD, Reg::createRegister(RegisterType::SP),
          Immediate::createImmediate(bytesToRemove), AssemblyType::QUAD_WORD);
      asmInstructions.push_back(deallocInstr);
    }

    // Retrieve return value (if dst is not null)
    if (irInstruction->dst)
    {
      AssemblyType returnType = getAssemblyType(irInstruction->dst);
      if (returnType == AssemblyType::DOUBLE_WORD)
      {
        auto movInstr = ASMInstruction::createMov(
            IRValueToOperand(irInstruction->dst),
            Reg::createRegister(RegisterType::XMM0), AssemblyType::DOUBLE_WORD);
        asmInstructions.push_back(movInstr);
      }
      else
      {
        auto movInstr = ASMInstruction::createMov(
            IRValueToOperand(irInstruction->dst),
            Reg::createRegister(RegisterType::AX), returnType);
        asmInstructions.push_back(movInstr);
      }
    }

    break;
  }
  case IROpType::TRUNCATE:
  {
    asmInstructions.push_back(ASMInstruction::createMov(
        IRValueToOperand(irInstruction->dst),
        IRValueToOperand(irInstruction->src1), AssemblyType::LONG_WORD));
    break;
  }
  case IROpType::SIGN_EXTEND:
  {
    asmInstructions.push_back(
        ASMInstruction::createMovsx(IRValueToOperand(irInstruction->dst),
                                    IRValueToOperand(irInstruction->src1)));
    break;
  }
  case IROpType::ZERO_EXTEND:
  {
    asmInstructions.push_back(ASMInstruction::createMovZeroExtend(
        IRValueToOperand(irInstruction->dst),
        IRValueToOperand(irInstruction->src1)));
    break;
  }
  case IROpType::DOUBLE_TO_LONG:
  {
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(ASMInstruction::createCvttsd2si(
        IRValueToOperand(irInstruction->dst),
        IRValueToOperand(irInstruction->src1), type));
    break;
  }
  case IROpType::DOUBLE_TO_ULONG:
  {
    AssemblyType type = getAssemblyType(irInstruction->dst);
    if (type == AssemblyType::LONG_WORD)
    {
      asmInstructions.push_back(ASMInstruction::createCvttsd2si(
          Reg::createRegister(RegisterType::AX),
          IRValueToOperand(irInstruction->src1), AssemblyType::QUAD_WORD));
      asmInstructions.push_back(ASMInstruction::createMov(
          IRValueToOperand(irInstruction->dst),
          Reg::createRegister(RegisterType::AX), AssemblyType::LONG_WORD));
    }
    /*
    double to ulong:
    check if double is less than max long
    if not subtract LONG_MAX + 1 from double
    convert to long
    add LONG_MAX + 1 to long result to get ulong
    */
    else
    {
      std::string out_of_range_label =
          ".L_out_of_range_" + std::to_string(local_label_counter++);
      std::string end_label = ".L_end_" + std::to_string(local_label_counter++);
      std::string upper_bound = generateStaticConstant(
          StaticInit::makeDoubleInit(
              static_cast<double>(static_cast<unsigned long>(
                  std::numeric_limits<int64_t>::max() + 1ul))),
          8);
      asmInstructions.push_back(ASMInstruction::createCmp(
          std::make_shared<Data>(upper_bound),
          IRValueToOperand(irInstruction->src1), AssemblyType::DOUBLE_WORD));
      asmInstructions.push_back(
          ASMInstruction::createJmpCC(ConditionCode::AE, out_of_range_label));
      asmInstructions.push_back(ASMInstruction::createCvttsd2si(
          IRValueToOperand(irInstruction->dst),
          IRValueToOperand(irInstruction->src1), AssemblyType::QUAD_WORD));
      asmInstructions.push_back(ASMInstruction::createJmp(end_label));
      asmInstructions.push_back(
          ASMInstruction::createLabel(out_of_range_label));
      asmInstructions.push_back(ASMInstruction::createMov(
          Reg::createRegister(RegisterType::XMM1),
          IRValueToOperand(irInstruction->src1), AssemblyType::DOUBLE_WORD));
      asmInstructions.push_back(ASMInstruction::createBinary(
          BinaryOpType::SUB, Reg::createRegister(RegisterType::XMM1),
          std::make_shared<Data>(upper_bound), AssemblyType::DOUBLE_WORD));
      asmInstructions.push_back(ASMInstruction::createCvttsd2si(
          IRValueToOperand(irInstruction->dst),
          Reg::createRegister(RegisterType::XMM1), AssemblyType::QUAD_WORD));
      asmInstructions.push_back(ASMInstruction::createBinary(
          BinaryOpType::ADD, IRValueToOperand(irInstruction->dst),
          Immediate::createImmediate(
              static_cast<unsigned long>(std::numeric_limits<int64_t>::max()) +
              1ul),
          AssemblyType::QUAD_WORD));
      asmInstructions.push_back(ASMInstruction::createLabel(end_label));
    }
    break;
  }
  case IROpType::LONG_TO_DOUBLE:
  {
    AssemblyType type = getAssemblyType(irInstruction->src1);
    asmInstructions.push_back(ASMInstruction::createCvttsi2sd(
        IRValueToOperand(irInstruction->dst),
        IRValueToOperand(irInstruction->src1), type));
    break;
  }
  case IROpType::ULONG_TO_DOUBLE:
  {
    AssemblyType type = getAssemblyType(irInstruction->src1);
    auto src = IRValueToOperand(irInstruction->src1);
    auto dst = IRValueToOperand(irInstruction->dst);

    if (type == AssemblyType::LONG_WORD)
    {
      // 32-bit unsigned: zero-extend to 64-bit, then convert
      asmInstructions.push_back(ASMInstruction::createMov(
          Reg::createRegister(RegisterType::AX), src, AssemblyType::LONG_WORD));
      asmInstructions.push_back(ASMInstruction::createCvttsi2sd(
          dst, Reg::createRegister(RegisterType::AX), AssemblyType::QUAD_WORD));
    }
    else
    {
      // 64-bit unsigned: check if value fits in signed range
      // Algorithm:
      // 1. Compare with 0 (check sign bit)
      // 2. If non-negative (< 2^63), use signed conversion directly
      // 3. If negative (>= 2^63), divide by 2, convert, then multiply by 2

      std::string label_out_of_range =
          ".L_ulong_out_of_range_" + std::to_string(local_label_counter++);
      std::string label_end =
          ".L_ulong_end_" + std::to_string(local_label_counter++);

      // cmpq $0, src
      asmInstructions.push_back(ASMInstruction::createCmp(
          src, std::make_shared<Immediate>(0), AssemblyType::QUAD_WORD));

      // jl .L_out_of_range (jump if negative, i.e., >= 2^63)
      asmInstructions.push_back(
          ASMInstruction::createJmpCC(ConditionCode::L, label_out_of_range));

      // Path 1: Value fits in signed range
      // cvtsi2sdq src, dst
      asmInstructions.push_back(
          ASMInstruction::createCvttsi2sd(dst, src, AssemblyType::QUAD_WORD));

      // jmp .L_end
      asmInstructions.push_back(ASMInstruction::createJmp(label_end));

      // Path 2: Value >= 2^63
      // .L_out_of_range:
      asmInstructions.push_back(
          ASMInstruction::createLabel(label_out_of_range));

      // movq src, %rax
      asmInstructions.push_back(ASMInstruction::createMov(
          Reg::createRegister(RegisterType::AX), src, AssemblyType::QUAD_WORD));

      // movq %rax, %rdx
      asmInstructions.push_back(ASMInstruction::createMov(
          Reg::createRegister(RegisterType::DX),
          Reg::createRegister(RegisterType::AX), AssemblyType::QUAD_WORD));

      // shrq $1, %rdx (unsigned divide by 2)
      asmInstructions.push_back(ASMInstruction::createBinary(
          BinaryOpType::UNSIGNED_RIGHT_SHIFT,
          Reg::createRegister(RegisterType::DX), std::make_shared<Immediate>(1),
          AssemblyType::QUAD_WORD));

      // andq $1, %rax (get last bit)
      asmInstructions.push_back(ASMInstruction::createBinary(
          BinaryOpType::AND, Reg::createRegister(RegisterType::AX),
          std::make_shared<Immediate>(1), AssemblyType::QUAD_WORD));

      // orq %rax, %rdx (add back the last bit to maintain precision)
      asmInstructions.push_back(ASMInstruction::createBinary(
          BinaryOpType::OR, Reg::createRegister(RegisterType::DX),
          Reg::createRegister(RegisterType::AX), AssemblyType::QUAD_WORD));

      // cvtsi2sdq %rdx, dst
      asmInstructions.push_back(ASMInstruction::createCvttsi2sd(
          dst, Reg::createRegister(RegisterType::DX), AssemblyType::QUAD_WORD));

      // addsd dst, dst (multiply by 2)
      asmInstructions.push_back(ASMInstruction::createBinary(
          BinaryOpType::ADD, dst, dst, AssemblyType::DOUBLE_WORD));

      // .L_end:
      asmInstructions.push_back(ASMInstruction::createLabel(label_end));
    }
    break;
  }
  case IROpType::LOAD:
  {
    asmInstructions.push_back(ASMInstruction::createMov(
        Reg::createRegister(RegisterType::AX),
        IRValueToOperand(irInstruction->src1),
        AssemblyType::QUAD_WORD));
    asmInstructions.push_back(ASMInstruction::createMov(
        IRValueToOperand(irInstruction->dst),
        Memory::createMemory(RegisterType::AX, 0),
        getAssemblyType(irInstruction->dst)));
    break;
  }
  case IROpType::STORE:
  {
    asmInstructions.push_back(ASMInstruction::createMov(
        Reg::createRegister(RegisterType::AX),
        IRValueToOperand(irInstruction->dst),
        AssemblyType::QUAD_WORD));
    asmInstructions.push_back(ASMInstruction::createMov(
        Memory::createMemory(RegisterType::AX, 0),
        IRValueToOperand(irInstruction->src1),
        getAssemblyType(irInstruction->src1)));
    break;
  }
  case IROpType::GET_ADDRESS:
  {
    asmInstructions.push_back(ASMInstruction::createLea(
        IRValueToOperand(irInstruction->dst),
        IRValueToOperand(irInstruction->src1)));
    break;
  }
  case IROpType::COPY_TO_OFFSET:
  {
    asmInstructions.push_back(ASMInstruction::createMov(PseudoMem::createPseudoMem(irInstruction->label, irInstruction->offset),
                                                        IRValueToOperand(irInstruction->src1),
                                                        getAssemblyType(irInstruction->src1)));
    break;
  }
  case IROpType::ADD_PTR:
  {
    // handling separately for constant index, variable index and scale of 1,2,4,8 and variable index and other scale
    // constant index
    if (irInstruction->src2->type == IRValueType::CONSTANT)
    {
      int64_t index = std::get<int64_t>(irInstruction->src2->value);
      asmInstructions.push_back(ASMInstruction::createMov(Reg::createRegister(RegisterType::AX), IRValueToOperand(irInstruction->src1), AssemblyType::QUAD_WORD));
      asmInstructions.push_back(ASMInstruction::createLea(IRValueToOperand(irInstruction->dst), Memory::createMemory(RegisterType::AX, index * irInstruction->scale)));
      break;
    }
    if ((irInstruction->scale == 1) || (irInstruction->scale == 2) || (irInstruction->scale == 4) || (irInstruction->scale == 8))
    {
      asmInstructions.push_back(ASMInstruction::createMov(
          Reg::createRegister(RegisterType::AX),
          IRValueToOperand(irInstruction->src1),
          AssemblyType::QUAD_WORD));
      asmInstructions.push_back(ASMInstruction::createMov(
          Reg::createRegister(RegisterType::DX),
          IRValueToOperand(irInstruction->src2),
          AssemblyType::QUAD_WORD));
      asmInstructions.push_back(ASMInstruction::createLea(
          IRValueToOperand(irInstruction->dst),
          Indexed::createIndexed(
              RegisterType::AX, RegisterType::DX, irInstruction->scale)));
      break;
    }
    // variable index and other scale
    asmInstructions.push_back(ASMInstruction::createMov(
        Reg::createRegister(RegisterType::AX),
        IRValueToOperand(irInstruction->src1),
        AssemblyType::QUAD_WORD));
    asmInstructions.push_back(ASMInstruction::createMov(
        Reg::createRegister(RegisterType::DX),
        IRValueToOperand(irInstruction->src2),
        AssemblyType::QUAD_WORD));

    asmInstructions.push_back(ASMInstruction::createBinary(
        BinaryOpType::MULT,
        Reg::createRegister(RegisterType::DX),
        Immediate::createImmediate(irInstruction->scale),
        AssemblyType::QUAD_WORD));
    asmInstructions.push_back(ASMInstruction::createLea(
        IRValueToOperand(irInstruction->dst),
        Indexed::createIndexed(
            RegisterType::AX, RegisterType::DX, 1)));
    break;
  }
  default:
    break;
  }
  return asmInstructions;
}

OperandPtr Codegen::IRValueToOperand(const IRValuePtr &irValue)
{
  if (!irValue)
    return nullptr;
  switch (irValue->type)
  {
  case IRValueType::CONSTANT:
  {
    auto imm = std::make_shared<Immediate>(0);
    std::string label = "";
    std::visit(
        [&imm, &label](auto &&arg)
        {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, int>)
          {
            // Handle int constant
            imm->value = static_cast<int64_t>(arg);
          }
          else if constexpr (std::is_same_v<T, long>)
          {
            // Handle long constant
            imm->value = static_cast<int64_t>(arg);
          }
          else if constexpr (std::is_same_v<T, unsigned int>)
          {
            // Handle unsigned int constant
            imm->value = static_cast<int64_t>(arg);
          }
          else if constexpr (std::is_same_v<T, unsigned long>)
          {
            // Handle unsigned long constant
            imm->value = static_cast<int64_t>(arg);
          }
          else if constexpr (std::is_same_v<T, double>)
          {
            // Handle double constant
            // Store the double in the static constants and use its label as an
            // operand
            label = generateStaticConstant(StaticInit::makeDoubleInit(arg), 8);
          }
          else
          {
            throw std::runtime_error(
                "Unsupported constant type in IRValueToOperand");
          }
        },
        irValue->value);
    if (std::holds_alternative<double>(irValue->value))
    {
      // For double constants, return a Data operand pointing to the static
      // constant
      return std::make_shared<Data>(label);
    }
    return imm;
  }
  case IRValueType::VARIABLE:
  {
    if (global_symbol_table.find(irValue->name) != global_symbol_table.end() && global_symbol_table[irValue->name].assemblyType == AssemblyType::BYTE_ARRAY)
    {
      return std::make_shared<PseudoMem>(irValue->name, 0);
    }
    return std::make_shared<Pseudo>(irValue->name);
  }
  case IRValueType::TEMPORARY:
  {
    return std::make_shared<Pseudo>(irValue->name);
  }
  case IRValueType::ARGS:
  {
  }
  default:
    return nullptr;
  }
}

int Codegen::replacePseudoRegisters(ASMBasePtr ast)
{
  static int offset = 0;
  if (auto program = std::dynamic_pointer_cast<ASMProgram>(ast))
  {
    for (auto &topLevel : program->topLevelItems)
    {
      replacePseudoRegisters(topLevel);
    }
  }
  else if (auto function = std::dynamic_pointer_cast<ASMFunction>(ast))
  {
    offset_table.clear();
    offset = 0;
    for (auto &instruction : function->instructions)
    {
      //   ---- PseudoMem Replacement ---
      // Replace dst
      if (instruction->dst)
      {
        if (auto pseudoMem = std::dynamic_pointer_cast<PseudoMem>(instruction->dst))
        {
          // Check if this is a static array
          if (global_symbol_table.find(pseudoMem->name) != global_symbol_table.end() &&
              global_symbol_table[pseudoMem->name].storageClass == StorageClass::STATIC)
          {
            // Static array - convert to Data operand (offset must be 0)
            if (pseudoMem->offset != 0)
            {
              std::cerr << "Error: PseudoMem with nonzero offset for static array "
                        << pseudoMem->name << std::endl;
              exit(1);
            }
            instruction->dst = std::make_shared<Data>(pseudoMem->name);
          }
          else
          {
            // Automatic storage duration array
            // Allocate the array if not already allocated
            if (offset_table.find(pseudoMem->name) == offset_table.end())
            {
              auto &entry = global_symbol_table[pseudoMem->name];
              if (entry.assemblyType == AssemblyType::BYTE_ARRAY)
              {
                // Get array size and alignment
                const auto &arrayType = std::get<ArrayType>(entry.type.data);
                int elementSize = 0;
                int numElements = arrayType.size;
                if (arrayType.element->kind == TypeKind::INT ||
                    arrayType.element->kind == TypeKind::UINT)
                {
                  elementSize = 4;
                }
                else if (arrayType.element->kind == TypeKind::LONG ||
                         arrayType.element->kind == TypeKind::ULONG ||
                         arrayType.element->kind == TypeKind::POINTER ||
                         arrayType.element->kind == TypeKind::DOUBLE)
                {
                  elementSize = 8;
                }
                else if (arrayType.element->kind == TypeKind::CHAR ||
                         arrayType.element->kind == TypeKind::UCHAR ||
                         arrayType.element->kind == TypeKind::SCHAR)
                {
                  elementSize = 1;
                }
                else if (arrayType.element->kind == TypeKind::ARRAY)
                {
                  // Nested array - recursively get the scalar element type
                  Type* scalarType = arrayType.element.get();
                  while (scalarType->kind == TypeKind::ARRAY)
                  {
                    numElements *= std::get<ArrayType>(scalarType->data).size;
                    scalarType = std::get<ArrayType>(scalarType->data).element.get();
                  }
                  
                  // Determine scalar element size and alignment
                  if (scalarType->kind == TypeKind::UCHAR || scalarType->kind == TypeKind::CHAR ||
                      scalarType->kind == TypeKind::SCHAR)
                  {
                    elementSize = 1;
                  }
                  else if (scalarType->kind == TypeKind::INT || scalarType->kind == TypeKind::UINT)
                  {
                    elementSize = 4;
                  }
                  else if (scalarType->kind == TypeKind::LONG || scalarType->kind == TypeKind::ULONG ||
                           scalarType->kind == TypeKind::DOUBLE || scalarType->kind == TypeKind::POINTER)
                  {
                    elementSize = 8;
                  }
                }

                int totalSize = elementSize * numElements;

                // Determine alignment based on total size and scalar element size
                int alignment;
                if (totalSize >= 16)
                {
                  alignment = 16;
                }
                else
                {
                  alignment = elementSize;
                }

                // Align offset
                offset += totalSize;
                offset = (offset + alignment - 1) & ~(alignment - 1);
                offset_table[pseudoMem->name] = offset;
              }
            }

            // Compute combined offset
            int combinedOffset = offset_table[pseudoMem->name] - pseudoMem->offset;
            instruction->dst = Memory::createMemory(RegisterType::BP, combinedOffset);
          }
        }
      }

      // Replace src1
      if (instruction->src1)
      {
        if (auto pseudoMem = std::dynamic_pointer_cast<PseudoMem>(instruction->src1))
        {
          // Check if this is a static array
          if (global_symbol_table.find(pseudoMem->name) != global_symbol_table.end() &&
              global_symbol_table[pseudoMem->name].storageClass == StorageClass::STATIC)
          {
            // Static array - convert to Data operand (offset must be 0)
            if (pseudoMem->offset != 0)
            {
              std::cerr << "Error: PseudoMem with nonzero offset for static array "
                        << pseudoMem->name << std::endl;
              exit(1);
            }
            instruction->src1 = std::make_shared<Data>(pseudoMem->name);
          }
          else
          {
            // Automatic storage duration array
            // Allocate the array if not already allocated
            if (offset_table.find(pseudoMem->name) == offset_table.end())
            {
              auto &entry = global_symbol_table[pseudoMem->name];
              if (entry.assemblyType == AssemblyType::BYTE_ARRAY)
              {
                // Get array size and alignment
                const auto &arrayType = std::get<ArrayType>(entry.type.data);
                int elementSize = 0;
                int numElements = arrayType.size;
                if (arrayType.element->kind == TypeKind::INT ||
                    arrayType.element->kind == TypeKind::UINT)
                {
                  elementSize = 4;
                }
                else if (arrayType.element->kind == TypeKind::LONG ||
                         arrayType.element->kind == TypeKind::ULONG ||
                         arrayType.element->kind == TypeKind::POINTER ||
                         arrayType.element->kind == TypeKind::DOUBLE)
                {
                  elementSize = 8;
                }
                else if (arrayType.element->kind == TypeKind::CHAR ||
                         arrayType.element->kind == TypeKind::UCHAR ||
                         arrayType.element->kind == TypeKind::SCHAR)
                {
                  elementSize = 1;
                }
                else if (arrayType.element->kind == TypeKind::ARRAY)
                {
                  // Nested array - recursively get the scalar element type
                  Type* scalarType = arrayType.element.get();
                  while (scalarType->kind == TypeKind::ARRAY)
                  {
                    numElements *= std::get<ArrayType>(scalarType->data).size;
                    scalarType = std::get<ArrayType>(scalarType->data).element.get();
                  }
                  
                  // Determine scalar element size and alignment
                  if (scalarType->kind == TypeKind::UCHAR || scalarType->kind == TypeKind::CHAR ||
                      scalarType->kind == TypeKind::SCHAR)
                  {
                    elementSize = 1;
                  }
                  else if (scalarType->kind == TypeKind::INT || scalarType->kind == TypeKind::UINT)
                  {
                    elementSize = 4;
                  }
                  else if (scalarType->kind == TypeKind::LONG || scalarType->kind == TypeKind::ULONG ||
                           scalarType->kind == TypeKind::DOUBLE || scalarType->kind == TypeKind::POINTER)
                  {
                    elementSize = 8;
                  }
                }

                int totalSize = elementSize * numElements;

                // Determine alignment based on total size and scalar element size
                int alignment;
                if (totalSize >= 16)
                {
                  alignment = 16;
                }
                else
                {
                  alignment = elementSize;
                }

                // Align offset
                offset += totalSize;
                offset = (offset + alignment - 1) & ~(alignment - 1);
                offset_table[pseudoMem->name] = offset;
              }
            }

            // Compute combined offset
            int combinedOffset = offset_table[pseudoMem->name] - pseudoMem->offset;
            instruction->src1 = Memory::createMemory(RegisterType::BP, combinedOffset);
          }
        }
      }

      // Replace src2
      if (instruction->src2)
      {
        if (auto pseudoMem = std::dynamic_pointer_cast<PseudoMem>(instruction->src2))
        {
          // Check if this is a static array
          if (global_symbol_table.find(pseudoMem->name) != global_symbol_table.end() &&
              global_symbol_table[pseudoMem->name].storageClass == StorageClass::STATIC)
          {
            // Static array - convert to Data operand (offset must be 0)
            if (pseudoMem->offset != 0)
            {
              std::cerr << "Error: PseudoMem with nonzero offset for static array "
                        << pseudoMem->name << std::endl;
              exit(1);
            }
            instruction->src2 = std::make_shared<Data>(pseudoMem->name);
          }
          else
          {
            // Automatic storage duration array
            // Allocate the array if not already allocated
            if (offset_table.find(pseudoMem->name) == offset_table.end())
            {
              auto &entry = global_symbol_table[pseudoMem->name];
              if (entry.assemblyType == AssemblyType::BYTE_ARRAY)
              {
                // Get array size and alignment
                const auto &arrayType = std::get<ArrayType>(entry.type.data);
                int elementSize = 0;
                int numElements = arrayType.size;
                if (arrayType.element->kind == TypeKind::INT ||
                    arrayType.element->kind == TypeKind::UINT)
                {
                  elementSize = 4;
                }
                else if (arrayType.element->kind == TypeKind::LONG ||
                         arrayType.element->kind == TypeKind::ULONG ||
                         arrayType.element->kind == TypeKind::POINTER ||
                         arrayType.element->kind == TypeKind::DOUBLE)
                {
                  elementSize = 8;
                }
                else if (arrayType.element->kind == TypeKind::CHAR ||
                         arrayType.element->kind == TypeKind::UCHAR ||
                         arrayType.element->kind == TypeKind::SCHAR)
                {
                  elementSize = 1;
                }
                else if (arrayType.element->kind == TypeKind::ARRAY)
                {
                  // Nested array - recursively get the scalar element type
                  Type* scalarType = arrayType.element.get();
                  while (scalarType->kind == TypeKind::ARRAY)
                  {
                    numElements *= std::get<ArrayType>(scalarType->data).size;
                    scalarType = std::get<ArrayType>(scalarType->data).element.get();
                  }
                  
                  // Determine scalar element size and alignment
                  if (scalarType->kind == TypeKind::UCHAR || scalarType->kind == TypeKind::CHAR ||
                      scalarType->kind == TypeKind::SCHAR)
                  {
                    elementSize = 1;
                  }
                  else if (scalarType->kind == TypeKind::INT || scalarType->kind == TypeKind::UINT)
                  {
                    elementSize = 4;
                  }
                  else if (scalarType->kind == TypeKind::LONG || scalarType->kind == TypeKind::ULONG ||
                           scalarType->kind == TypeKind::DOUBLE || scalarType->kind == TypeKind::POINTER)
                  {
                    elementSize = 8;
                  }
                }

                int totalSize = elementSize * numElements;

                // Determine alignment based on total size and scalar element size
                int alignment;
                if (totalSize >= 16)
                {
                  alignment = 16;
                }
                else
                {
                  alignment = elementSize;
                }

                // Align offset
                offset += totalSize;
                offset = (offset + alignment - 1) & ~(alignment - 1);
                offset_table[pseudoMem->name] = offset;
              }
            }

            // Compute combined offset
            int combinedOffset = offset_table[pseudoMem->name] - pseudoMem->offset;
            instruction->src2 = Memory::createMemory(RegisterType::BP, combinedOffset);
          }
        }
      }

      //   ---- Pseudo replacement ------
      // Replace dst
      if (instruction->dst)
      {
        if (auto pseudo = std::dynamic_pointer_cast<Pseudo>(instruction->dst))
        {
          if (offset_table.find(pseudo->name) == offset_table.end())
          {
            if (global_symbol_table.find(pseudo->name) !=
                    global_symbol_table.end() &&
                global_symbol_table[pseudo->name].storageClass ==
                    StorageClass::STATIC)
            {
              instruction->dst = std::make_shared<Data>(pseudo->name);
            }
            else
            {
              if (global_symbol_table[pseudo->name].assemblyType ==
                      AssemblyType::QUAD_WORD ||
                  global_symbol_table[pseudo->name].assemblyType ==
                      AssemblyType::DOUBLE_WORD)
              {
                offset = (offset + 7) & ~7; // Align to 8 bytes
                offset += 8;
              }
              // will always be 4 byte aligned since minimum size of type is 4
              // bytes
              else
              {
                offset = (offset + 3) & ~3; // Align to 4 bytes
                offset += 4;
              }
              offset_table[pseudo->name] = offset; // Negative offset from RBP
              instruction->dst =
                  Memory::createMemory(RegisterType::BP, offset_table[pseudo->name]);
            }
          }
          else
          {
            instruction->dst =
                Memory::createMemory(RegisterType::BP, offset_table[pseudo->name]);
          }
        }
      }

      // Replace src1
      if (instruction->src1)
      {
        if (auto pseudo =
                std::dynamic_pointer_cast<Pseudo>(instruction->src1))
        {
          if (offset_table.find(pseudo->name) == offset_table.end())
          {
            if (global_symbol_table.find(pseudo->name) !=
                    global_symbol_table.end() &&
                global_symbol_table[pseudo->name].storageClass ==
                    StorageClass::STATIC)
            {
              instruction->src1 = std::make_shared<Data>(pseudo->name);
            }
            else
            {
              if (global_symbol_table[pseudo->name].assemblyType ==
                      AssemblyType::QUAD_WORD ||
                  global_symbol_table[pseudo->name].assemblyType ==
                      AssemblyType::DOUBLE_WORD)
              {
                offset = (offset + 7) & ~7; // Align to 8 bytes
                offset += 8;
              }
              // will always be 4 byte aligned since minimum size of type is 4
              // bytes
              else
              {
                offset = (offset + 3) & ~3; // Align to 4 bytes
                offset += 4;
              }
              offset_table[pseudo->name] = offset; // Negative offset from RBP
              instruction->src1 =
                  Memory::createMemory(RegisterType::BP, offset_table[pseudo->name]);
            }
          }
          else
          {
            instruction->src1 =
                Memory::createMemory(RegisterType::BP, offset_table[pseudo->name]);
          }
        }
      }

      // Replace src2
      if (instruction->src2)
      {
        if (auto pseudo =
                std::dynamic_pointer_cast<Pseudo>(instruction->src2))
        {
          if (offset_table.find(pseudo->name) == offset_table.end())
          {
            if (global_symbol_table.find(pseudo->name) !=
                    global_symbol_table.end() &&
                global_symbol_table[pseudo->name].storageClass ==
                    StorageClass::STATIC)
            {
              instruction->src2 = std::make_shared<Data>(pseudo->name);
            }
            else
            {
              if (global_symbol_table[pseudo->name].assemblyType ==
                      AssemblyType::QUAD_WORD ||
                  global_symbol_table[pseudo->name].assemblyType ==
                      AssemblyType::DOUBLE_WORD)
              {
                offset = (offset + 7) & ~7; // Align to 8 bytes
                offset += 8;
              }
              // will always be 4 byte aligned since minimum size of type is 4
              // bytes
              else
              {
                offset = (offset + 3) & ~3; // Align to 4 bytes
                offset += 4;
              }
              offset_table[pseudo->name] = offset; // Negative offset from RBP
              instruction->src2 =
                  Memory::createMemory(RegisterType::BP, offset_table[pseudo->name]);
            }
          }
          else
          {
            instruction->src2 =
                Memory::createMemory(RegisterType::BP, offset_table[pseudo->name]);
          }
        }
      }
    }
  }
  return offset;
}

void Codegen::finalPass(ASMBasePtr ast)
{
  // Implementation for final optimizations and adjustments
  if (auto program = std::dynamic_pointer_cast<ASMProgram>(ast))
  {
    for (auto &topLevel : program->topLevelItems)
    {
      finalPass(topLevel);
    }
  }
  else if (auto function = std::dynamic_pointer_cast<ASMFunction>(ast))
  {
    auto newinstructions = std::vector<ASMInstructionPtr>{};
    for (auto &instruction : function->instructions)
    {
      switch (instruction->opType)
      {
      case ASMOpType::MOV:
      {
        if (instruction->assemblyType == AssemblyType::DOUBLE_WORD)
        {
          if (isMemoryAddress(instruction->dst))
          {
            if (isMemoryAddress(instruction->src1))
            {
              // mov can't contain both dst and src as address
              newinstructions.push_back(ASMInstruction::createMov(
                  Reg::createRegister(RegisterType::XMM14), instruction->src1,
                  instruction->assemblyType));
              instruction->src1 = Reg::createRegister(RegisterType::XMM14);
            }
          }
          newinstructions.push_back(instruction);
          break;
        }
        // converting 8 byte immediate values in src to 4 bytes
        if (auto imm = dynamic_pointer_cast<Immediate>(instruction->src1))
        {
          if (instruction->assemblyType == AssemblyType::LONG_WORD)
          {
            if ((int64_t)imm->value > std::numeric_limits<int32_t>::max() ||
                (int64_t)imm->value < std::numeric_limits<int32_t>::min())
            {
              // truncate to 4 bytes
              instruction->src1 = Immediate::createImmediate(
                  static_cast<int32_t>(imm->value) & 0xFFFFFFFF);
            }
          }
          else
          {
            if ((int64_t)imm->value > std::numeric_limits<int32_t>::max() ||
                (int64_t)imm->value < std::numeric_limits<int32_t>::min())
            {
              if (isMemoryAddress(instruction->dst))
              {
                // move immediate to R10 and then to instruction dst
                newinstructions.push_back(ASMInstruction::createMov(
                    Reg::createRegister(RegisterType::R10), instruction->src1,
                    AssemblyType::QUAD_WORD));
                instruction->src1 = Reg::createRegister(RegisterType::R10);
              }
            }
          }
        }
        if (instruction->dst && instruction->src1)
        {
          if (isMemoryAddress(instruction->dst))
          {
            if (isMemoryAddress(instruction->src1))
            {
              // mov can't contain both dst and src as address
              newinstructions.push_back(ASMInstruction::createMov(
                  Reg::createRegister(RegisterType::R10), instruction->src1,
                  instruction->assemblyType));
              instruction->src1 = Reg::createRegister(RegisterType::R10);
            }
          }
        }
        newinstructions.push_back(instruction);
        break;
      }
      case ASMOpType::BINARY:
      {
        if (instruction->assemblyType == AssemblyType::DOUBLE_WORD)
        {
          if (instruction->binaryOpType == BinaryOpType::XOR)
          {
            // XOR for doubles - used for negation by flipping sign bit
            // requires src to have reg or 16 byte aligned memory as its
            // argument apart from dst being a reg
            if (!dynamic_cast<Reg *>(instruction->dst.get()))
            {
              // Load dst into XMM15
              newinstructions.push_back(ASMInstruction::createMov(
                  Reg::createRegister(RegisterType::XMM15), instruction->dst,
                  instruction->assemblyType));
              // Change dst to XMM15
              auto temp = instruction->dst;
              instruction->dst = Reg::createRegister(RegisterType::XMM15);
              // Execute XOR
              newinstructions.push_back(instruction);
              // Store result back
              newinstructions.push_back(ASMInstruction::createMov(
                  temp, Reg::createRegister(RegisterType::XMM15),
                  instruction->assemblyType));
            }
            else
            {
              newinstructions.push_back(instruction);
            }
            break;
          }
          if (!dynamic_cast<Reg *>(instruction->dst.get()))
          {
            newinstructions.push_back(ASMInstruction::createMov(
                Reg::createRegister(RegisterType::XMM15), instruction->dst,
                instruction->assemblyType));
            auto temp = instruction->dst;
            instruction->dst = Reg::createRegister(RegisterType::XMM15);
            newinstructions.push_back(instruction);
            newinstructions.push_back(ASMInstruction::createMov(
                temp, Reg::createRegister(RegisterType::XMM15),
                instruction->assemblyType));
            break;
          }
          newinstructions.push_back(instruction);
          break;
        }
        if (instruction->assemblyType == AssemblyType::QUAD_WORD)
        {
          if (auto imm = dynamic_pointer_cast<Immediate>(instruction->src2))
          {
            if ((int64_t)imm->value > std::numeric_limits<int32_t>::max() ||
                (int64_t)imm->value < std::numeric_limits<int32_t>::min())
            {
              // move immediate to R10 and then to instruction src2
              newinstructions.push_back(ASMInstruction::createMov(
                  Reg::createRegister(RegisterType::R10), instruction->src2,
                  AssemblyType::QUAD_WORD));
              instruction->src2 = Reg::createRegister(RegisterType::R10);
            }
          }
        }
        if (instruction->binaryOpType == BinaryOpType::MULT)
        {
          if (isMemoryAddress(instruction->dst))
          {
            // move address to R11
            auto inst = std::make_shared<ASMInstruction>();
            inst->opType = ASMOpType::MOV;
            inst->dst = Reg::createRegister(RegisterType::R11);
            inst->src1 = instruction->dst;
            inst->assemblyType = instruction->assemblyType;
            newinstructions.push_back(inst);
            instruction->dst = Reg::createRegister(RegisterType::R11);
            newinstructions.push_back(instruction);
            newinstructions.push_back(ASMInstruction::createMov(
                inst->src1, Reg::createRegister(RegisterType::R11),
                instruction->assemblyType));
          }
          else
          {
            newinstructions.push_back(instruction);
          }
        }
        else if (instruction->binaryOpType == BinaryOpType::LEFT_SHIFT ||
                 instruction->binaryOpType == BinaryOpType::RIGHT_SHIFT)
        {
          if (!dynamic_pointer_cast<Immediate>(instruction->src2))
          {
            // move src2 to ECX
            newinstructions.push_back(ASMInstruction::createMov(
                Reg::createRegister(RegisterType::CX), instruction->src2,
                instruction->assemblyType));
            instruction->src2 = Reg::createRegister(RegisterType::CX);
          }
          newinstructions.push_back(instruction);
        }
        else
        {
          if (isMemoryAddress(instruction->dst))
          {
            if (isMemoryAddress(instruction->src2))
            {
              // mov can't contain both dst and src as address
              newinstructions.push_back(ASMInstruction::createMov(
                  Reg::createRegister(RegisterType::R10), instruction->src2,
                  instruction->assemblyType));
              instruction->src2 = Reg::createRegister(RegisterType::R10);
            }
          }
          newinstructions.push_back(instruction);
        }
        break;
      }
      case ASMOpType::IDIV:
      {
        if (dynamic_pointer_cast<Immediate>(instruction->src1))
        {
          // move immediate to R10
          newinstructions.push_back(ASMInstruction::createMov(
              Reg::createRegister(RegisterType::R10), instruction->src1,
              instruction->assemblyType));
          instruction->src1 = Reg::createRegister(RegisterType::R10);
        }
        newinstructions.push_back(instruction);
        break;
      }
      case ASMOpType::DIV:
      {
        if (dynamic_pointer_cast<Immediate>(instruction->src1))
        {
          // move immediate to R10
          newinstructions.push_back(ASMInstruction::createMov(
              Reg::createRegister(RegisterType::R10), instruction->src1,
              instruction->assemblyType));
          instruction->src1 = Reg::createRegister(RegisterType::R10);
        }
        newinstructions.push_back(instruction);
        break;
      }
      case ASMOpType::CMP:
      {
        if (instruction->assemblyType == AssemblyType::DOUBLE_WORD)
        {
          if (!dynamic_cast<Reg *>(instruction->src1.get()))
          {
            newinstructions.push_back(ASMInstruction::createMov(
                Reg::createRegister(RegisterType::XMM15), instruction->src1,
                instruction->assemblyType));
            instruction->src1 = Reg::createRegister(RegisterType::XMM15);
          }
          newinstructions.push_back(instruction);
          break;
        }
        if (isMemoryAddress(instruction->src1))
        {
          if (isMemoryAddress(instruction->src2))
          {
            // cmp can't contain both src1 and src2 as address
            newinstructions.push_back(ASMInstruction::createMov(
                Reg::createRegister(RegisterType::R10), instruction->src2,
                instruction->assemblyType));
            instruction->src2 = Reg::createRegister(RegisterType::R10);
          }
        }
        if (dynamic_cast<Immediate *>(instruction->src1.get()))
        {
          newinstructions.push_back(ASMInstruction::createMov(
              Reg::createRegister(RegisterType::R11), instruction->src1,
              instruction->assemblyType));
          instruction->src1 = Reg::createRegister(RegisterType::R11);
        }
        if (instruction->assemblyType == AssemblyType::QUAD_WORD)
        {
          if (auto imm = dynamic_pointer_cast<Immediate>(instruction->src2))
          {
            if ((int64_t)imm->value > std::numeric_limits<int32_t>::max() ||
                (int64_t)imm->value < std::numeric_limits<int32_t>::min())
            {
              // move immediate to R10 and then to instruction src2
              newinstructions.push_back(ASMInstruction::createMov(
                  Reg::createRegister(RegisterType::R10), instruction->src2,
                  AssemblyType::QUAD_WORD));
              instruction->src2 = Reg::createRegister(RegisterType::R10);
            }
          }
        }
        newinstructions.push_back(instruction);
        break;
      }
      case ASMOpType::MOVSX:
      {
        if (instruction->src1 &&
            dynamic_cast<Immediate *>(instruction->src1.get()))
        {
          newinstructions.push_back(ASMInstruction::createMov(
              Reg::createRegister(RegisterType::R10), instruction->src1,
              AssemblyType::LONG_WORD));
          instruction->src1 = Reg::createRegister(RegisterType::R10);
        }
        if (isMemoryAddress(instruction->dst))
        {
          // move address to R11
          auto d = instruction->dst;
          instruction->dst = Reg::createRegister(RegisterType::R11);
          newinstructions.push_back(instruction);
          newinstructions.push_back(ASMInstruction::createMov(
              d, Reg::createRegister(RegisterType::R11),
              AssemblyType::QUAD_WORD));
        }
        else
        {
          newinstructions.push_back(instruction);
        }
        break;
      }
      case ASMOpType::PUSH:
      {
        if (auto imm = dynamic_pointer_cast<Immediate>(instruction->src1))
        {
          if ((int64_t)imm->value > std::numeric_limits<int32_t>::max() ||
              (int64_t)imm->value < std::numeric_limits<int32_t>::min())
          {
            // move immediate to R10 and then to instruction src1
            newinstructions.push_back(ASMInstruction::createMov(
                Reg::createRegister(RegisterType::R10), instruction->src1,
                AssemblyType::QUAD_WORD));
            instruction->src1 = Reg::createRegister(RegisterType::R10);
          }
          newinstructions.push_back(instruction);
        }
        else if (auto reg = dynamic_cast<Reg *>(instruction->src1.get()))
        {
          if (reg->name == RegisterType::XMM0 || reg->name == RegisterType::XMM1 ||
              reg->name == RegisterType::XMM2 || reg->name == RegisterType::XMM3 ||
              reg->name == RegisterType::XMM4 || reg->name == RegisterType::XMM5 ||
              reg->name == RegisterType::XMM6 || reg->name == RegisterType::XMM7 ||
              reg->name == RegisterType::XMM8 || reg->name == RegisterType::XMM9 ||
              reg->name == RegisterType::XMM10 || reg->name == RegisterType::XMM11 ||
              reg->name == RegisterType::XMM12 || reg->name == RegisterType::XMM13 ||
              reg->name == RegisterType::XMM14 || reg->name == RegisterType::XMM15)
          {
            // move xmm to memory first
            newinstructions.push_back(ASMInstruction::createBinary(BinaryOpType::SUB,
                                                                   Reg::createRegister(RegisterType::SP), Immediate::createImmediate(8),
                                                                   AssemblyType::QUAD_WORD));
            newinstructions.push_back(ASMInstruction::createMov(
                Memory::createMemory(RegisterType::SP, -8), instruction->src1,
                AssemblyType::DOUBLE_WORD));
            instruction->src1 = Memory::createMemory(RegisterType::SP, -8);
          }
          else
          {
            newinstructions.push_back(instruction);
          }
        }
        else
        {
          newinstructions.push_back(instruction);
        }
        break;
      }
      case ASMOpType::MOVZEROEXTEND:
      {
        if (dynamic_cast<Reg *>(instruction->dst.get()))
        {
          newinstructions.push_back(ASMInstruction::createMov(
              instruction->dst, instruction->src1, AssemblyType::LONG_WORD));
        }
        else if (isMemoryAddress(instruction->dst))
        {
          newinstructions.push_back(ASMInstruction::createMov(
              Reg::createRegister(RegisterType::R11), instruction->src1,
              AssemblyType::LONG_WORD));
          newinstructions.push_back(ASMInstruction::createMov(
              instruction->dst, Reg::createRegister(RegisterType::R11),
              AssemblyType::QUAD_WORD));
        }
        break;
      }
      case ASMOpType::CVTTSD2SI:
      {
        if (!dynamic_cast<Reg *>(instruction->dst.get()))
        {
          newinstructions.push_back(ASMInstruction::createCvttsd2si(
              Reg::createRegister(RegisterType::R11), instruction->src1,
              instruction->assemblyType));
          newinstructions.push_back(ASMInstruction::createMov(
              instruction->dst, Reg::createRegister(RegisterType::R11),
              instruction->assemblyType));
        }
        else
        {
          newinstructions.push_back(instruction);
        }
        break;
      }
      case ASMOpType::CVTTSI2SD:
      {
        if (!dynamic_cast<Reg *>(instruction->dst.get()))
        {
          if (dynamic_cast<Immediate *>(instruction->src1.get()))
          {
            newinstructions.push_back(ASMInstruction::createMov(
                Reg::createRegister(RegisterType::R10), instruction->src1,
                instruction->assemblyType));
            instruction->src1 = Reg::createRegister(RegisterType::R10);
          }
          newinstructions.push_back(ASMInstruction::createCvttsi2sd(
              Reg::createRegister(RegisterType::XMM15), instruction->src1,
              instruction->assemblyType));
          newinstructions.push_back(ASMInstruction::createMov(
              instruction->dst, Reg::createRegister(RegisterType::XMM15),
              AssemblyType::DOUBLE_WORD));
        }
        else if (dynamic_cast<Immediate *>(instruction->src1.get()))
        {
          newinstructions.push_back(ASMInstruction::createMov(
              Reg::createRegister(RegisterType::R10), instruction->src1,
              instruction->assemblyType));
          instruction->src1 = Reg::createRegister(RegisterType::R10);
          newinstructions.push_back(instruction);
        }
        else
        {
          newinstructions.push_back(instruction);
        }
        break;
      }
      case ASMOpType::LEA:
      {
        // dst must be a register
        if (!dynamic_cast<Reg *>(instruction->dst.get()))
        {
          newinstructions.push_back(ASMInstruction::createLea(
              Reg::createRegister(RegisterType::R11), instruction->src1));
          newinstructions.push_back(ASMInstruction::createMov(
              instruction->dst, Reg::createRegister(RegisterType::R11),
              AssemblyType::QUAD_WORD));
        }
        else
        {
          newinstructions.push_back(instruction);
        }
        break;
      }
      default:
        newinstructions.push_back(instruction);
        break;
      }
    }
    function->instructions = std::move(newinstructions);
  }
}