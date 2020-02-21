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

void Function::AddVariable(std::string var_name, Variable* var) {
    variable_map_[var_name] = var;
}

ValueIndex Function::CreateConstant(int constant) {
    value_counter_++;

    ConstantValue* c = new ConstantValue(constant);
    local_value_map_[value_counter_] = c;

    return value_counter_;

}

int Function::GetOffsetForVariable(const std::string& var_name) const {
    if (variable_map_.find(var_name) != variable_map_.end()) {
        return variable_map_.at(var_name)->GetOffset();
    } else {
        // search global context
        return -1;
    }
}
