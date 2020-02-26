#include "IR.h"

using namespace papyrus;

#define NOTFOUND -1

Value::Value(ValueType vty) :
    vty_(vty) {}

void Value::RemoveUse(InstructionIndex ins_idx) {
    uses_.erase(std::remove(uses_.begin(), uses_.end(), ins_idx), uses_.end());
}

Function::Function(const std::string& func_name, ValueIndex value_counter, std::unordered_map<ValueIndex, Value*>* value_map):
    func_name_(func_name),
    value_counter_(value_counter),
    bb_counter_(0),
    instruction_counter_(0),
    value_map_(value_map) {
        SetLocalBase(CreateValue(V::VAL_LOCALBASE));
        SetCurrentBB(CreateBB());
}

const Variable* Function::GetVariable(const std::string& var_name) const { 
    return variable_map_.at(var_name); 
}

void Function::AddVariable(const std::string& var_name, Variable* var) {
    variable_map_[var_name] = var;
}

bool Function::IsVariableLocal(const std::string& var_name) const {
    return variable_map_.find(var_name) != variable_map_.end();
}

int Function::GetOffset(const std::string& var_name) const {
    return variable_map_.at(var_name)->Offset();
}

ValueIndex Function::CreateConstant(int val) {
    Value* v = new Value(V::VAL_CONST);
    v->SetConstant(val); 

    value_counter_++;
    value_map_->emplace(value_counter_, v);

    return value_counter_;
}

ValueIndex Function::CreateValue(V vty) {
    Value* val = new Value(vty);

    value_counter_++;
    value_map_->emplace(value_counter_, val);

    return value_counter_;
}

void Function::AddUsage(ValueIndex val_idx, InstructionIndex ins_idx) {
    value_map_->at(val_idx)->AddUsage(ins_idx);
}

Value* Function::GetValue(ValueIndex val_idx) const {
    return value_map_->at(val_idx);
}

void Function::SetValueType(ValueIndex val_idx, V val_type) {
    Value* val = GetValue(val_idx);
    val->SetType(val_type);
}

ValueIndex Function::TryRemoveTrivialPhi(InstructionIndex phi_ins) {
    ValueIndex same = NOTFOUND;
    Instruction* ins = GetInstruction(phi_ins);
    ValueIndex result = ins->Result();

    for (auto op: ins->Operands()) {
        if (op == same || op == result) {
            continue;
        }
        if (same != NOTFOUND) {
            return phi_ins;
        }
        same = op;
    }

    if (same == NOTFOUND) {
        same = CreateValue(V::VAL_ANY);
    }

    Value* phi_val = GetValue(result);
    phi_val->RemoveUse(phi_ins);

    for (auto use_idx: phi_val->GetUsers()) {
        TryRemoveTrivialPhi(use_idx);
    }

    return same;
}

bool Function::IsPhi(InstructionIndex ins_idx) const {
    return instruction_map_.at(ins_idx)->IsPhi();
}

ValueIndex Function::AddPhiOperands(const std::string& var_name, InstructionIndex phi_ins) {
    BasicBlock* bb = GetBB(GetBBForInstruction(phi_ins));
    Instruction* ins = GetInstruction(phi_ins);

    for (auto pred: bb->Predecessors()) {
        ins->AddOperand(ReadVariable(var_name, pred));
    }

    return TryRemoveTrivialPhi(phi_ins);
}

ValueIndex Function::ReadVariableRecursive(const std::string& var_name, BBIndex bb_idx) {
    ValueIndex result = NOTFOUND;
    BasicBlock* bb = GetBB(bb_idx);

    if (!bb->IsSealed()) {
        SetCurrentBB(bb_idx);
        result = MakePhi();
        incomplete_phis_[var_name][bb_idx] = result;
    } else if (bb->Predecessors().size() == 1) {
        result = ReadVariable(var_name, bb->Predecessors()[0]);
    } else {
        SetCurrentBB(bb_idx);
        InstructionIndex phi_ins = MakePhi();
        WriteVariable(var_name, bb_idx, phi_ins);
        result = AddPhiOperands(var_name, phi_ins);
    }

    WriteVariable(var_name, bb_idx, result);
    return result;
}

ValueIndex Function::ReadVariable(const std::string& var_name, BBIndex bb_idx) {
    if (local_defs_[var_name].find(bb_idx) != local_defs_[var_name].end()) {
        return local_defs_[var_name][bb_idx];
    } else {
        return ReadVariableRecursive(var_name, bb_idx);
    }
}

void Function::WriteVariable(const std::string& var_name, BBIndex bb_idx, ValueIndex val_idx) {
    local_defs_[var_name][bb_idx] = val_idx;
}

void Function::WriteVariable(const std::string& var_name, ValueIndex val_idx) {
    WriteVariable(var_name, CurrentBBIdx(), val_idx);
}

BBIndex Function::CreateBB() {
    bb_counter_++;

    BasicBlock* bb = new BasicBlock(bb_counter_);
    basic_block_map_[bb_counter_] = bb;

    return bb_counter_;
}

void Function::AddBBEdge(BBIndex pred, BBIndex succ) {
    AddBBPredecessor(succ, pred);
    AddBBSuccessor(pred, succ);
}

const std::map<BBIndex, BasicBlock*> Function::BasicBlocks() const {
    return basic_block_map_;
}

BasicBlock* Function::GetBB(BBIndex bb_idx) const {
    return basic_block_map_.at(bb_idx);
}

BasicBlock* Function::CurrentBB() const { 
    return GetBB(CurrentBBIdx());
}

void Function::AddBBPredecessor(BBIndex source, BBIndex pred) {
    basic_block_map_[source]->AddPredecessor(pred);
}

void Function::AddBBSuccessor(BBIndex source, BBIndex pred) {
    basic_block_map_[source]->AddSuccessor(pred);
}

InstructionIndex Function::MakePhi() {
    instruction_counter_++;

    Instruction* inst = new Instruction(T::INS_PHI,
                                       CurrentBBIdx(),
                                       instruction_counter_);

    instruction_map_[instruction_counter_] = inst;
    instruction_order_.push_front(instruction_counter_);

    // XXX: Do we really need VAL_ANY here?
    ValueIndex result = CreateValue(V::VAL_ANY);
    inst->SetResult(result);

    CurrentBB()->AddInstruction(instruction_counter_, inst);

    return instruction_counter_;
}

BBIndex Function::GetBBForInstruction(InstructionIndex ins_idx) {
    return instruction_map_[ins_idx]->ContainingBB();
}

ValueIndex Function::MakeInstruction(T insty) {
    instruction_counter_++;
    Instruction* inst = new Instruction(insty,
                                        CurrentBBIdx(),
                                        instruction_counter_);

    instruction_map_[instruction_counter_] = inst;
    instruction_order_.push_back(instruction_counter_);

    ValueIndex result = CreateValue(V::VAL_ANY);
    inst->SetResult(result);

    CurrentBB()->AddInstruction(instruction_counter_, inst);

    return result;
}

ValueIndex Function::MakeInstruction(T insty, ValueIndex arg_1) {
    ValueIndex result = MakeInstruction(insty);

    CurrentInstruction()->AddOperand(arg_1);
    AddUsage(arg_1, instruction_counter_);

    return result;
}

ValueIndex Function::MakeInstruction(T insty, ValueIndex arg_1, ValueIndex arg_2) {
    ValueIndex result = MakeInstruction(insty);

    CurrentInstruction()->AddOperand(arg_1);
    AddUsage(arg_1, instruction_counter_);

    CurrentInstruction()->AddOperand(arg_2);
    AddUsage(arg_2, instruction_counter_);

    return result;
}

Instruction* Function::GetInstruction(InstructionIndex ins_idx) const {
    return instruction_map_.at(ins_idx);
}

Instruction* Function::CurrentInstruction() const {
    return GetInstruction(instruction_counter_);
}

Instruction::Instruction(T insty, BBIndex containing_bb, InstructionIndex ins_idx) :
    ins_type_(insty),
    containing_bb_(containing_bb),
    ins_idx_(ins_idx) {}

BasicBlock::BasicBlock(BBIndex idx) :
    idx_(idx),
    is_sealed_(false) {}

void BasicBlock::AddPredecessor(BBIndex pred_idx) {
    predecessors_.push_back(pred_idx);
}

void BasicBlock::AddSuccessor(BBIndex succ_idx) {
    successors_.push_back(succ_idx);
}

void BasicBlock::AddInstruction(InstructionIndex idx, Instruction* inst) {
    instructions_[idx] = inst;
}

const std::vector<BBIndex> BasicBlock::Predecessors() const {
    return predecessors_;
}
const std::vector<BBIndex> BasicBlock::Successors() const {
    return successors_;
}
