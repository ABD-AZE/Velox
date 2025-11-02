#include "codegen.hpp"
#define TAB "    "
std::unordered_map<std::string, int> offset_table;
bool oneByte = 0;

std::string ASMProgram::toString() const {
  std::stringstream ss;
  for (const auto &function : functions) {
    ss << function->toString() << "\n";
  }
  // disable stack execution
  ss << TAB << ".section .note.GNU-stack,\"\",@progbits\n";
  return ss.str();
}

std::string ASMFunction::toString() const {
  std::stringstream ss;
  ss << ".globl " << name << "\n";
  ss << name << ":\n";
  ss << TAB << "pushq %rbp\n";
  ss << TAB << "movq %rsp, %rbp\n";
  for (const auto &instruction : instructions) {
    ss << TAB << instruction->toString() << "\n";
  }
  return ss.str();
}

ConditionCode binOptoConditionCode(   IROpType binOp) {
  switch (binOp) {
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

std::string ASMInstruction::toString() const {
  std::stringstream ss;
  switch (opType) {
  case ASMOpType::MOV:
    ss << "movl " << src1->toString() << ", " << dst->toString();
    break;
  case ASMOpType::UNARY:
    switch (unaryOpType) {
    case UnaryOpType::NEG:
      ss << "negl " << src1->toString();
      break;
    case UnaryOpType::NOT:
      ss << "notl " << src1->toString();
      break;
    }
    break;
  case ASMOpType::ALLOCATE_STACK:
    ss << "subq " << src1->toString() << ", %rsp";
    break;
  case ASMOpType::RET:
    ss << "movq %rbp, %rsp\n";
    ss << TAB << "popq %rbp\n";
    ss << TAB<< "ret";
    break;
  case ASMOpType::BINARY:
    switch (binaryOpType) {
    case BinaryOpType::ADD:
      ss << "addl " << src2->toString() << ", " << dst->toString();
      break;
    case BinaryOpType::SUB:
      ss << "subl " << src2->toString() << ", " << dst->toString();
      break;
    case BinaryOpType::MULT:
      ss << "imull " << src2->toString() << ", " << dst->toString();
      break;
    case BinaryOpType::AND:
      ss << "andl " << src2->toString() << ", " << dst->toString();
      break;
    case BinaryOpType::OR:
      ss << "orl " << src2->toString() << ", " << dst->toString();
      break;  
    case BinaryOpType::XOR:
      ss << "xorl " << src2->toString() << ", " << dst->toString();
      break;
    case BinaryOpType::LEFT_SHIFT:
      oneByte = 1;
      ss << "sall " << src2->toString() << ", " << dst->toString();
      oneByte = 0;
      break;
    case BinaryOpType::RIGHT_SHIFT:
      oneByte = 1;
      ss << "sarl " << src2->toString() << ", " << dst->toString();
      oneByte = 0;
      break;
    }
    break;
  case ASMOpType::IDIV:
    ss << "idivl " << src1->toString();
    break;
  case ASMOpType::CDQ:
    ss << "cdq";
    break;
  case ASMOpType::CMP:
    ss << "cmpl " << src2->toString() << ", " << src1->toString();
    break;
  case ASMOpType::JMP:
    ss << "jmp " << ".L" << label;
    break;
  case ASMOpType::JMPCC:
    switch (conditionCode) {
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
    switch (conditionCode) {
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
  default:
    break;
  }
  return ss.str();
}

ASMProgramPtr Codegen::generateCode(IRProgramPtr &irProgram){
  auto asmProgram = IRProgramtoASM(irProgram);
  return asmProgram;
}

ASMProgramPtr Codegen::IRProgramtoASM(const IRProgramPtr &irProgram){
  auto asmProgram = std::make_shared<ASMProgram>();
  for (const auto &irTopLevel : irProgram->topLevelItems) {
    auto asmFunction = IRFunctionToASM(std::dynamic_pointer_cast<IRFunctionNode>(irTopLevel));
    asmProgram->functions.push_back(asmFunction);
  }
  return asmProgram;
}

ASMFunctionPtr Codegen::IRFunctionToASM(const IRFunctionPtr &irFunction){
  auto asmFunction = std::make_shared<ASMFunction>();
  asmFunction->name = irFunction->identifier;
  for (const auto &irInstruction : irFunction->instructions) {
    auto instrs = IRInstructionToASM(irInstruction);
    for(auto instr : instrs){
      asmFunction->instructions.push_back(instr);
    }
  }
  int offset = replacePseudoRegisters(asmFunction);
  finalPass(asmFunction);
  // allocate stack
  auto allocateStackInstr = std::make_shared<ASMInstruction>();
  allocateStackInstr->opType = ASMOpType::ALLOCATE_STACK;
  allocateStackInstr->src1 = Immediate::createImmediate(offset);
  asmFunction->instructions.insert(asmFunction->instructions.begin(), allocateStackInstr);
  return asmFunction;
}

std::vector<ASMInstructionPtr> Codegen::IRInstructionToASM(const IRInstructionPtr &irInstruction){
  std::vector<ASMInstructionPtr> asmInstructions;
  switch (irInstruction->opType) {
  case IROpType::NEGATE:{
    // First, move src to dst
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src1)));
    
    // Then, negate the dst
    asmInstructions.push_back(ASMInstruction::createUnary(UnaryOpType::NEG, IRValueToOperand(irInstruction->dst)));
    break;
  }
  case IROpType::COMPLEMENT: {
    // First, move src to dst
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src1)));

    // Then, complement the dst
    asmInstructions.push_back(ASMInstruction::createUnary(UnaryOpType::NOT, IRValueToOperand(irInstruction->dst)));
    break;
  }
  case IROpType::RETURN: {
    asmInstructions.push_back(ASMInstruction::createMov(Reg::createRegister(RegisterType::AX),IRValueToOperand(irInstruction->src1)));
    asmInstructions.push_back(ASMInstruction::createRet());
    break;
  }
  case IROpType::ADD:{
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst),IRValueToOperand(irInstruction->src1)));
    asmInstructions.push_back(ASMInstruction::createBinary(BinaryOpType::ADD, IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src2)));
    break;
  }
  case IROpType::SUBTRACT:{
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst),IRValueToOperand(irInstruction->src1)));
    asmInstructions.push_back(ASMInstruction::createBinary(BinaryOpType::SUB, IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src2)));
    break;
  }
  case IROpType::MULTIPLY:{
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst),IRValueToOperand(irInstruction->src1)));
    asmInstructions.push_back(ASMInstruction::createBinary(BinaryOpType::MULT, IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src2)));
    break;
  }
  case IROpType::DIVIDE:{
    asmInstructions.push_back(ASMInstruction::createMov(Reg::createRegister(RegisterType::AX),IRValueToOperand(irInstruction->src1)));
    asmInstructions.push_back(ASMInstruction::createCDQ());

    asmInstructions.push_back(ASMInstruction::createIDiv(IRValueToOperand(irInstruction->src2)));
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst),Reg::createRegister(RegisterType::AX)));
    break;
  }
  case IROpType::REMAINDER:{
    asmInstructions.push_back(ASMInstruction::createMov(Reg::createRegister(RegisterType::AX),IRValueToOperand(irInstruction->src1)));
    asmInstructions.push_back(ASMInstruction::createCDQ());

    asmInstructions.push_back(ASMInstruction::createIDiv(IRValueToOperand(irInstruction->src2)));
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst),Reg::createRegister(RegisterType::DX)));
    break;
  }
  case IROpType::AND:{
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst),IRValueToOperand(irInstruction->src1)));
    asmInstructions.push_back(ASMInstruction::createBinary(BinaryOpType::AND, IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src2)));
    break;
  }
  case IROpType::OR:{
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst),IRValueToOperand(irInstruction->src1)));
    asmInstructions.push_back(ASMInstruction::createBinary(BinaryOpType::OR, IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src2)));
    break;
  }
  case IROpType::XOR:{
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst),IRValueToOperand(irInstruction->src1)));
    asmInstructions.push_back(ASMInstruction::createBinary(BinaryOpType::XOR, IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src2)));
    break;
  }
  case IROpType::LEFT_SHIFT:{
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst),IRValueToOperand(irInstruction->src1)));
    asmInstructions.push_back(ASMInstruction::createBinary(BinaryOpType::LEFT_SHIFT, IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src2)));
    break;
  }
  case IROpType::RIGHT_SHIFT:{
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst),IRValueToOperand(irInstruction->src1)));
    asmInstructions.push_back(ASMInstruction::createBinary(BinaryOpType::RIGHT_SHIFT, IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src2)));
    break;
  }
  case IROpType::EQUAL:
  case IROpType::NOT_EQUAL:
  case IROpType::GREATER_THAN:
  case IROpType::GREATER_EQUAL:
  case IROpType::LESS_THAN:
  case IROpType::LESS_EQUAL:{
    asmInstructions.push_back(ASMInstruction::createCmp(IRValueToOperand(irInstruction->src1),IRValueToOperand(irInstruction->src2)));
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst), Immediate::createImmediate(0)));
    asmInstructions.push_back(ASMInstruction::createSetCC(binOptoConditionCode(irInstruction->opType), IRValueToOperand(irInstruction->dst)));
    break;
  }
  case IROpType::LABEL:{
    asmInstructions.push_back(ASMInstruction::createLabel(irInstruction->label));
    break;
  }
  case IROpType::JUMP:{
    asmInstructions.push_back(ASMInstruction::createJmp(irInstruction->label));
    break;
  }
  case IROpType::JUMP_IF_ZERO:{
    asmInstructions.push_back(ASMInstruction::createCmp(IRValueToOperand(irInstruction->src1), Immediate::createImmediate(0)));
    asmInstructions.push_back(ASMInstruction::createJmpCC(ConditionCode::E, irInstruction->label));
    break;
  }
  case IROpType::JUMP_IF_NOT_ZERO:{
    asmInstructions.push_back(ASMInstruction::createCmp(IRValueToOperand(irInstruction->src1), Immediate::createImmediate(0)));
    asmInstructions.push_back(ASMInstruction::createJmpCC(ConditionCode::NE, irInstruction->label));
    break;
  }
  case IROpType::NOT:{
    asmInstructions.push_back(ASMInstruction::createCmp(IRValueToOperand(irInstruction->src1), Immediate::createImmediate(0)));
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst), Immediate::createImmediate(0)));
    asmInstructions.push_back(ASMInstruction::createSetCC(ConditionCode::E, IRValueToOperand(irInstruction->dst)));
    break;
  }
  case IROpType::COPY:{
    asmInstructions.push_back(ASMInstruction::createMov(IRValueToOperand(irInstruction->dst), IRValueToOperand(irInstruction->src1)));
    break;
  }
  default:
    break;
  }
  return asmInstructions;
}

OperandPtr Codegen::IRValueToOperand(const IRValuePtr &irValue){
  if (!irValue) return nullptr;
  switch (irValue->type) {
  case IRValueType::CONSTANT :{
    return Immediate::createImmediate(std::get<int>(irValue->value));
  }
  case IRValueType::VARIABLE:{
    return std::make_shared<Pseudo>(irValue->name);
  }
  case IRValueType::TEMPORARY:{
    return std::make_shared<Pseudo>(irValue->name);
  }
  case IRValueType::ARGS:{

  }
  default:
    return nullptr;
  }
}

int Codegen::replacePseudoRegisters(ASMBasePtr ast) {
  static int offset = 0;
  // Implementation for replacing pseudo registers with actual registers
  if (auto program = std::dynamic_pointer_cast<ASMProgram>(ast)) {
    for (auto &function : program->functions) {
      replacePseudoRegisters(function);
    }
  } else if (auto function = std::dynamic_pointer_cast<ASMFunction>(ast)) {
    offset_table.clear();
    offset = 0;
    for (auto &instruction : function->instructions) {
      // Replace dst
      if (instruction->dst) {
        if (auto pseudo = std::dynamic_pointer_cast<Pseudo>(instruction->dst)) {
          if (offset_table.find(pseudo->name) == offset_table.end()) {
            offset += 4;
            offset_table[pseudo->name] = offset; // Negative offset from RBP
          }
          instruction->dst = std::make_shared<Stack>(offset_table[pseudo->name]);
        }
      }
      
      // Replace src1
      if (instruction->src1) {
        if (auto pseudo = std::dynamic_pointer_cast<Pseudo>(instruction->src1)) {
          if (offset_table.find(pseudo->name) == offset_table.end()) {
            offset += 4;
            offset_table[pseudo->name] = offset; // Negative offset from RBP
          }
          instruction->src1 = std::make_shared<Stack>(offset_table[pseudo->name]);
        }
      }
      
      // Replace src2
      if (instruction->src2) {
        if (auto pseudo = std::dynamic_pointer_cast<Pseudo>(instruction->src2)) {
          if (offset_table.find(pseudo->name) == offset_table.end()) {
            offset += 4;
            offset_table[pseudo->name] = offset; // Negative offset from RBP
          }
          instruction->src2 = std::make_shared<Stack>(offset_table[pseudo->name]);
        }
      }
    }
  }
  return offset;
}

void Codegen::finalPass(ASMBasePtr ast) {
  // Implementation for final optimizations and adjustments
  if (auto program = std::dynamic_pointer_cast<ASMProgram>(ast)) {
    for (auto &function : program->functions) {
      finalPass(function);
    }
  } else if (auto function = std::dynamic_pointer_cast<ASMFunction>(ast)) {
    auto newinstructions = std::vector<ASMInstructionPtr>{};
    for (auto &instruction : function->instructions) {
      switch (instruction->opType) {
        case ASMOpType::MOV:{
          if(instruction->dst && instruction->src1){
          if(auto dstStack = dynamic_cast<Stack*>(instruction->dst.get())){
            if(auto srcImm = dynamic_cast<Stack*>(instruction->src1.get())){
              // mov can't contain both dst and src as address
              auto inst = std::make_shared<ASMInstruction>();
              inst->opType = ASMOpType::MOV;
              inst->dst = Reg::createRegister(RegisterType::R10);
              inst->src1 = std::make_shared<Stack>(srcImm->offset);
              newinstructions.push_back(inst);
              instruction->src1 = Reg::createRegister(RegisterType::R10);
            }
          }
        }
          newinstructions.push_back(instruction);
          break;
        }
        case ASMOpType::BINARY:{
          if(instruction->binaryOpType == BinaryOpType::MULT){
            if(dynamic_pointer_cast<Stack>(instruction->dst)){
              // move address to R11
              auto inst = std::make_shared<ASMInstruction>();
              inst->opType = ASMOpType::MOV;
              inst->dst = Reg::createRegister(RegisterType::R11);
              inst->src1 = instruction->dst;
              newinstructions.push_back(inst);
              instruction->dst = Reg::createRegister(RegisterType::R11);
              newinstructions.push_back(instruction);
              newinstructions.push_back(ASMInstruction::createMov(inst->src1, Reg::createRegister(RegisterType::R11)));
            } else{
              newinstructions.push_back(instruction);
            }
          } else if(instruction->binaryOpType == BinaryOpType::LEFT_SHIFT || instruction->binaryOpType == BinaryOpType::RIGHT_SHIFT){
            if(!dynamic_pointer_cast<Immediate>(instruction->src2)){
              // move src2 to ECX
              newinstructions.push_back(ASMInstruction::createMov(Reg::createRegister(RegisterType::ECX), instruction->src2));
              instruction->src2 = Reg::createRegister(RegisterType::ECX);
            }
            newinstructions.push_back(instruction);
          }
          else{
            if(auto dstStack = dynamic_cast<Stack*>(instruction->dst.get())){
              if(auto srcImm = dynamic_cast<Stack*>(instruction->src2.get())){
                // mov can't contain both dst and src as address
                newinstructions.push_back(ASMInstruction::createMov(Reg::createRegister(RegisterType::R10), instruction->src2));
                instruction->src2 = Reg::createRegister(RegisterType::R10);
              }
            }
            newinstructions.push_back(instruction);
          }
          break;
        }
        case ASMOpType::IDIV:{
          if(dynamic_pointer_cast<Immediate>(instruction->src1)){
            // move immediate to R10
            newinstructions.push_back(ASMInstruction::createMov(Reg::createRegister(RegisterType::R10), instruction->src1));
            instruction->src1 = Reg::createRegister(RegisterType::R10);
          }
          newinstructions.push_back(instruction);
          break;
        }
        case ASMOpType::CMP:{
          if(auto src1Stack = dynamic_cast<Stack*>(instruction->src1.get())){
            if(auto src2Stack = dynamic_cast<Stack*>(instruction->src2.get())){
              // cmp can't contain both src1 and src2 as address
              newinstructions.push_back(ASMInstruction::createMov(Reg::createRegister(RegisterType::R10), instruction->src2));
              instruction->src2 = Reg::createRegister(RegisterType::R10);
            }
          }
          // checking if the second operand is an immediate value
          if(auto src1Imm = dynamic_cast<Immediate*>(instruction->src1.get())){
            newinstructions.push_back(ASMInstruction::createMov(Reg::createRegister(RegisterType::R11), instruction->src1));
            instruction->src1 = Reg::createRegister(RegisterType::R11);
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