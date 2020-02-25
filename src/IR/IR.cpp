#include "IR.h"

using namespace papyrus;

Value::Value(ValueType vty) :
    vty_(vty) {}

Function::Function(const std::string& func_name, ValueIndex value_counter, std::unordered_map<ValueIndex, Value*>* value_map):
    func_name_(func_name),
    value_counter_(value_counter),
    bb_counter_(0),
    value_map_(value_map) {
        SetLocalBase(CreateValue(V::VAL_LOCALBASE));
        SetCurrentBB(CreateBB());
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

void Function::AddBBPredecessor(BBIndex source, BBIndex pred) {
    basic_block_map_[source]->AddPredecessor(pred);
}

void Function::AddBBSuccessor(BBIndex source, BBIndex pred) {
    basic_block_map_[source]->AddSuccessor(pred);
}

// store x y : store y to memory address x
// case T::INS_STORE
// mul x y : multiplication
// case T::INS_ADD
// add x y : addition
// case T::INS_SUB
// sub x y : substitution
// case T::INS_DIV
// div x y : division
// case T::INS_MUL 
// adda x y : add two addresses x and y (used only with arrays)
// case T::INS_ADDA

ValueIndex Function::MakeInstruction(T insty) {
    instruction_counter_++;
    Instruction* inst = new Instruction(insty,
                                        bb_counter_,
                                        instruction_counter_);

    instruction_map_[instruction_counter_] = inst;

    ValueIndex result = CreateValue(V::VAL_ANY);
    inst->SetResult(result);

    CurrentBB()->AddInstruction(instruction_counter_, inst);

    return result;
}

ValueIndex Function::MakeInstruction(T insty, ValueIndex arg_1) {
    ValueIndex result = MakeInstruction(insty);

    CurrentInstruction()->AddArgument(arg_1);
    AddUsage(arg_1, instruction_counter_);

    return result;
}

ValueIndex Function::MakeInstruction(T insty, ValueIndex arg_1, ValueIndex arg_2) {
    ValueIndex result = MakeInstruction(insty);

    CurrentInstruction()->AddArgument(arg_1);
    AddUsage(arg_1, instruction_counter_);

    CurrentInstruction()->AddArgument(arg_2);
    AddUsage(arg_2, instruction_counter_);

    return result;
}

Instruction* Function::CurrentInstruction() const {
    return instruction_map_.at(instruction_counter_);
}

Instruction::Instruction(T insty, BBIndex containing_bb, InstructionIndex ins_idx) :
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

void BasicBlock::AddInstruction(InstructionIndex idx, Instruction* inst) {
    instructions_[idx] = inst;
}
