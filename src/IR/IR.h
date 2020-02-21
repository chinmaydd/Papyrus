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

class Value {
public:
    enum ValueType {
        VAL_NONE,
        VAL_CONST,
        VAL_ANY,
    };
    const ValueType GetValueType() const { return vty; }
protected:
    ValueType vty;
    std::vector<InstructionIndex> uses_;
};

class ConstantValue : public Value {
public:
    ConstantValue(int);
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
        INS_CALL,
        INS_ADDA,
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

    ValueIndex AddValue(Value*);
    ValueIndex GetLocalBase() const { return local_base_value_; }

    void AddVariable(std::string, Variable*);
    ValueIndex CreateConstant(int);
    int GetOffsetForVariable(const std::string&) const;

private:
    std::string func_name_;
    Value* local_base_;
    ValueIndex local_base_value_;

    ValueIndex value_counter_;
    std::unordered_map<std::string, Variable*> variable_map_;
    std::unordered_map<ValueIndex, Value*> local_value_map_;
    std::unordered_map<std::string, std::unordered_map<BBIndex, ValueIndex> > local_defs_;
    
    BBIndex bb_counter_;
    std::unordered_map<BBIndex, BasicBlock*> bb_map_;
};

} // namespace papyrus

#endif /* PAPYRUS_IR_H */
