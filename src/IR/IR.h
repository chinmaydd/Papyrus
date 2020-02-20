#ifndef PAPYRUS_IR_H
#define PAPYRUS_IR_H

#include "Papyrus/Logger/Logger.h"

#include "FrontEnd/ASTConstructor.h"
#include "Variable.h"

#include <vector>
#include <unordered_map>
#include <string>

using ValueIndex = int;
using BBIndex = int;
using InstructionIndex = int;

namespace papyrus {

class Function;
class BasicBlock;
class IRCtxInfo;

class Value {
protected:
    std::vector<InstructionIndex> uses_;
};

class ConstantValue : public Value {
private:
    int val_;
};

class NamedValue : public Value {
private:
    std::string name_;
};

class Instruction {
public:
    enum InstructionType {
        INS_STORE,
    };
private:
    InstructionType ins_type_;
    std::vector<ValueIndex> arguments_;
    ValueIndex result_;
    BBIndex containing_bb_;
};

class BasicBlock {
public:
    BasicBlock();

private:
    BBIndex idx_;
    std::unordered_map<InstructionIndex, Instruction*> instructions_;
};

class Function {
public:
    Function(const std::string&);
    const std::string& GetFunctionName() const { return func_name_; }
    
private:
    std::string func_name_;
    Value* local_base_;

    BBIndex bb_counter_;
    ValueIndex value_counter_;
    std::unordered_map<ValueIndex, Value*> local_value_map_;
    std::unordered_map<BBIndex, BasicBlock*> bb_map_;
    std::unordered_map<std::string, std::unordered_map<BBIndex, ValueIndex> > current_defs_;
};

class IRCtxInfo {
public:
    IRCtxInfo();

    void AddFunction(Function*);
    Function* GetCurrentFunction() { return current_function_; }
    void SetCurrentFunction(Function* f) { current_function_ = f; }
    void ClearCurrentFunction() { current_function_ = nullptr; }

private:
    Value* global_base_;

    int global_value_counter_;
    std::unordered_map<std::string, ValueIndex> global_values_;
    std::unordered_map<ValueIndex, Value*> global_value_map_;

    std::unordered_map<std::string, Function*> functions_;
    Function* current_function_;
};

} // namespace papyrus

#endif /* PAPYRUS_IR_H */
