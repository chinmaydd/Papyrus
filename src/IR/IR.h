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

using VI = int; // ValueIndex
using BI = int; // BasicBlockIndex
using II = int; // InstructionIndex

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

        VAL_FUNC,
        
        VAL_BRANCH,

        VAL_ANY,
    };

    Value(ValueType);

    const ValueType GetType() const { return vty_; }
    void SetType(ValueType vty) { vty_ = vty; }
    void AddUsage(II ins_idx) { uses_.push_back(ins_idx); }
    void SetConstant(int val) { val_ = val; }

    std::string Identifier() const { return identifier_; }
    void SetIdentifier(const std::string& ident) { identifier_ = ident; }

    int GetValue() const { return val_; }

    void RemoveUse(II);

    const std::vector<II>& GetUsers() const { return uses_; }

private:
    ValueType vty_;
    int val_;
    std::string identifier_;
    std::vector<II> uses_;
};

class Instruction {
public:
    enum InstructionType {
        INS_NONE,

        INS_ADDA,

        INS_LOAD,
        INS_STORE,
        INS_CALL,
        INS_RET,

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

    Instruction(InstructionType, BI, II);

    void AddOperand(VI val_idx) { operands_.push_back(val_idx); }

    void SetResult(VI res) { result_ = res; }
    VI Result() const { return result_; }

    BI ContainingBB() const { return containing_bb_; }

    InstructionType Type() const { return ins_type_; }
    bool IsPhi() const { return ins_type_ == INS_PHI; }

    const std::vector<VI>& Operands() const { return operands_; }

    void ReplaceUse(VI, VI);

    std::string ConvertToString() const;

    bool IsActive() const { return is_active_; }
    void MakeInactive() { is_active_ = false; }

private:
    InstructionType ins_type_;

    std::vector<VI> operands_;
    VI result_;

    II ins_idx_;

    BI containing_bb_;

    bool is_active_;

};

using T = Instruction::InstructionType;
using V = Value::ValueType;

static const std::unordered_map<T, std::string> ins_to_str_ = {
    {T::INS_NONE,   "NONE"},
    {T::INS_ADDA,   "ADDA"},
    {T::INS_LOAD,   "LOAD"},
    {T::INS_STORE,  "STORE"},
    {T::INS_CALL,   "CALL"},
    {T::INS_PHI,    "PHI"},
    {T::INS_ADD,    "ADD"},
    {T::INS_SUB,    "SUB"},
    {T::INS_MUL,    "MUL"},
    {T::INS_DIV,    "DIV"},
    {T::INS_CMP,    "CMP"},
    {T::INS_BEQ,    "BEQ"},
    {T::INS_BNE,    "BNE"},
    {T::INS_BLT,    "BLT"},
    {T::INS_BLE,    "BLE"},
    {T::INS_BGT,    "BGT"},
    {T::INS_BGE,    "BGE"},
    {T::INS_BRA,    "BRA"},
    {T::INS_END,    "END"},
    {T::INS_CALL,   "CALL"},
    {T::INS_RET,    "RET"},
    {T::INS_ANY,    "ANY"},
};

class BasicBlock {
public:
    BasicBlock(BI);
    void AddPredecessor(BI);
    void AddSuccessor(BI);

    void AddInstruction(II, Instruction*);

    const std::vector<BI> Predecessors() const;
    const std::vector<BI> Successors() const;

    bool IsSealed() const { return is_sealed_; }
    void Seal() { is_sealed_ = true; }

    const std::deque<II>& InstructionOrder() const { return instruction_order_; }
    const std::map<II, Instruction*> Instructions() const { return instructions_; }

    bool HasActiveInstructions() const;

    void EndBB() { is_ended_ = true;}
    bool HasEnded() const { return is_ended_; }
    
    VI GetSelfValue() const { return self_value_; }
    void SetSelfValue(VI sv) { self_value_ = sv; }

private:
    BI idx_;
    std::map<II, Instruction*> instructions_;
    std::deque<II> instruction_order_;

    std::vector<BI> predecessors_;
    std::vector<BI> successors_;

    bool is_sealed_;
    bool is_ended_;

    VI self_value_;
};

class Function {
public:
    Function(const std::string&, VI, std::unordered_map<VI, Value*>*);

    const std::string& FunctionName() const { return func_name_; }

    VI LocalBase() const { return local_base_; }
    void SetLocalBase(VI val) { local_base_ = val; }

    const Variable* GetVariable(const std::string& var_name) const;
    void AddVariable(const std::string&, Variable*);

    bool IsVariableLocal(const std::string&) const;
    int GetOffset(const std::string&) const;

    VI CreateConstant(int);
    VI CreateValue(V);
    void AddUsage(VI, II);


    Value* GetValue(VI) const;
    void SetValueType(VI, V);

    VI GetCounter() const { return value_counter_; }
    
    VI ReadVariable(const std::string&, BI);
    void WriteVariable(const std::string&, VI);
    void WriteVariable(const std::string&, BI, VI);

    BI CreateBB();
    void AddBBEdge(BI, BI);        // pred, succ
    const std::unordered_map<BI, BasicBlock*> BasicBlocks() const;
    void SealBB(BI);

    BI CurrentBBIdx() const { return current_bb_; }
    BasicBlock* CurrentBB() const;
    BasicBlock* GetBB(BI) const;
    void SetCurrentBB(BI idx) { current_bb_ = idx; }
    bool HasEndedBB(BI idx) const;

    VI MakeInstruction(T);
    VI MakeInstruction(T, VI);
    VI MakeInstruction(T, VI, VI);

    Instruction* CurrentInstruction() const;
    Instruction* GetInstruction(II) const;
    bool IsActive(II) const;

    std::string ConvertInstructionToString(II) const;
    std::string ConvertValueToString(VI) const;

    VI SelfIdx() const { return self_idx_; }

private:
    std::string func_name_;

    VI local_base_;
    
    Value* self_;
    VI self_idx_;

    VI value_counter_;
    std::unordered_map<VI, Value*>* value_map_;
    std::unordered_map<std::string, std::unordered_map<BI, VI> > local_defs_;
    std::unordered_map<BI, std::unordered_map<std::string, II> > incomplete_phis_;

    II instruction_counter_;
    std::unordered_map<II, Instruction*> instruction_map_;
    std::deque<II> instruction_order_;

    BI current_bb_;
    BI bb_counter_;
    std::unordered_map<BI, BasicBlock*> basic_block_map_;

    std::unordered_map<std::string, Variable*> variable_map_;

    VI ReadVariableRecursive(const std::string&, BI);
    VI AddPhiOperands(const std::string&, VI);
    II MakePhi();
    VI TryRemoveTrivialPhi(II);

    void AddBBPredecessor(BI, BI); // current, predecessor
    void AddBBSuccessor(BI, BI);   // current, successor

    BI GetBBForInstruction(II);

    bool IsPhi(II) const;

    VI ResultForInstruction(II) const;

    bool IsRelational(T) const;
};

} // namespace papyrus

#endif /* PAPYRUS_IR_H */
