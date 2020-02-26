#ifndef PAPYRUS_IR_H
#define PAPYRUS_IR_H

#include "Papyrus/Logger/Logger.h"
#include "FrontEnd/Operation.h"
#include "Variable.h"

#include <vector>
#include <map>
#include <unordered_map>
#include <deque>
#include <algorithm>
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

        VAL_PHI,

        VAL_ANY,
    };

    Value(ValueType);

    const ValueType GetType() const { return vty_; }
    void SetType(ValueType vty) { vty_ = vty; }
    void AddUsage(InstructionIndex ins_idx) { uses_.push_back(ins_idx); }
    void SetConstant(int val) { val_ = val; }

    void RemoveUse(InstructionIndex);

    const std::vector<InstructionIndex>& GetUsers() const { return uses_; }

private:
    ValueType vty_;
    int val_;
    std::vector<InstructionIndex> uses_;
};

class Instruction {
public:
    enum InstructionType {
        INS_NONE,

        INS_ADDA,

        INS_LOAD,
        INS_STORE,
        INS_CALL,

        INS_PHI,

        INS_ADD,
        INS_SUB,
        INS_MUL,
        INS_DIV,

        INS_CMP,
        INS_BEQ,
        INS_BNE,
        INS_BLT,
        INS_BLE,
        INS_BGT,
        INS_BGE,
        INS_BRA,

        INS_END,

        INS_ANY,
    };

    Instruction(InstructionType, BBIndex, InstructionIndex);
    void AddOperand(ValueIndex val_idx) { operands_.push_back(val_idx); }
    void SetResult(ValueIndex res) { result_ = res; }
    ValueIndex Result() const { return result_; }
    BBIndex ContainingBB() const { return containing_bb_; }

    bool IsPhi() const { return ins_type_ == INS_PHI; }

    const std::vector<ValueIndex>& Operands() const { return operands_; }

private:
    InstructionType ins_type_;

    std::vector<ValueIndex> operands_;
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

    const std::vector<BBIndex> Predecessors() const;
    const std::vector<BBIndex> Successors() const;

    bool IsSealed() const { return is_sealed_; }
    void Seal() { is_sealed_ = true; }

private:
    BBIndex idx_;
    std::map<InstructionIndex, Instruction*> instructions_;

    std::vector<BBIndex> predecessors_;
    std::vector<BBIndex> successors_;

    bool is_sealed_;
};

class Function {
public:
    Function(const std::string&, ValueIndex, std::unordered_map<ValueIndex, Value*>*);

    const std::string& FunctionName() const { return func_name_; }

    ValueIndex LocalBase() const { return local_base_; }
    void SetLocalBase(ValueIndex val) { local_base_ = val; }

    const Variable* GetVariable(const std::string& var_name) const;
    void AddVariable(const std::string&, Variable*);

    bool IsVariableLocal(const std::string&) const;
    int GetOffset(const std::string&) const;

    ValueIndex CreateConstant(int);
    ValueIndex CreateValue(V);
    void AddUsage(ValueIndex, InstructionIndex);


    Value* GetValue(ValueIndex) const;
    void SetValueType(ValueIndex, V);

    ValueIndex GetCounter() const { return value_counter_; }
    
    ValueIndex ReadVariable(const std::string&, BBIndex);
    void WriteVariable(const std::string&, ValueIndex);
    void WriteVariable(const std::string&, BBIndex, ValueIndex);

    BBIndex CreateBB();
    void AddBBEdge(BBIndex, BBIndex);        // pred, succ
    const std::map<BBIndex, BasicBlock*> BasicBlocks() const;

    BBIndex CurrentBBIdx() const { return current_bb_; }
    BasicBlock* CurrentBB() const;
    BasicBlock* GetBB(BBIndex) const;
    void SetCurrentBB(BBIndex idx) { current_bb_ = idx; }

    ValueIndex MakeInstruction(T);
    ValueIndex MakeInstruction(T, ValueIndex);
    ValueIndex MakeInstruction(T, ValueIndex, ValueIndex);

    Instruction* CurrentInstruction() const;
    Instruction* GetInstruction(InstructionIndex) const;

private:
    std::string func_name_;

    ValueIndex local_base_;

    ValueIndex value_counter_;
    std::unordered_map<ValueIndex, Value*>* value_map_;
    std::unordered_map<std::string, std::unordered_map<BBIndex, ValueIndex> > local_defs_;
    std::unordered_map<std::string, std::unordered_map<BBIndex, ValueIndex> > incomplete_phis_;

    InstructionIndex instruction_counter_;
    std::unordered_map<InstructionIndex, Instruction*> instruction_map_;
    std::deque<InstructionIndex> instruction_order_;

    BBIndex current_bb_;
    BBIndex bb_counter_;
    std::map<BBIndex, BasicBlock*> basic_block_map_;

    std::map<std::string, Variable*> variable_map_;

    ValueIndex ReadVariableRecursive(const std::string&, BBIndex);
    ValueIndex AddPhiOperands(const std::string&, ValueIndex);
    InstructionIndex MakePhi();
    ValueIndex TryRemoveTrivialPhi(InstructionIndex);

    void AddBBPredecessor(BBIndex, BBIndex); // current, predecessor
    void AddBBSuccessor(BBIndex, BBIndex);   // current, successor

    BBIndex GetBBForInstruction(InstructionIndex);

    bool IsPhi(InstructionIndex) const;
};

} // namespace papyrus

#endif /* PAPYRUS_IR_H */
