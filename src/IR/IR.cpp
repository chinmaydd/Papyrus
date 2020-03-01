#include "IR.h"

using namespace papyrus;

#define NOTFOUND -1

Value::Value(ValueType vty) :
    vty_(vty) {}

Function::Function(const std::string& func_name, VI value_counter, std::unordered_map<VI, Value*>* value_map):
    func_name_(func_name),
    value_counter_(value_counter),
    bb_counter_(0),
    current_bb_(0),
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

VI Function::CreateConstant(int val) {
    Value* v = new Value(V::VAL_CONST);
    v->SetConstant(val); 

    value_counter_++;
    value_map_->emplace(value_counter_, v);

    return value_counter_;
}

VI Function::CreateValue(V vty) {
    Value* val = new Value(vty);

    value_counter_++;
    value_map_->emplace(value_counter_, val);

    return value_counter_;
}

void Function::AddUsage(VI val_idx, II ins_idx) {
    value_map_->at(val_idx)->AddUsage(ins_idx);
}

Value* Function::GetValue(VI val_idx) const {
    return value_map_->at(val_idx);
}

void Function::SetValueType(VI val_idx, V val_type) {
    Value* val = GetValue(val_idx);
    val->SetType(val_type);
}

BI Function::CreateBB() {
    bb_counter_++;

    BasicBlock* bb = new BasicBlock(bb_counter_);
    basic_block_map_[bb_counter_] = bb;

    VI sv = CreateValue(V::VAL_BRANCH);
    GetValue(sv)->SetConstant(bb_counter_);
    bb->SetSelfValue(sv);

    return bb_counter_;
}

void Function::AddBBEdge(BI pred, BI succ) {
    AddBBPredecessor(succ, pred);
    AddBBSuccessor(pred, succ);
}

const std::unordered_map<BI, BasicBlock*> Function::BasicBlocks() const {
    return basic_block_map_;
}

BasicBlock* Function::GetBB(BI bb_idx) const {
    return basic_block_map_.at(bb_idx);
}

BasicBlock* Function::CurrentBB() const { 
    return GetBB(CurrentBBIdx());
}

void Function::AddBBPredecessor(BI source, BI pred) {
    basic_block_map_[source]->AddPredecessor(pred);
}

void Function::AddBBSuccessor(BI source, BI pred) {
    basic_block_map_[source]->AddSuccessor(pred);
}

bool Function::HasEndedBB(BI bb_idx) const {
    return GetBB(bb_idx)->HasEnded();
}

II Function::MakePhi() {
    instruction_counter_++;

    Instruction* inst = new Instruction(T::INS_PHI,
                                        CurrentBBIdx(),
                                        instruction_counter_);

    instruction_map_[instruction_counter_] = inst;
    instruction_order_.push_front(instruction_counter_);

    VI result = CreateValue(V::VAL_ANY);
    inst->SetResult(result);

    CurrentBB()->AddInstruction(instruction_counter_, inst);

    return instruction_counter_;
}

BI Function::GetBBForInstruction(II ins_idx) {
    return instruction_map_[ins_idx]->ContainingBB();
}

VI Function::MakeInstruction(T insty) {
    instruction_counter_++;
    Instruction* inst = new Instruction(insty,
                                        CurrentBBIdx(),
                                        instruction_counter_);

    instruction_map_[instruction_counter_] = inst;
    instruction_order_.push_back(instruction_counter_);

    V vty;
    
    VI result = CreateValue(vty);
    inst->SetResult(result);

    CurrentBB()->AddInstruction(instruction_counter_, inst);

    return result;
}

VI Function::MakeInstruction(T insty, VI arg_1) {
    VI result = MakeInstruction(insty);

    CurrentInstruction()->AddOperand(arg_1);

    AddUsage(arg_1, instruction_counter_);

    return result;
}

VI Function::MakeInstruction(T insty, VI arg_1, VI arg_2) {
    VI result = MakeInstruction(insty);

    CurrentInstruction()->AddOperand(arg_1);
    CurrentInstruction()->AddOperand(arg_2);

    AddUsage(arg_1, instruction_counter_);
    AddUsage(arg_2, instruction_counter_);

    return result;
}

Instruction* Function::GetInstruction(II ins_idx) const {
    return instruction_map_.at(ins_idx);
}

Instruction* Function::CurrentInstruction() const {
    return GetInstruction(instruction_counter_);
}

bool Function::IsActive(II ins_idx) const {
    return instruction_map_.at(ins_idx)->IsActive();
}

bool Function::IsRelational(T insty) const {
    return (insty == T::INS_BEQ ||
            insty == T::INS_BNE ||
            insty == T::INS_BLT ||
            insty == T::INS_BLE ||
            insty == T::INS_BGE ||
            insty == T::INS_BRA);
}

Instruction::Instruction(T insty, BI containing_bb, II ins_idx) :
    ins_type_(insty),
    containing_bb_(containing_bb),
    ins_idx_(ins_idx),
    is_active_(true) {}

BasicBlock::BasicBlock(BI idx) :
    idx_(idx),
    is_sealed_(false),
    is_ended_(false) {}

void BasicBlock::AddPredecessor(BI pred_idx) {
    predecessors_.push_back(pred_idx);
}

void BasicBlock::AddSuccessor(BI succ_idx) {
    successors_.push_back(succ_idx);
}

void BasicBlock::AddInstruction(II idx, Instruction* inst) {
    instructions_[idx] = inst;
    
    if (inst->IsPhi()) {
        instruction_order_.push_front(idx);
    } else {
        instruction_order_.push_back(idx);
    }
}

const std::vector<BI> BasicBlock::Predecessors() const {
    return predecessors_;
}

const std::vector<BI> BasicBlock::Successors() const {
    return successors_;
}

bool BasicBlock::HasActiveInstructions() const {
    Instruction* ins;
    for (auto ins_pair: instructions_) {
        ins = ins_pair.second;
        if (ins->IsActive()) {
            return true;
        }
    }

    return false;
}

VI Function::ResultForInstruction(II ins_idx) const {
    return instruction_map_.at(ins_idx)->Result();
}

