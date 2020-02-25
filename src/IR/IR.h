#ifndef PAPYRUS_IR_H
#define PAPYRUS_IR_H

#include "Papyrus/Logger/Logger.h"
#include "FrontEnd/Operation.h"
#include "Variable.h"

#include <vector>
#include <map>
#include <unordered_map>
#include <string>

using ValueIndex       = int;
using BBIndex          = int;
using InstructionIndex = int;

namespace papyrus {

class IRConstructor;

class Value {
public:
    enum ValueType {
        VAL_NONE,

        VAL_CONST,
        VAL_LOCALBASE,
        VAL_GLOBALBASE,

        VAL_ANY,
    };

    Value(ValueType);

    const ValueType GetType() const { return vty_; }
    void AddUsage(InstructionIndex ins_idx) { uses_.push_back(ins_idx); }
    void SetConstant(int val) { val_ = val; }

private:
    ValueType vty_;
    int val_;
    std::vector<InstructionIndex> uses_;
};

class Instruction {
public:
    enum InstructionType {
        INS_NONE,

        INS_STORE,
        INS_CALL,

        INS_ADDA,

        INS_ADD,
        INS_SUB,
        INS_MUL,
        INS_DIV,

        INS_CMP,
        INS_EQ,
        INS_NEQ,
        INS_LT,
        INS_LTE,
        INS_GT,
        INS_GTE,

        INS_END,

        INS_ANY,
    };

    Instruction(InstructionType, BBIndex, InstructionIndex);
    void AddArgument(ValueIndex val_idx) { arguments_.push_back(val_idx); }
    void SetResult(ValueIndex res) { result_ = res; }

private:
    InstructionType ins_type_;

    std::vector<ValueIndex> arguments_;
    ValueIndex result_;

    InstructionIndex ins_idx_;

    BBIndex containing_bb_;
};

using T = Instruction::InstructionType;
using V = Value::ValueType;

class BasicBlock {
public:
    BasicBlock(BBIndex);
    void AddPredecessor(BBIndex);
    void AddSuccessor(BBIndex);

    void AddInstruction(InstructionIndex, Instruction*);

private:
    BBIndex idx_;
    std::map<InstructionIndex, Instruction*> instructions_;

    std::vector<BBIndex> predecessors_;
    std::vector<BBIndex> successors_;
};

class Function {
public:
    Function(const std::string&, ValueIndex, std::unordered_map<ValueIndex, Value*>*);

    const std::string& FunctionName() const { return func_name_; }

    ValueIndex LocalBase() const { return local_base_; }
    void SetLocalBase(ValueIndex val) { local_base_ = val; }

    const Variable* GetVariable(const std::string& var_name) const { return variable_map_.at(var_name); }
    void AddVariable(const std::string&, Variable*);

    bool IsVariableLocal(const std::string&) const;
    int GetOffset(const std::string&) const;

    ValueIndex CreateConstant(int);
    ValueIndex CreateValue(Value::ValueType);
    void AddUsage(ValueIndex, InstructionIndex);
    ValueIndex GetCounter() const { return value_counter_; }
    
    void WriteVariable(const std::string&, ValueIndex);
    void WriteVariable(const std::string&, BBIndex, ValueIndex);

    BBIndex CreateBB();
    void AddBBEdge(BBIndex, BBIndex);        // pred, succ
    void AddBBPredecessor(BBIndex, BBIndex); // current, predecessor
    void AddBBSuccessor(BBIndex, BBIndex);   // current, successor

    BBIndex CurrentBBIdx() const { return current_bb_; }
    BasicBlock* CurrentBB() const { return basic_block_map_.at(CurrentBBIdx()); }
    void SetCurrentBB(BBIndex idx) { current_bb_ = idx; }

    ValueIndex MakeInstruction(T);
    ValueIndex MakeInstruction(T, ValueIndex);
    ValueIndex MakeInstruction(T, ValueIndex, ValueIndex);

    Instruction* CurrentInstruction() const;

private:
    std::string func_name_;

    ValueIndex local_base_;

    ValueIndex value_counter_;
    std::unordered_map<ValueIndex, Value*>* value_map_;
    std::unordered_map<std::string, std::unordered_map<BBIndex, ValueIndex> > local_defs_;

    InstructionIndex instruction_counter_;
    std::unordered_map<InstructionIndex, Instruction*> instruction_map_;

    BBIndex entry_idx_;
    BBIndex exit_idx_;
    BBIndex current_bb_;
    BBIndex bb_counter_;
    std::unordered_map<BBIndex, BasicBlock*> basic_block_map_;

    std::map<std::string, Variable*> variable_map_;
};

} // namespace papyrus

#endif /* PAPYRUS_IR_H */
