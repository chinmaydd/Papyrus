#include "IRConstructor.h"

using namespace papyrus;

#define NOTFOUND -1

using IRC  = IRConstructor;
using ASTC = ASTConstructor;

IRConstructor::IRConstructor(ASTC& astconst) :
    astconst_(astconst),
    value_counter_(0),
    value_map_(new std::unordered_map<ValueIndex, Value*>()) {

    DeclareIntrinsicFunctions();
}

bool IRConstructor::IsIntrinsic(const std::string& func_name) const {
    return (func_name == "InputNum" ||
            func_name == "OutputNum" ||
            func_name == "OutputNewLine");
}

void IRConstructor::DeclareIntrinsicFunctions() {
    functions_["InputNum"] = nullptr;
    functions_["OutputNum"] = nullptr;
    functions_["OutputNewLine"] = nullptr;
}

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
            return T::INS_BNE;
        case RELOP_NEQ:
            return T::INS_BEQ;
        case RELOP_LT:
            return T::INS_BGE;
        case RELOP_LTE:
            return T::INS_BGT;
        case RELOP_GT:
            return T::INS_BLE;
        case RELOP_GTE:
            return T::INS_BLT;
        default:
            return T::INS_ANY;
    }
}

void IRC::DeclareFunction(const std::string& func_name) {
    if (IsExistFunction(func_name)) {
        LOG(ERROR) << "[IR] Multiple definitions of function " + func_name + " found.";
        exit(1);
    }

    functions_[func_name] = nullptr;
}

void IRC::AddFunction(const std::string& func_name, Function *func) {
    functions_[func_name] = func;
}

bool IRC::IsExistFunction(const std::string& func_name) const {
    return functions_.find(func_name) != functions_.end();
}

Function* IRC::GetFunction(const std::string& func_name) {
    return functions_.at(func_name);
}

std::vector<BI> IRC::PostOrderCFG(const std::string& func_name) const {
    return functions_.at(func_name)->PostOrderCFG();
}

const std::unordered_map<std::string, Function*>& IRC::Functions() const {
    return functions_;
}

const std::map<std::string, Variable*>& IRC::Globals() const {
    return global_variable_map_;
}

Variable* IRC::GetGlobal(const std::string& var_name) const {
    return global_variable_map_.at(var_name);
}

void IRC::AddGlobal(const std::string& var_name, Variable* var) {
    if (IsVariableGlobal(var_name)) {
        LOG(ERROR) << "[IR] Attempt to redefine global variable " + var_name + " found.";
        exit(1);
    }

    global_variable_map_[var_name] = var;
}

void IRC::RemoveGlobal(const std::string& var_name) {
    global_variable_map_.erase(var_name);
}

int IRC::GlobalOffset(const std::string& var_name) const {
    return global_variable_map_.at(var_name)->Offset();
}

bool IRC::IsVariableGlobal(const std::string& var_name) const {
    return global_variable_map_.find(var_name) != global_variable_map_.end();
}

VI IRC::GetLocationValue(const std::string& var_name) const {
    return GetGlobal(var_name)->GetLocationIdx();
}

void IRC::DeclareGlobalBase() {
    value_counter_++;

    global_base_ = new Value(V::VAL_GLOBALBASE);
    global_base_idx_ = value_counter_;

    value_map_->emplace(value_counter_, global_base_);
}

VI IRC::CreateValue(V vty) {
    value_counter_++;

    Value* val = new Value(vty);
    value_map_->emplace(value_counter_, val);
    
    return value_counter_;
}

Value* IRC::GetValue(VI value_idx) const {
    return value_map_->at(value_idx);
}

void IRC::BuildIR() {
    const ComputationNode* root = astconst_.GetRoot();
    root->GenerateIR(*this);
}
