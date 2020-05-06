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
#include <stack>
#include <unordered_map>
#include <unordered_set>

using VI = int; // ValueIndex
using BI = int; // BasicBlockIndex
using II = int; // InstructionIndex

namespace papyrus {

class IRConstructor;

/*
 * Value class describes the fundamental value in the compiler.
 */
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
        VAL_STACK,

        VAL_NONE // Just in case
    };

    Value(ValueType);

    const std::vector<II>& GetUsers() const { return uses_; }
    const ValueType Type() const { return vty_; }

    void SetType(ValueType vty) { vty_ = vty; }
    void AddUsage(II ins_idx) { uses_.push_back(ins_idx); }
    void SetConstant(int val) { val_ = val; }
    void SetIdentifier(const std::string& ident) { identifier_ = ident; }
    void RemoveUse(II);

    void SetDepth(int depth) { loop_depth_ = depth; }
    void SetSpillCost(long double cost) { spill_cost_ = cost; }

    std::string Identifier() const { return identifier_; }

    int GetConstant() const { return val_; }

    int LoopDepth() const { return loop_depth_; }
    long double SpillCost() const { return spill_cost_; }

    bool IsConstant() const { return vty_ == VAL_CONST; }

private:
    // ValueType for each value describes the "type" of the value. The distinction
    // is blurry but it serves its purpose during IR construction and visualization
    ValueType vty_;

    // If the value is a constant (number), val_ contains that constant value
    int val_;
    
    // loop_depth_ and spill_cost_ are used during Register Allocation to determine
    // how much "costly" the value is for spilling
    int loop_depth_;
    long double spill_cost_;

    // In case the value is a variable or global which can be identified, the
    // identifier is stored (also helps during the visualization phase)
    std::string identifier_;

    // uses_ vector contains all the uses of the value in various instructions
    // inside the function
    std::vector<II> uses_;
};

/*
 * The Instruction class is the description of an instruction of the IR
 */
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

        INS_KILL,

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

    BI FindSource(VI) const;

    VI Result() const { return result_; }

    BI ContainingBB() const { return containing_bb_; }

    InstructionType Type() const { return ins_type_; }

    const std::vector<VI>& Operands() const { return operands_; }

    II Index() const { return ins_idx_; }

    std::string ConvertToString() const;

    bool IsPhi() const { return ins_type_ == INS_PHI; }
    bool IsKill() const { return ins_type_ == INS_KILL; }
    bool IsActive() const { return is_active_; }

    const std::unordered_map<BI, VI> OpSource() { return op_source_; }

private:
    // For each instruction, we store the type of the instruction. Unlike 
    // that for `Value` this field is well-defined and important
    InstructionType ins_type_;

    // Vector of operands (ValueIndex) for the instruction
    std::vector<VI> operands_;
    
    // ValueIndex of the result
    VI result_;

    // Instruction Index of self
    II ins_idx_;

    // Index of BasicBlock containing the instruction
    BI containing_bb_;

    // Currently implemented only for Phi
    // For each BasicBlock, ideally the predecessor of the containing_bb, this 
    // tells is which value flows into the current instruction
    std::unordered_map<BI, VI> op_source_;

    // Instructions are not deleted in the IR, rather they are made "inactive".
    // This allows us to get around memory issues such as use-after-free. It 
    // might make sense to keep the deleted instructions around since some
    // analyses might require them.
    bool is_active_;
};

using T = Instruction::InstructionType;
using V = Value::ValueType;

/* 
 * Utility functions to identify instruction types
 */

static bool IsGlobalStore(T insty) {
    return (insty == T::INS_STORE);
}

static bool IsGlobalLoad(T insty) {
    return (insty == T::INS_LOAD);
}

static bool IsFunctionCall(T insty) {
    return (insty == T::INS_CALL);
}

// InstructionType -> String map to be used during visualization
static const std::unordered_map<T, std::string> ins_to_str_ = {
    {T::INS_NONE,    "NONE"},
    {T::INS_ADDA,    "adda"},
    {T::INS_LOAD,    "load"},
    {T::INS_STORE,   "store"},
    {T::INS_CALL,    "call"},
    {T::INS_PHI,     "φ"},
    {T::INS_KILL,    "kill"},
    {T::INS_ADD,     "add"},
    {T::INS_SUB,     "sub"},
    {T::INS_MUL,     "mul"},
    {T::INS_DIV,     "div"},
    {T::INS_CMP,     "cmp"},
    {T::INS_BEQ,     "beq"},
    {T::INS_BNE,     "bne"},
    {T::INS_BLT,     "blt"},
    {T::INS_BLE,     "ble"},
    {T::INS_BGT,     "bgt"},
    {T::INS_BGE,     "bge"},
    {T::INS_BRA,     "bra"},
    {T::INS_END,     "end"},
    {T::INS_MOVE,    "move"},
    {T::INS_RET,     "ret"},
    {T::INS_ANY,     "any"},
    {T::INS_ARG,     "arg"},
    {T::INS_READ,    "read"},
    {T::INS_WRITEX,  "write"},
    {T::INS_WRITENL, "writenl"},
};

/*
 * The BasicBlock Class describes the BasicBlock in the IR
 */
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
    void AddInstructionFront(II, Instruction*);
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

    II GetPreviousInstruction(II) const;

    BI Idx() const { return idx_; }

    BBType Type() const { return type_; }

private:
    // Index of the BasicBlock
    BI idx_;
    
    // Type of the BasicBlock. This is mainly useful for identifying loop headers
    // and join nodes.
    BBType type_;

    // Vector of instructions contained inside this BasicBlock
    std::unordered_map<II, Instruction*> instructions_;
    
    // We store a deque of the ordering of instructions contained by the BasicBlock
    // The reason for keeping this a deque is that when creating Phi functions 
    // lazily, we would like to add them to the beginning of the instruction_order
    std::deque<II> instruction_order_;

    // Predecessors and Successors of the current BB
    std::vector<BI> predecessors_;
    std::vector<BI> successors_;

    // Check if the BB is sealed. This is used during SSA generation
    bool is_sealed_;

    // Check if the BB has ended. If the current BB contains a return instruction, 
    // it cannot have any more successors. We mark it as ended
    bool is_ended_;

    // Check if the BB is dead. This is specifically useful during DCE.
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

/*
 * The Function Class is what we mostly operate on during analysis 
 * and IR creation. The Function contains multiple BBs which in-turn
 * have instructions.
 *
 * NOTE to the curios reader: Yes, the API is extremely bloated and a lot
 * of functions are unncessary leading to code duplication. But at the time, 
 * the need of the hour was to get the project done. Please create issues 
 * highlighting what you would like to change
 */
class Function {
public:
    Function(const std::string&, VI, std::unordered_map<VI, Value*>*);

    const std::string& FunctionName() const { return func_name_; }
    const Variable* GetVariable(const std::string&) const;
    const std::unordered_map<BI, BasicBlock*> BasicBlocks() const;
    const std::unordered_map<std::string, Variable*> Variables() const;
    const std::unordered_set<VI> GetKilledValues(BI) const;
    const std::unordered_map<BI, BI>& DominatorTree() const;
    const std::unordered_map<BI, BI>& DominanceFrontier() const;

    std::vector<BI> PostOrderCFG();
    std::vector<BI> ReversePostOrderCFG();
    std::vector<BI> ExitBlocks();

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
    void AddBackEdge(BI, BI);
    void LoadFormal(const std::string&);
    void InsertHash(const std::string&, VI);

    void ComputeDominatorTree();
    void ComputeDominanceFrontier();

    // (instruction_idx, final_ins_idx)
    void AddArrContributor(II, II);
    void KillBB(BI, VI);

    VI CreateMove(BI, VI, int);

    int GetOffset(const std::string&) const;
    int ReduceCondition(RelationalOperator, VI, VI) const;

    Value* GetValue(VI) const;

    BI CreateBB(B);
    BI CurrentBBIdx() const { return current_bb_; }

    II CurrentInstructionIdx() const;

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
    VI MakeInstructionFront(T);
    VI MakeInstructionFront(T, VI);

    VI SelfIdx() const { return self_idx_; }
    VI TryReduce(ArithmeticOperator, VI, VI);
    VI GetLocationValue(const std::string&) const;
    
    VI GetHash(const std::string& hash_str) const;

    Instruction* CurrentInstruction() const;
    Instruction* GetInstruction(II) const;

    bool IsActive(II) const;
    bool HasEndedBB(BI idx) const;

    bool IsVariableLocal(const std::string&) const;
    bool IsVariableFormal(const std::string&) const;
    bool IsFormalLoaded(const std::string&) const;

    bool IsReducible(VI, VI) const;
    bool IsArithmetic(T) const;
    bool IsRelational(T) const;
    bool IsBackEdge(BI, BI) const;
    bool IsPhi(II) const;
    bool IsEliminable(T) const;

    bool HashExists(const std::string&) const;

    bool IsKilled(BI) const;

    // NOTE:
    // Added later
    // Ideally, they should be behind an API
    // Temporary access_string for an array access (load/store). This is used
    // when walking over the AST.
    std::string access_str_;
    // For each value, we save the load access hashes and store access hashes.
    // These are then used later during an analysis phase to remove 
    // redundant array loads and stores.
    std::unordered_map<VI, std::string> load_hash_;
    std::unordered_map<VI, std::string> store_hash_;

    // Temporary used across functions during ASTWalk
    std::vector<II> load_contributors;

    // For each load_instruction, we store the related instructions used
    // for generating this load instruction. Ideally, this isnt needed
    // if the DCE pass is good.
    std::unordered_map<II, std::unordered_set<II> > load_related_insts_;

private:
    // Name of the function
    std::string func_name_;

    // Value of the local base of the function
    VI local_base_;

    // Current Value Counter. There is a global Value counter maintained when 
    // IR is generated. This value counter tells what would be the number
    // of the next value generated in the function
    VI value_counter_;

    // Each function also has a value by itself. This is done so that during
    // call <function> we can use that value and later during visualisation, we 
    // can extract the identifier and treat it like a var. Just becomes more
    // convenient
    VI self_idx_;
    Value* self_;

    // This points to the Global Value Map. This map is created only once, on
    // the heap and shared across all the functions for which IR is generated.
    std::unordered_map<VI, Value*>* value_map_;
    
    // The following are data-structures used during SSA construction as described
    // by Braun et. al. 
    //
    // local_defs_ describes for each variable, in each BB, the value it is
    // currently defined by. This is used when there is a read() for that variable.
    // When there is a write, the current value is overwritten. If a value is not
    // found, we recursively search for its definition in the predecessors. Please
    // refer to SSA.cpp for more details.
    std::unordered_map<std::string, std::unordered_map<BI, VI> > local_defs_;
    // incomplete_phis_ are those which are lazily added before the sealing of
    // the block is done. If phis are trivial, they are removed.
    std::unordered_map<BI, std::unordered_map<std::string, II> > incomplete_phis_;

    // Stores a map from BI -> pointer to BasicBlock object
    std::unordered_map<BI, BasicBlock*> basic_block_map_;
    // Stores a map from variable name -> pointer to Variable Object
    std::unordered_map<std::string, Variable*> variable_map_;
    
    // Keep a hash_map for storing the hash_string and the value associated with it.
    // This is used for Common-Subexpression elimination (on-the-fly)
    std::unordered_map<std::string, VI> hash_map_;
    
    // This was an optimization I was experimenting with. What if, rather than
    // creating a new Value for each constant, we try and reuse those which 
    // are already created. For example if VI:4 is constant 9 which is used multiple
    // times, it might make sense to create it only once and reference it at
    // multiple places. However, this lead to longer live-ranges and I couldnt
    // really figure out how to get around that problem.
    std::unordered_map<int, VI> constant_map_;

    // Stores a map from II -> pointer to Instruction Object.
    // A copy of this is present in the containing BB as well.
    std::unordered_map<II, Instruction*> instruction_map_;

    // Order of the instruction sequence
    std::deque<II> instruction_order_;

    // PostOrder CFG of the BBs
    std::vector<BI> postorder_cfg_;
    // Reverse PostOrder CFG of the BBs
    std::vector<BI> rev_postorder_cfg_;
    // All the exit blocks of the function. Reverse analyses start here.
    std::vector<BI> exit_blocks_;

    // This is used to to understand which edges in the IR are back-edges
    // Some amount of bookkeeping to help with analyses later.
    std::unordered_map<BI, BI> back_edges_;

    // This is used to know which formal parameters are already loaded into registers
    // Since parameters are passed on the stack (as assumed by the ABI), we need
    // to load them into registers.
    //
    // I guess it is possible to understand how to pass them directly through 
    // registers and know which registers contain what formals. This requires
    // a context-sensitive and also a flow-sensitive analysis which I am not
    // quite sure how to incorporate with the Karlsruhe method. Keeping it 
    // as future work.
    std::unordered_set<std::string> loaded_formals_;

    // Stores the dominator tree of the graph. This is needed to fold the CFG
    // of the prorgam once "empty" blocks have been identified.
    std::unordered_map<BI, BI> dominator_tree_;

    // Stores dominance frontier
    std::unordered_map<BI, BI> dominance_frontier_;

    // Stores which BBs are killed
    std::unordered_map<BI, std::unordered_set<VI> > is_killed_;

    // Current BB used for instruction generation. New instructions in the current
    // context are added to this BB
    BI current_bb_;
    // Counter for BB creation.
    BI bb_counter_;

    BI GetBBForInstruction(II);
    BI Intersect(BI, BI);

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
};

} // namespace papyrus

#endif /* PAPYRUS_IR_H */
