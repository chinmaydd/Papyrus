#include "IRConstructor.h"

using namespace papyrus;

#define NOTFOUND -1

using IRC  = IRConstructor;
using ASTC = ASTConstructor;

IRConstructor::IRConstructor(ASTC& astconst) :
    astconst_(astconst),
    value_counter_(0),
    instruction_counter_(0),
    bb_counter_(0) {}

T IRC::ConvertInstruction(ArithmeticOperator op) {
    switch(op) {
        case BINOP_MUL:
            return T::INS_MUL;
        case BINOP_DIV:
            return T::INS_DIV;
        case BINOP_ADD:
            return T::INS_ADD;
        case BINOP_SUB:
            return T::INS_SUB;
        default:
            return T::INS_ANY;
    }
}

std::string IRC::GetInstructionString(T insty) {
    switch(insty) {
        case T::INS_ADDA:
            return "adda";
        case T::INS_STORE:
            return "store";
        case T::INS_MUL:
            return "mul";
        case T::INS_ADD:
            return "add";
        case T::INS_SUB:
            return "sub";
        default:
            return "NF";
    }
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

ValueIndex IRC::MakeInstruction(T insty) {
    instruction_counter_++;
    Instruction* inst = new Instruction(insty,
                                        bb_counter_,
                                        instruction_counter_);

    instruction_map_[instruction_counter_] = inst;

    ValueIndex result = CreateValue(V::VAL_ANY);
    inst->SetResult(result);

    return result;
}

ValueIndex IRC::MakeInstruction(T insty, ValueIndex arg_1) {
    ValueIndex result = MakeInstruction(insty);

    GetCurrentInstruction()->AddArgument(arg_1);
    AddUsage(arg_1, instruction_counter_);

    return result;
}

ValueIndex IRC::MakeInstruction(T insty, ValueIndex arg_1, ValueIndex arg_2) {
    ValueIndex result = MakeInstruction(insty);

    GetCurrentInstruction()->AddArgument(arg_1);
    AddUsage(arg_1, instruction_counter_);

    GetCurrentInstruction()->AddArgument(arg_2);
    AddUsage(arg_2, instruction_counter_);

    return result;
}

void IRC::AddFunction(const std::string& func_name, Function *func) {
    functions_[func_name] = func;
}

void IRC::WriteVariable(const std::string& var_name, BBIndex bb_idx, ValueIndex val_idx) {
    if (current_function_->IsVariableLocal(var_name)) {
        current_function_->WriteVariable(var_name, bb_idx, val_idx);
    } else {
        global_defs_[var_name][bb_idx] = val_idx;
    }
}

void IRC::WriteVariable(const std::string& var_name, ValueIndex val_idx) {
    WriteVariable(var_name, bb_counter_, val_idx);
}

const Variable* IRC::GetGlobalVariable(const std::string& var_name) const {
    return global_variable_map_.at(var_name);
}

void IRC::AddGlobalVariable(const std::string& var_name, Variable* var) {
    global_variable_map_[var_name] = var;
}

int IRC::GetOffsetForGlobalVariable(const std::string& var_name) const {
    return global_variable_map_.at(var_name)->GetOffset();
}

bool IRC::IsVariableGlobal(const std::string& var_name) const {
    return global_variable_map_.find(var_name) != global_variable_map_.end();
}

bool IRC::IsVariableLocal(const std::string& var_name) const {
    return current_function_->IsVariableLocal(var_name);
}

ValueIndex IRC::CreateValue(V vty) {
    Value* val = new Value(vty);

    value_counter_++;
    value_map_[value_counter_] = val;

    return value_counter_;
}

ValueIndex IRC::CreateConstant(int val) {
    Value* v = new Value(V::VAL_CONST);
    v->SetConstant(val);

    value_counter_++;
    value_map_[value_counter_] = v;

    return value_counter_;
}

void IRC::AddUsage(ValueIndex val_idx, InstructionIndex ins_idx) {
    value_map_[val_idx]->AddUsage(ins_idx);
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

void IRC::DeclareGlobalBase() {
    global_base_idx_ = CreateValue(V::VAL_BASE);
}

void IRC::BuildIR() {
    const ComputationNode* root = astconst_.GetRoot();
    root->GenerateIR(this);
}


