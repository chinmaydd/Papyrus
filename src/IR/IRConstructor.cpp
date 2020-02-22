#include "IRConstructor.h"

using namespace papyrus;

using IRC  = IRConstructor;
using ASTC = ASTConstructor;

IRConstructor::IRConstructor(ASTC& astconst) :
    astconst_(astconst),
    value_counter_(0),
    instruction_counter_(0),
    bb_counter_(0) {}

void IRC::AddInstruction(InstTy insty) {}
void IRC::AddInstruction(InstTy insty, ValueIndex arg_1) {}
void IRC::AddInstruction(InstTy insty, ValueIndex arg_1, ValueIndex arg_2) {
    switch(insty) {
        case InstTy::INS_STORE: {
        }
        case InstTy::INS_ADDA: {
        }
    }
}

void IRC::AddFunction(const std::string& func_name, Function *func) {
    functions_[func_name] = func;
}

void IRC::WriteVariable(const std::string& var_name, ValueIndex value) {
    Function *func = GetCurrentFunction();
}

void IRC::AddGlobalVariable(const std::string& var_name, Variable* var) {
    global_variable_map_[var_name] = var;
}

int IRC::GetOffsetForGlobalVariable(const std::string& var_name) const {
    if (CheckIfGlobal(var_name)) {
        return global_variable_map_.at(var_name)->GetOffset();
    }

    return -1;
}

bool IRC::CheckIfGlobal(const std::string& var_name) const {
    return global_variable_map_.find(var_name) != global_variable_map_.end();
}

ValueIndex IRC::AddValue(Value* val) {
    value_counter_++;

    value_map_[value_counter_] = val;

    return value_counter_;
}

ValueIndex IRC::CreateConstant(int val) {
    ConstantValue* c = new ConstantValue(val);

    value_counter_++;
    value_map_[value_counter_] = c;

    return value_counter_;
}

BBIndex IRC::CreateBB(std::string func_name) {
    bb_counter_++;
    BasicBlock *bb = new BasicBlock(bb_counter_);

    basic_block_map_[bb_counter_] = bb;

    return bb_counter_;
}

BBIndex IRC::CreateBB(std::string func_name, BBIndex pred_idx) {
    BBIndex bb_idx = CreateBB(func_name);
    AddBBPredecessor(bb_idx, pred_idx);

    return bb_idx;
}

void IRC::AddBBPredecessor(BBIndex current_idx, BBIndex pred_idx) {
    basic_block_map_[current_idx]->AddPredecessor(pred_idx);
}

void IRC::construct() {
    const ComputationNode* root = astconst_.GetRoot();
    root->GenerateIR(this);
}
