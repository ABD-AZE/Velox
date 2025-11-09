#include "codegen.hpp"
std::unordered_map<std::string, int> offset_table;
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

AssemblyType getAssemblyType(IRValuePtr irValue){
  if(global_symbol_table.find(irValue->name) != global_symbol_table.end()){
    return global_symbol_table[irValue->name].assemblyType;
  }
  else{
    return irValue->constType == TypeKind::LONG ? AssemblyType::QUAD_WORD : AssemblyType::LONG_WORD;
  }
}

bool isMemoryAddress(const OperandPtr &operand)
{
  return (std::dynamic_pointer_cast<Stack>(operand) != nullptr ||
          std::dynamic_pointer_cast<Data>(operand) != nullptr);
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

ConditionCode binOptoConditionCode(IROpType binOp)
{
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
      else
        ss << "addl " << src2->toString() << ", " << dst->toString();
      break;
    case BinaryOpType::SUB:
      if (assemblyType == AssemblyType::QUAD_WORD)
        ss << "subq " << src2->toString() << ", " << dst->toString();
      else
        ss << "subl " << src2->toString() << ", " << dst->toString();
      break;
    case BinaryOpType::MULT:
      if (assemblyType == AssemblyType::QUAD_WORD)
        ss << "imulq " << src2->toString() << ", " << dst->toString();
      else
        ss << "imull " << src2->toString() << ", " << dst->toString();
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
      else
        ss << "xorl " << src2->toString() << ", " << dst->toString();
      break;
    case BinaryOpType::LEFT_SHIFT:
      oneByte = 1;
      if (assemblyType == AssemblyType::QUAD_WORD)
        ss << "salq " << src2->toString() << ", " << dst->toString();
      else
        ss << "sall " << src2->toString() << ", " << dst->toString();
      oneByte = 0;
      break;
    case BinaryOpType::RIGHT_SHIFT:
      oneByte = 1;
      if (assemblyType == AssemblyType::QUAD_WORD)
        ss << "sarq " << src2->toString() << ", " << dst->toString();
      else
        ss << "sarl " << src2->toString() << ", " << dst->toString();
      oneByte = 0;
      break;
    }
    break;
  case ASMOpType::IDIV:
    if (assemblyType == AssemblyType::QUAD_WORD)
      ss << "idivq " << src1->toString();
    else
      ss << "idivl " << src1->toString();
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
    if (auto irFunction = std::dynamic_pointer_cast<IRFunctionNode>(irTopLevel))
    {
      auto asmFunction = IRFunctionToASM(irFunction);
      asmProgram->topLevelItems.push_back(asmFunction);
    }
    else if (auto irStaticVar = std::dynamic_pointer_cast<IRStaticVariableNode>(irTopLevel))
    {
      auto init = irStaticVar->init_list[0];
      int alignment = init.data.index() == 0 ? 4 : 8;
      auto asmStaticVar = ASMStaticVariable::createStaticVariable(
          irStaticVar->identifier, irStaticVar->global, alignment, init);
      asmProgram->topLevelItems.push_back(asmStaticVar);
    }
  }
  return asmProgram;
}

ASMFunctionPtr Codegen::IRFunctionToASM(const IRFunctionPtr &irFunction)
{
  auto asmFunction = std::make_shared<ASMFunction>();
  asmFunction->name = irFunction->identifier;
  asmFunction->global = irFunction->global;
  auto regs = std::vector<RegisterType>{
      RegisterType::DI, RegisterType::SI, RegisterType::DX,
      RegisterType::CX, RegisterType::R8, RegisterType::R9};
  for (size_t i = 0; i < irFunction->parameters.size(); i++)
  {
    if (i < 6)
    {
      asmFunction->instructions.push_back(ASMInstruction::createMov(Pseudo::createPseudo(global_symbol_table[irFunction->parameters[i]].name), Reg::createRegister(regs[i]), global_symbol_table[irFunction->parameters[i]].assemblyType));
    }
    else
    {
      asmFunction->instructions.push_back(ASMInstruction::createMov(Pseudo::createPseudo(global_symbol_table[irFunction->parameters[i]].name), Stack::createStack(-8 * (i - 4)), global_symbol_table[irFunction->parameters[i]].assemblyType));
    }
  }
  for (const auto &irInstruction : irFunction->instructions)
  {
    auto instrs = IRInstructionToASM(irInstruction);
    for (auto instr : instrs)
    {
      asmFunction->instructions.push_back(instr);
    }
  }
  int offset = replacePseudoRegisters(asmFunction);
  finalPass(asmFunction);
  // allocate stack size in multiple of 16
  auto allocateStackInstr = ASMInstruction::createAllocateStack(((offset + 15) / 16) * 16);
  asmFunction->instructions.insert(asmFunction->instructions.begin(), allocateStackInstr);
  asmFunction->stackSize = offset;
  return asmFunction;
}

std::vector<ASMInstructionPtr> Codegen::IRInstructionToASM(const IRInstructionPtr &irInstruction)
{
  std::vector<ASMInstructionPtr> asmInstructions;
  switch (irInstruction->opType)
  {
  case IROpType::NEGATE:
  {
    // First, move src to dst
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src1), type));

    // Then, negate the dst
    asmInstructions.push_back(ASMInstruction::createUnary(UnaryOpType::NEG, IRValueToOperand(irInstruction->dst), type));
    break;
  }
  case IROpType::COMPLEMENT:
  {
    // First, move src to dst
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src1), type));

    // Then, complement the dst
    asmInstructions.push_back(ASMInstruction::createUnary(UnaryOpType::NOT, IRValueToOperand(irInstruction->dst), type));
    break;
  }
  case IROpType::RETURN:
  {
    AssemblyType type = getAssemblyType(irInstruction->src1);
    asmInstructions.push_back(ASMInstruction::createMov(Reg::createRegister(RegisterType::AX), IRValueToOperand(irInstruction->src1), type));
    asmInstructions.push_back(ASMInstruction::createRet());
    break;
  }
  case IROpType::ADD:
  {
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src1), type));
    asmInstructions.push_back(ASMInstruction::createBinary(BinaryOpType::ADD, IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src2), type));
    break;
  }
  case IROpType::SUBTRACT:
  {
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src1), type));
    asmInstructions.push_back(ASMInstruction::createBinary(BinaryOpType::SUB, IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src2), type));
    break;
  }
  case IROpType::MULTIPLY:
  {
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src1), type));
    asmInstructions.push_back(ASMInstruction::createBinary(BinaryOpType::MULT, IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src2), type));
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
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(ASMInstruction::createMov(Reg::createRegister(RegisterType::AX), IRValueToOperand(irInstruction->src1), type));
    asmInstructions.push_back(ASMInstruction::createCDQ(type));

    asmInstructions.push_back(ASMInstruction::createIDiv(IRValueToOperand(irInstruction->src2), type));
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst), Reg::createRegister(RegisterType::AX), type));
    break;
  }
  case IROpType::REMAINDER:
  {
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(ASMInstruction::createMov(Reg::createRegister(RegisterType::AX), IRValueToOperand(irInstruction->src1), type));
    asmInstructions.push_back(ASMInstruction::createCDQ(type));

    asmInstructions.push_back(ASMInstruction::createIDiv(IRValueToOperand(irInstruction->src2), type));
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst), Reg::createRegister(RegisterType::DX), type));
    break;
  }
  case IROpType::AND:
  {
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src1), type));
    asmInstructions.push_back(ASMInstruction::createBinary(BinaryOpType::AND, IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src2), type));
    break;
  }
  case IROpType::OR:
  {
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src1), type));
    asmInstructions.push_back(ASMInstruction::createBinary(BinaryOpType::OR, IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src2), type));
    break;
  }
  case IROpType::XOR:
  {
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src1), type));
    asmInstructions.push_back(ASMInstruction::createBinary(BinaryOpType::XOR, IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src2), type));
    break;
  }
  case IROpType::LEFT_SHIFT:
  {
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src1), type));
    asmInstructions.push_back(ASMInstruction::createBinary(BinaryOpType::LEFT_SHIFT, IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src2), type));
    break;
  }
  case IROpType::RIGHT_SHIFT:
  {
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src1), type));
    asmInstructions.push_back(ASMInstruction::createBinary(BinaryOpType::RIGHT_SHIFT, IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src2), type));
    break;
  }
  case IROpType::EQUAL:
  case IROpType::NOT_EQUAL:
  case IROpType::GREATER_THAN:
  case IROpType::GREATER_EQUAL:
  case IROpType::LESS_THAN:
  case IROpType::LESS_EQUAL:
  {
    asmInstructions.push_back(ASMInstruction::createCmp(IRValueToOperand(irInstruction->src1), IRValueToOperand(irInstruction->src2), getAssemblyType(irInstruction->src1)));
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst), Immediate::createImmediate(0), AssemblyType::LONG_WORD));
    asmInstructions.push_back(ASMInstruction::createSetCC(binOptoConditionCode(irInstruction->opType), IRValueToOperand(irInstruction->dst)));
    break;
  }
  case IROpType::LABEL:
  {
    asmInstructions.push_back(ASMInstruction::createLabel(irInstruction->label));
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
    asmInstructions.push_back(ASMInstruction::createCmp(IRValueToOperand(irInstruction->src1), Immediate::createImmediate(0), type));
    asmInstructions.push_back(ASMInstruction::createJmpCC(ConditionCode::E, irInstruction->label));
    break;
  }
  case IROpType::JUMP_IF_NOT_ZERO:
  {
    AssemblyType type = getAssemblyType(irInstruction->src1);
    asmInstructions.push_back(ASMInstruction::createCmp(IRValueToOperand(irInstruction->src1), Immediate::createImmediate(0), type));
    asmInstructions.push_back(ASMInstruction::createJmpCC(ConditionCode::NE, irInstruction->label));
    break;
  }
  case IROpType::NOT:
  {
    AssemblyType type = getAssemblyType(irInstruction->src1);
    asmInstructions.push_back(ASMInstruction::createCmp(IRValueToOperand(irInstruction->src1), Immediate::createImmediate(0), type));
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst), Immediate::createImmediate(0), AssemblyType::LONG_WORD));
    asmInstructions.push_back(ASMInstruction::createSetCC(ConditionCode::E, IRValueToOperand(irInstruction->dst)));
    break;
  }
  case IROpType::COPY:
  {
    AssemblyType type = getAssemblyType(irInstruction->dst);
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src1), type));
    break;
  }
  case IROpType::CALL:
  {
    auto argRegisters = std::vector<RegisterType>{
        RegisterType::DI, RegisterType::SI, RegisterType::DX,
        RegisterType::CX, RegisterType::R8, RegisterType::R9};

    // Split arguments into register args and stack args
    std::vector<IRValuePtr> registerArgs;
    std::vector<IRValuePtr> stackArgs;

    for (size_t i = 0; i < irInstruction->src2->args.size(); ++i)
    {
      if (i < argRegisters.size())
      {
        registerArgs.push_back(irInstruction->src2->args[i]);
      }
      else
      {
        stackArgs.push_back(irInstruction->src2->args[i]);
      }
    }

    // Calculate stack padding for 16-byte alignment
    int stackPadding = 0;
    if (stackArgs.size() % 2 != 0)
    {
      stackPadding = 8;
    }

    // Allocate stack padding if needed
    if (stackPadding != 0)
    {
      auto allocInstr = ASMInstruction::createAllocateStack(stackPadding);
      asmInstructions.push_back(allocInstr);
    }

    // Pass arguments in registers
    for (size_t i = 0; i < registerArgs.size(); ++i)
    {
      AssemblyType type = getAssemblyType(registerArgs[i]);
      auto movInstr = ASMInstruction::createMov(Reg::createRegister(argRegisters[i]), IRValueToOperand(registerArgs[i]), type);
      asmInstructions.push_back(movInstr);
    }

    // Pass arguments on stack in reverse order
    for (auto it = stackArgs.rbegin(); it != stackArgs.rend(); ++it)
    {
      auto assemblyArg = IRValueToOperand(*it);
      AssemblyType type = getAssemblyType(*it);
      // Check if operand is a register or immediate
      bool isRegOrImm = (std::dynamic_pointer_cast<Reg>(assemblyArg) != nullptr) ||
                        (std::dynamic_pointer_cast<Immediate>(assemblyArg) != nullptr);

      if (isRegOrImm)
      {
        // Can push directly
        auto pushInstr = ASMInstruction::createPush(assemblyArg);
        asmInstructions.push_back(pushInstr);
      }
      else
      {
        // Need to move to AX first, then push
        auto movInstr = ASMInstruction::createMov(Reg::createRegister(RegisterType::AX), assemblyArg, type);
        asmInstructions.push_back(movInstr);

        auto pushInstr = ASMInstruction::createPush(Reg::createRegister(RegisterType::AX));
        asmInstructions.push_back(pushInstr);
      }
    }

    // Emit call instruction
    auto callInstr = ASMInstruction::createCall(irInstruction->src1->name);
    asmInstructions.push_back(callInstr);

    // Deallocate stack (remove args + padding)
    int bytesToRemove = 8 * stackArgs.size() + stackPadding;
    if (bytesToRemove != 0)
    {
      auto deallocInstr = ASMInstruction::createDeallocateStack(bytesToRemove);
      asmInstructions.push_back(deallocInstr);
    }

    // Retrieve return value (if dst is not null)
    if (irInstruction->dst)
    {
      AssemblyType type = getAssemblyType(irInstruction->dst);
      auto movInstr = ASMInstruction::createMov(IRValueToOperand(irInstruction->dst), Reg::createRegister(RegisterType::AX), type);
      asmInstructions.push_back(movInstr);
    }

    break;
  }
  case IROpType::TRUNCATE:
  {
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src1), AssemblyType::LONG_WORD));
    break;
  }
  case IROpType::SIGN_EXTEND:
  {
    asmInstructions.push_back(ASMInstruction::createMovsx(IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src1)));
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
    std::visit([&imm](auto &&arg)
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
      } }, irValue->value);
    return imm;
  }
  case IRValueType::VARIABLE:
  {
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
      // Replace dst
      if (instruction->dst)
      {
        if (auto pseudo = std::dynamic_pointer_cast<Pseudo>(instruction->dst))
        {
          if (offset_table.find(pseudo->name) == offset_table.end())
          {
            if (global_symbol_table.find(pseudo->name) != global_symbol_table.end() && global_symbol_table[pseudo->name].storageClass == StorageClass::STATIC)
            {
              instruction->dst = std::make_shared<Data>(pseudo->name);
            }
            else
            {
              if (global_symbol_table[pseudo->name].assemblyType == AssemblyType::QUAD_WORD)
              {
                offset = (offset + 7) & ~7; // Align to 8 bytes
                offset += 8;
              }
              // will always be 4 byte aligned since minimum size of type is 4 bytes
              else
              {
                offset = (offset + 3) & ~3; // Align to 4 bytes
                offset += 4;
              }
              offset_table[pseudo->name] = offset; // Negative offset from RBP
              instruction->dst = std::make_shared<Stack>(offset_table[pseudo->name]);
            }
          }
          else
          {
            instruction->dst = std::make_shared<Stack>(offset_table[pseudo->name]);
          }
        }
      }

      // Replace src1
      if (instruction->src1)
      {
        if (auto pseudo = std::dynamic_pointer_cast<Pseudo>(instruction->src1))
        {
          if (offset_table.find(pseudo->name) == offset_table.end())
          {
            if (global_symbol_table.find(pseudo->name) != global_symbol_table.end() && global_symbol_table[pseudo->name].storageClass == StorageClass::STATIC)
            {
              instruction->src1 = std::make_shared<Data>(pseudo->name);
            }
            else
            {
              if (global_symbol_table[pseudo->name].assemblyType == AssemblyType::QUAD_WORD)
              {
                offset = (offset + 7) & ~7; // Align to 8 bytes
                offset += 8;
              }
              // will always be 4 byte aligned since minimum size of type is 4 bytes
              else
              {
                offset = (offset + 3) & ~3; // Align to 4 bytes
                offset += 4;
              }
              offset_table[pseudo->name] = offset; // Negative offset from RBP
              instruction->src1 = std::make_shared<Stack>(offset_table[pseudo->name]);
            }
          }
          else
          {
            instruction->src1 = std::make_shared<Stack>(offset_table[pseudo->name]);
          }
        }
      }

      // Replace src2
      if (instruction->src2)
      {
        if (auto pseudo = std::dynamic_pointer_cast<Pseudo>(instruction->src2))
        {
          if (offset_table.find(pseudo->name) == offset_table.end())
          {
            if (global_symbol_table.find(pseudo->name) != global_symbol_table.end() && global_symbol_table[pseudo->name].storageClass == StorageClass::STATIC)
            {
              instruction->src2 = std::make_shared<Data>(pseudo->name);
            }
            else
            {
              if (global_symbol_table[pseudo->name].assemblyType == AssemblyType::QUAD_WORD)
              {
                offset = (offset + 7) & ~7; // Align to 8 bytes
                offset += 8;
              }
              // will always be 4 byte aligned since minimum size of type is 4 bytes
              else
              {
                offset = (offset + 3) & ~3; // Align to 4 bytes
                offset += 4;
              }
              offset_table[pseudo->name] = offset; // Negative offset from RBP
              instruction->src2 = std::make_shared<Stack>(offset_table[pseudo->name]);
            }
          }
          else
          {
            instruction->src2 = std::make_shared<Stack>(offset_table[pseudo->name]);
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
        // converting 8 byte immediate values in src to 4 bytes
        if (auto imm = dynamic_pointer_cast<Immediate>(instruction->src1))
        {
          if (instruction->assemblyType == AssemblyType::LONG_WORD)
          {
            if (imm->value > std::numeric_limits<int32_t>::max() || imm->value < std::numeric_limits<int32_t>::min())
            {
              // truncate to 4 bytes
              instruction->src1 = Immediate::createImmediate(static_cast<int32_t>(imm->value) & 0xFFFFFFFF);
            }
          }
          else
          {
            if (imm->value > std::numeric_limits<int32_t>::max() || imm->value < std::numeric_limits<int32_t>::min())
            {
              if (isMemoryAddress(instruction->dst))
              {
                // move immediate to R10 and then to instruction dst
                newinstructions.push_back(ASMInstruction::createMov(Reg::createRegister(RegisterType::R10), instruction->src1, AssemblyType::QUAD_WORD));
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
              newinstructions.push_back(ASMInstruction::createMov(Reg::createRegister(RegisterType::R10), instruction->src1, instruction->assemblyType));
              instruction->src1 = Reg::createRegister(RegisterType::R10);
            }
          }
        }
        newinstructions.push_back(instruction);
        break;
      }
      case ASMOpType::BINARY:
      {
        if (instruction->assemblyType == AssemblyType::QUAD_WORD)
        {
          if (auto imm = dynamic_pointer_cast<Immediate>(instruction->src2))
          {
            if (imm->value > std::numeric_limits<int32_t>::max() || imm->value < std::numeric_limits<int32_t>::min())
            {
              // move immediate to R10 and then to instruction src2
              newinstructions.push_back(ASMInstruction::createMov(Reg::createRegister(RegisterType::R10), instruction->src2, AssemblyType::QUAD_WORD));
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
            newinstructions.push_back(ASMInstruction::createMov(inst->src1, Reg::createRegister(RegisterType::R11), instruction->assemblyType));
          }
          else
          {
            newinstructions.push_back(instruction);
          }
        }
        else if (instruction->binaryOpType == BinaryOpType::LEFT_SHIFT || instruction->binaryOpType == BinaryOpType::RIGHT_SHIFT)
        {
          if (!dynamic_pointer_cast<Immediate>(instruction->src2))
          {
            // move src2 to ECX
            newinstructions.push_back(ASMInstruction::createMov(Reg::createRegister(RegisterType::CX), instruction->src2, instruction->assemblyType));
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
              newinstructions.push_back(ASMInstruction::createMov(Reg::createRegister(RegisterType::R10), instruction->src2, instruction->assemblyType));
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
          newinstructions.push_back(ASMInstruction::createMov(Reg::createRegister(RegisterType::R10), instruction->src1, instruction->assemblyType));
          instruction->src1 = Reg::createRegister(RegisterType::R10);
        }
        newinstructions.push_back(instruction);
        break;
      }
      case ASMOpType::CMP:
      {
        if (isMemoryAddress(instruction->src1))
        {
          if (isMemoryAddress(instruction->src2))
          {
            // cmp can't contain both src1 and src2 as address
            newinstructions.push_back(ASMInstruction::createMov(Reg::createRegister(RegisterType::R10), instruction->src2, instruction->assemblyType));
            instruction->src2 = Reg::createRegister(RegisterType::R10);
          }
        }
        // checking if the second operand is an immediate value
        if (auto src1Imm = dynamic_cast<Immediate *>(instruction->src1.get()))
        {
          newinstructions.push_back(ASMInstruction::createMov(Reg::createRegister(RegisterType::R11), instruction->src1, instruction->assemblyType));
          instruction->src1 = Reg::createRegister(RegisterType::R11);
        }
        if (instruction->assemblyType == AssemblyType::QUAD_WORD)
        {
          if (auto imm = dynamic_pointer_cast<Immediate>(instruction->src2))
          {
            if (imm->value > std::numeric_limits<int32_t>::max() || imm->value < std::numeric_limits<int32_t>::min())
            {
              // move immediate to R10 and then to instruction src2
              newinstructions.push_back(ASMInstruction::createMov(Reg::createRegister(RegisterType::R10), instruction->src2, AssemblyType::QUAD_WORD));
              instruction->src2 = Reg::createRegister(RegisterType::R10);
            }
          }
        }
        newinstructions.push_back(instruction);
        break;
      }
      case ASMOpType::MOVSX:
      {
        if (instruction->src1 && dynamic_cast<Immediate *>(instruction->src1.get()))
        {
          newinstructions.push_back(ASMInstruction::createMov(Reg::createRegister(RegisterType::R10), instruction->src1, AssemblyType::LONG_WORD));
          instruction->src1 = Reg::createRegister(RegisterType::R10);
        }
        if (isMemoryAddress(instruction->dst))
        {
          // move address to R11
          auto d = instruction->dst;
          instruction->dst = Reg::createRegister(RegisterType::R11);
          newinstructions.push_back(instruction);
          newinstructions.push_back(ASMInstruction::createMov(d, Reg::createRegister(RegisterType::R11), AssemblyType::QUAD_WORD));
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
          if (imm->value > std::numeric_limits<int32_t>::max() || imm->value < std::numeric_limits<int32_t>::min())
          {
            // move immediate to R10 and then to instruction src1
            newinstructions.push_back(ASMInstruction::createMov(Reg::createRegister(RegisterType::R10), instruction->src1, AssemblyType::QUAD_WORD));
            instruction->src1 = Reg::createRegister(RegisterType::R10);
          }
        }
        newinstructions.push_back(instruction);
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