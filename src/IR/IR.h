#ifndef PAPYRUS_IR_H
#define PAPYRUS_IR_H

#include "Papyrus/Logger/Logger.h"
#include "Variable.h"

#include <vector>
#include <map>
#include <unordered_map>
#include <string>

using ValueIndex = int;
using BBIndex = int;
using InstructionIndex = int;

namespace papyrus {

class IRConstructor;

class Value {
public:
    enum ValueType {
        VAL_NONE,
        VAL_CONST,
        VAL_GLOBAL,
        VAL_LOCAL,
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
    std::string containing_function_;
};

class BasicBlock {
public:
    BasicBlock(BBIndex);
    void AddPredecessor(BBIndex);
    void AddSuccessor(BBIndex);

private:
    BBIndex idx_;
    std::map<InstructionIndex, Instruction*> instructions_;

    std::vector<BBIndex> predecessors_;
    std::vector<BBIndex> successors_;
};

class Function {
public:
    Function(const std::string&);

    const std::string& GetFunctionName() const { return func_name_; }

    ValueIndex GetLocalBase() const { return local_base_; }

    void AddVariable(std::string, Variable*);
    int GetOffsetForVariable(const std::string&) const;

    ValueIndex CreateConstant(int);

    void SetEntry(BBIndex);
    void SetExit(BBIndex);

private:
    std::string func_name_;

    ValueIndex local_base_;

    BBIndex entry_idx_;
    BBIndex exit_idx_;

    std::map<std::string, Variable*> variable_map_;
    std::unordered_map<std::string, std::unordered_map<BBIndex, ValueIndex> > local_defs_;
};

} // namespace papyrus

#endif /* PAPYRUS_IR_H */
