#include <cassert>
#include "highlevel.h"

PrintHighLevelInstructionSequence::PrintHighLevelInstructionSequence(InstructionSequence *ins)
        : PrintInstructionSequence(ins) {
}

std::string PrintHighLevelInstructionSequence::get_opcode_name(int opcode) {
    switch (opcode) {
        case HINS_NOP:         return "nop";
        case HINS_LOAD_ICONST: return "ldci";
        case HINS_INT_ADD:     return "addi";
        case HINS_INT_SUB:     return "subi";
        case HINS_INT_MUL:     return "muli";
        case HINS_INT_DIV:     return "divi";
        case HINS_INT_MOD:     return "modi";
        case HINS_INT_NEGATE:  return "negi";
        case HINS_LOCALADDR:   return "localaddr";
        case HINS_LOAD_INT:    return "ldi";
        case HINS_STORE_INT:   return "sti";
        case HINS_READ_INT:    return "readi";
        case HINS_WRITE_INT:   return "writei";
        case HINS_JUMP:        return "jmp";
        case HINS_JE:          return "je";
        case HINS_JNE:         return "jne";
        case HINS_JLT:         return "jlt";
        case HINS_JLTE:        return "jlte";
        case HINS_JGT:         return "jgt";
        case HINS_JGTE:        return "jgte";
        case HINS_INT_COMPARE: return "cmpi";
        case HINS_LEA:         return "lea";
        case HINS_MOV:         return "mov";
        case HINS_CALL:        return "call";
        case HINS_FUNC_ENTER:  return "funcenter";
        case HINS_FUNC_LEAVE:  return "funcleave";

        default:
            assert(false);
            return "<invalid>";
    }
}

bool HighLevel::is_def(Instruction *ins) {
    int opcode = ins->get_opcode();
    switch(opcode) {
        case HINS_LOAD_ICONST:  return true;
        case HINS_INT_ADD:      return true;
        case HINS_INT_SUB:      return true;
        case HINS_INT_MUL:      return true;
        case HINS_INT_DIV:      return true;
        case HINS_INT_MOD:      return true;
        case HINS_INT_NEGATE:   return true;
        case HINS_LOCALADDR:    return true;
        case HINS_LOAD_INT:     return true;
        case HINS_READ_INT:     return true;
        default:                return false;
    }
}

bool HighLevel::is_use(Instruction *ins, unsigned i) {
    int opcode = ins->get_opcode();

    Operand op = ins->get_operand(i);
    bool op_is_vreg = op.has_base_reg();

    switch(opcode) {
        // TODO:
    }

    if (op_is_vreg) {
        return true;
    }

    return false;
}

int HighLevel::get_num_vregs(InstructionSequence *hins) {
    // map unique vregs
    std::map<long, Operand> vregs;

    const long num_ins = hins->get_length();
    for (int i = 0; i < num_ins; i++) {
        auto *hin = hins->get_instruction(i);
        const long num_operands = hin->get_num_operands();
        for (int j = 0; j < num_operands; j++) {
            Operand operand = hin->get_operand(j);
            if (operand.get_kind() == OPERAND_VREG || operand.get_kind() == OPERAND_VREG_MEMREF) {
                int base_reg = operand.get_base_reg();
                vregs[base_reg] = operand;
            }
        }
    }

    return vregs.size();
}

std::string PrintHighLevelInstructionSequence::get_mreg_name(int regnum) {
    // high level instructions should not use machine registers
    assert(false);
    return "<invalid>";
}

HighLevelControlFlowGraphBuilder::HighLevelControlFlowGraphBuilder(InstructionSequence *iseq)
        : ControlFlowGraphBuilder(iseq) {
}

HighLevelControlFlowGraphBuilder::~HighLevelControlFlowGraphBuilder() {
}

bool HighLevelControlFlowGraphBuilder::falls_through(Instruction *ins) {
    // only unconditional jump instructions don't fall through
    return ins->get_opcode() != HINS_JUMP;
}

HighLevelControlFlowGraphPrinter::HighLevelControlFlowGraphPrinter(ControlFlowGraph *cfg)
        : ControlFlowGraphPrinter(cfg) {
}

HighLevelControlFlowGraphPrinter::~HighLevelControlFlowGraphPrinter() {
}

void HighLevelControlFlowGraphPrinter::print_basic_block(BasicBlock *bb) {
    for (auto i = bb->cbegin(); i != bb->cend(); i++) {
        Instruction *ins = *i;
        std::string s = format_instruction(bb, ins);
        printf("\t%s\n", s.c_str());
    }
}

std::string HighLevelControlFlowGraphPrinter::format_instruction(BasicBlock *bb,
                                                                 Instruction *ins) {
    PrintHighLevelInstructionSequence p(bb);
    return p.format_instruction(ins);
}