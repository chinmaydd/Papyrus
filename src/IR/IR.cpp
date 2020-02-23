#include "IR.h"

using namespace papyrus;

ConstantValue::ConstantValue(int val) :
    val_(val) {
    vty = ValueType::VAL_CONST;
}

Function::Function(const std::string& func_name):
    func_name_(func_name) {}

void Function::AddVariable(const std::string& var_name, Variable* var) {
    variable_map_[var_name] = var;
}

void Function::WriteVariable(const std::string& var_name, BBIndex bb_idx, ValueIndex val_idx) {
    local_defs_[var_name][bb_idx] = val_idx;
}

void Function::SetEntry(BBIndex entry_idx) {
    entry_idx_ = entry_idx;
}

void Function::SetExit(BBIndex exit_idx) {
    exit_idx_ = exit_idx;
}

int Function::GetOffsetForVariable(const std::string& var_name) const {
    if (variable_map_.find(var_name) != variable_map_.end()) {
        return variable_map_.at(var_name)->GetOffset();
    } else {
        return -1;
    }
}

Instruction::Instruction(InstructionType insty, BBIndex containing_bb, InstructionIndex ins_idx) :
    ins_type_(insty),
    containing_bb_(containing_bb),
    ins_idx_(ins_idx) {}

BasicBlock::BasicBlock(BBIndex idx) :
    idx_(idx) {}

void BasicBlock::AddPredecessor(BBIndex pred_idx) {
    predecessors_.push_back(pred_idx);
}

void BasicBlock::AddSuccessor(BBIndex succ_idx) {
    successors_.push_back(succ_idx);
}
