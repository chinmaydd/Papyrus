#ifndef PAPYRUS_IR_H
#define PAPYRUS_IR_H

#include "Papyrus/Logger/Logger.h"
#include "FrontEnd/Operation.h"
#include "Variable.h"

#include <vector>
#include <algorithm>

#include <deque>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>

using VI = int; // ValueIndex
using BI = int; // BasicBlockIndex
using II = int; // InstructionIndex

namespace papyrus {

class IRConstructor;

class Value {
public:
    enum ValueType {
        VAL_ANY,
        VAL_CONST,
        VAL_LOCALBASE,
        VAL_GLOBALBASE,
        VAL_LOCATION,
        VAL_PHI,
        VAL_FUNC,
        VAL_VAR,
        VAL_BRANCH,
        VAL_FORMAL,
    };

    Value(ValueType);

    const std::vector<II>& GetUsers() const { return uses_; }
    const ValueType Type() const { return vty_; }

    void SetType(ValueType vty) { vty_ = vty; }
    void AddUsage(II ins_idx) { uses_.push_back(ins_idx); }
    void SetConstant(int val) { val_ = val; }
    void SetIdentifier(const std::string& ident) { identifier_ = ident; }
    void RemoveUse(II);
    void AddDepth(int depth) { loop_depth_ = depth; }

    std::string Identifier() const { return identifier_; }

    int GetConstant() const { return val_; }

    bool IsConstant() const { return vty_ == VAL_CONST; }
    bool RequiresReg() const;

private:
    ValueType vty_;

    int val_;
    int loop_depth_;

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
        INS_ARG,

        INS_RET,
        INS_MOVE,

        INS_PHI,

        INS_NEG,
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

        INS_READ,
        INS_WRITEX,
        INS_WRITENL,

        INS_END,

        INS_ANY,
    };

    Instruction(InstructionType, BI, II);

    void AddOperand(VI); 
    void AddOperand(VI, BI);

    void SetResult(VI res) { result_ = res; }
    void MakeInactive() { is_active_ = false; }
    void ReplaceUse(VI, VI);

    VI Result() const { return result_; }

    BI ContainingBB() const { return containing_bb_; }

    InstructionType Type() const { return ins_type_; }

    const std::vector<VI>& Operands() const { return operands_; }

    II Index() const { return ins_idx_; }

    std::string ConvertToString() const;

    bool IsPhi() const { return ins_type_ == INS_PHI; }
    bool IsActive() const { return is_active_; }

    const std::unordered_map<BI, VI> OpSource() { return op_source_; }

private:
    InstructionType ins_type_;

    std::vector<VI> operands_;
    VI result_;

    II ins_idx_;

    BI containing_bb_;
    // Currently implemented only for Phi
    std::unordered_map<BI, VI> op_source_;

    bool is_active_;
};

using T = Instruction::InstructionType;
using V = Value::ValueType;

static bool IsGlobalStore(T insty) {
    return (insty == T::INS_STORE);
}

static bool IsGlobalLoad(T insty) {
    return (insty == T::INS_LOAD);
}

static bool IsFunctionCall(T insty) {
    return (insty == T::INS_CALL);
}

static const std::unordered_map<T, std::string> ins_to_str_ = {
    {T::INS_NONE,    "NONE"},
    {T::INS_ADDA,    "ADDA"},
    {T::INS_LOAD,    "LOAD"},
    {T::INS_STORE,   "STORE"},
    {T::INS_LOAD,   "LOADG"},
    {T::INS_STORE,  "STOREG"},
    {T::INS_CALL,    "CALL"},
    {T::INS_PHI,     "PHI"},
    {T::INS_ADD,     "ADD"},
    {T::INS_SUB,     "SUB"},
    {T::INS_MUL,     "MUL"},
    {T::INS_DIV,     "DIV"},
    {T::INS_CMP,     "CMP"},
    {T::INS_BEQ,     "BEQ"},
    {T::INS_BNE,     "BNE"},
    {T::INS_BLT,     "BLT"},
    {T::INS_BLE,     "BLE"},
    {T::INS_BGT,     "BGT"},
    {T::INS_BGE,     "BGE"},
    {T::INS_BRA,     "BRA"},
    {T::INS_END,     "END"},
    {T::INS_CALL,    "CALL"},
    {T::INS_MOVE,    "MOVE"},
    {T::INS_RET,     "RET"},
    {T::INS_ANY,     "ANY"},
    {T::INS_ARG,     "ARG"},
    {T::INS_READ,    "READ"},
    {T::INS_WRITEX,  "WRITE"},
    {T::INS_WRITENL, "WRITENL"},
};

class BasicBlock {
public:
    enum BBType {
        BB_START,
        BB_LOOPHEAD,
        BB_LOOPBODY,
        BB_THEN,
        BB_ELSE,
        BB_THROUGH,
        BB_END
    };

    BasicBlock(BI, BBType);

    void AddPredecessor(BI);
    void AddSuccessor(BI);
    void AddInstruction(II, Instruction*);
    void Seal() { is_sealed_ = true; }
    void Unseal() { is_sealed_ = false; }
    void MarkDead() { is_dead_ = true; }
    void EndBB() { is_ended_ = true; }
    void SetSelfValue(VI sv) { self_value_ = sv; }

    const std::vector<BI> Predecessors() const;
    const std::vector<BI> Successors() const;

    const std::deque<II>& InstructionOrder() const { return instruction_order_; }
    const std::unordered_map<II, Instruction*> Instructions() const { return instructions_; }

    bool HasActiveInstructions() const;
    bool IsSealed() const { return is_sealed_; }
    bool HasEnded() const { return is_ended_; }
    bool IsDead() const { return is_dead_; }
    
    VI GetSelfValue() const { return self_value_; }

    II GetAddAForLS(II) const;

    BI Idx() const { return idx_; }

    BBType Type() const { return type_; }

private:
    BI idx_;
    BBType type_;

    std::unordered_map<II, Instruction*> instructions_;
    std::deque<II> instruction_order_;

    std::vector<BI> predecessors_;
    std::vector<BI> successors_;

    bool is_sealed_;
    bool is_ended_;
    bool is_dead_;

    VI self_value_;
};

using B = BasicBlock::BBType;

static const std::unordered_map<B, std::string> bra_to_str_ = {
    { B::BB_START,    "BB_START"}, 
    { B::BB_LOOPHEAD, "BB_LOOPHEAD"},
    { B::BB_LOOPBODY, "BB_LOOPBODY"},
    { B::BB_THEN,     "BB_THEN"}, 
    { B::BB_ELSE,     "BB_ELSE"},  
    { B::BB_THROUGH,  "BB_THROUGH"},
    { B::BB_END,      "BB_END"}
};

class Function {
public:
    Function(const std::string&, VI, std::unordered_map<VI, Value*>*);

    const std::string& FunctionName() const { return func_name_; }
    const Variable* GetVariable(const std::string& var_name) const;
    const std::unordered_map<BI, BasicBlock*> BasicBlocks() const;

    std::vector<BI> PostOrderCFG();
    std::vector<BI> ReversePostOrderCFG();
    std::vector<BI> ExitBlocks();

    std::string ConvertInstructionToString(II) const;
    std::string ConvertValueToString(VI) const;
    
    std::string HashInstruction(T, VI, VI) const;
    std::string HashInstruction(T, VI) const;
    std::string HashInstruction(T) const;

    void SetLocalBase(VI val) { local_base_ = val; }
    void AddVariable(const std::string&, Variable*);
    void AddUsage(VI, II);
    void SetValueType(VI, V);
    void WriteVariable(const std::string&, VI);
    void WriteVariable(const std::string&, BI, VI);
    void AddBBEdge(BI, BI);        // pred, succ
    void SealBB(BI);
    void UnsealAllBB();
    void SetCurrentBB(BI idx) { current_bb_ = idx; }
    void AddExitBlock(BI idx);
    void MakeMove(const std::string&, VI);
    void ReplaceUse(VI, VI);
    void AddBackEdge(VI, VI);

    int GetOffset(const std::string&) const;
    int ReduceCondition(RelationalOperator, VI, VI) const;

    Value* GetValue(VI) const;

    BI CreateBB(B);
    BI CurrentBBIdx() const { return current_bb_; }

    BasicBlock* CurrentBB() const;
    BasicBlock* GetBB(BI) const;

    VI CreateConstant(int);
    VI CreateValue(V);

    VI GetCounter() const { return value_counter_; }
    VI ReadVariable(const std::string&, BI);
    VI ReadGlobal(const std::string&, BI);
    VI LocalBase() const { return local_base_; }

    VI MakeInstruction(T);
    VI MakeInstruction(T, VI);
    VI MakeInstruction(T, VI, VI);

    VI SelfIdx() const { return self_idx_; }
    VI Reduce(VI, VI, ArithmeticOperator);
    VI Reduce(VI, VI, T);
    VI GetLocationValue(const std::string&) const;

    Instruction* CurrentInstruction() const;
    Instruction* GetInstruction(II) const;

    bool IsActive(II) const;
    bool HasEndedBB(BI idx) const;
    bool IsVariableLocal(const std::string&) const;
    bool IsReducible(VI, VI) const;
    bool IsArithmetic(T) const;
    bool IsRelational(T) const;
    bool IsBackEdge(VI, VI) const;

private:
    std::string func_name_;

    VI local_base_;
    VI value_counter_;
    VI self_idx_;
    
    Value* self_;

    std::unordered_map<VI, Value*>* value_map_;
    std::unordered_map<std::string, std::unordered_map<BI, VI> > local_defs_;
    std::unordered_map<BI, std::unordered_map<std::string, II> > incomplete_phis_;
    std::unordered_map<BI, BasicBlock*> basic_block_map_;
    std::unordered_map<std::string, Variable*> variable_map_;
    std::unordered_map<std::string, VI> hash_map_;
    std::unordered_map<int, VI> constant_map_;

    std::unordered_map<II, Instruction*> instruction_map_;

    std::deque<II> instruction_order_;

    std::vector<BI> postorder_cfg_;
    std::vector<BI> rev_postorder_cfg_;
    std::vector<BI> exit_blocks_;

    std::unordered_map<VI, VI> back_edges_;

    BI current_bb_;
    BI bb_counter_;
    BI GetBBForInstruction(II);

    VI ReadVariableRecursive(const std::string&, BI);
    VI ReadGlobalRecursive(const std::string&, BI);
    VI AddPhiOperands(const std::string&, VI);
    VI SearchAndFillPhi(const std::string&, VI);
    VI TryRemoveTrivialPhi(II);
    VI ResultForInstruction(II) const;

    II instruction_counter_;
    II MakePhi();

    void AddBBPredecessor(BI, BI); // current, predecessor
    void AddBBSuccessor(BI, BI);   // current, successor
    void Visit(BI, std::unordered_set<BI>&);

    bool IsPhi(II) const;
    bool IsEliminable(T) const;
};

} // namespace papyrus

#endif /* PAPYRUS_IR_H */
