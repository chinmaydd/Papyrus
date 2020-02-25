#include "IRConstructor.h"

using namespace papyrus;

#define NOTFOUND -1

using IRC  = IRConstructor;
using ASTC = ASTConstructor;

IRConstructor::IRConstructor(ASTC& astconst) :
    astconst_(astconst),
    value_counter_(0),
    value_map_(new std::unordered_map<ValueIndex, Value*>()) {}

T IRC::ConvertOperation(ArithmeticOperator op) {
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

T IRC::ConvertOperation(RelationalOperator op) {
    switch(op) {
        case RELOP_EQ:
            return T::INS_EQ;
        case RELOP_NEQ:
            return T::INS_NEQ;
        case RELOP_LT:
            return T::INS_LT;
        case RELOP_LTE:
            return T::INS_LTE;
        case RELOP_GT:
            return T::INS_GT;
        case RELOP_GTE:
            return T::INS_GTE;
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

void IRC::AddFunction(const std::string& func_name, Function *func) {
    functions_[func_name] = func;
}

const Variable* IRC::GetGlobal(const std::string& var_name) const {
    return global_variable_map_.at(var_name);
}

void IRC::AddGlobal(const std::string& var_name, Variable* var) {
    global_variable_map_[var_name] = var;
}

int IRC::GlobalOffset(const std::string& var_name) const {
    return global_variable_map_.at(var_name)->Offset();
}

bool IRC::IsVariableGlobal(const std::string& var_name) const {
    return global_variable_map_.find(var_name) != global_variable_map_.end();
}

void IRC::DeclareGlobalBase() {
    value_counter_++;

    global_base_ = new Value(V::VAL_GLOBALBASE);
    global_base_idx_ = value_counter_;

    value_map_->emplace(value_counter_, global_base_);
}

void IRC::BuildIR() {
    const ComputationNode* root = astconst_.GetRoot();
    root->GenerateIR(*this);
}

