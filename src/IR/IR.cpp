#include "IR.h"

using namespace papyrus;

ConstantValue::ConstantValue(int val) :
    val_(val) {
    vty = ValueType::VAL_CONST;
}

Function::Function(const std::string& func_name):
    func_name_(func_name) {}

ValueIndex Function::AddValue(Value* val) {
    value_counter_++;
    local_value_map_[value_counter_] = val;
    return value_counter_;
}

IRCtxInfo::IRCtxInfo() {}

void IRCtxInfo::AddFunction(const std::string& func_name, Function *func) {
    functions_[func_name] = func;
}

void IRCtxInfo::AddInstruction(InstTy insty) {}
void IRCtxInfo::AddInstruction(InstTy insty, ValueIndex arg_1) {}

void IRCtxInfo::AddInstruction(InstTy insty, ValueIndex arg_1, ValueIndex arg_2) {
}
