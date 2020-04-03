#include "IR.h"

using namespace papyrus;

#define NOTFOUND -1 

Value::Value(ValueType vty) :
    vty_(vty) {}

Function::Function(const std::string& func_name, VI value_counter, std::unordered_map<VI, Value*>* value_map):
    func_name_(func_name),
    value_counter_(value_counter),
    bb_counter_(0),
    current_bb_(0),
    instruction_counter_(0),
    postorder_cfg_({}),
    rev_postorder_cfg_({}),
    hash_map_({}),
    back_edges_({}),
    constant_map_({}),
    load_contributors({}),
    load_related_insts_({}),
    is_killed_({}),
    value_map_(value_map) {
        SetLocalBase(CreateValue(V::VAL_LOCALBASE));
        SetCurrentBB(CreateBB(B::BB_START));
        // Seal the Start basic block since it has no predecessors
        SealBB(CurrentBBIdx());
}

const Variable* Function::GetVariable(const std::string& var_name) const { 
    return variable_map_.at(var_name); 
}

void Function::AddVariable(const std::string& var_name, Variable* var) {
    if (IsVariableLocal(var_name)) {
        LOG(ERROR) << "[IR] Attempt to redefine local variable " + var_name;
        exit(1);
    }

    variable_map_[var_name] = var;
}

void Function::LoadFormal(const std::string& var_name) {
    loaded_formals_.insert(var_name);
}

bool Function::IsVariableLocal(const std::string& var_name) const {
    return variable_map_.find(var_name) != variable_map_.end();
}

bool Function::IsVariableFormal(const std::string& var_name) const {
    return variable_map_.at(var_name)->IsFormal();
}

bool Function::IsFormalLoaded(const std::string& var_name) const {
    return loaded_formals_.find(var_name) != loaded_formals_.end();
}

int Function::GetOffset(const std::string& var_name) const {
    return variable_map_.at(var_name)->Offset();
}

// This is the Value generation API template code. When generating a value there
// are 2 main things to take care of:
//
// 1. Value Counter should be incremented. The NEW value_counter is the idx of the 
// new value.
// 2. Pointer to the new Value should be inserted into the Value map.
VI Function::CreateValue(V vty) {
    Value* val = new Value(vty);

    value_counter_++;
    value_map_->emplace(value_counter_, val);

    return value_counter_;
}

VI Function::CreateConstant(int val) {
    // Disabling this for now.
    // if (constant_map_.find(val) != constant_map_.end()) {
    //     return constant_map_.at(val);
    // }

    Value* v = new Value(V::VAL_CONST);
    v->SetConstant(val); 

    value_counter_++;
    value_map_->emplace(value_counter_, v);

    constant_map_.insert({val, value_counter_});

    return value_counter_;
}

void Function::AddUsage(VI val_idx, II ins_idx) {
    value_map_->at(val_idx)->AddUsage(ins_idx);
}

Value* Function::GetValue(VI val_idx) const {
    return value_map_->at(val_idx);
}

void Function::SetValueType(VI val_idx, V val_type) {
    Value* val = GetValue(val_idx);
    val->SetType(val_type);
}

// BasicBlock generation API. Things to take care of:
//
// 1. BB Counter should be incremented. New BB counter is what is assigned to
// the newly created block
// 2. Pointer to BB should be inserted into basic_block_map_ for the Function.
// 3. A Value should be assigned to the BB. This is used to treat the BB as 
// as an operand to the Jump instruction
BI Function::CreateBB(B bb_type) {
    bb_counter_++;

    BasicBlock* bb = new BasicBlock(bb_counter_, bb_type);
    basic_block_map_[bb_counter_] = bb;

    VI sv = CreateValue(V::VAL_BRANCH);
    GetValue(sv)->SetConstant(bb_counter_);
    bb->SetSelfValue(sv);

    return bb_counter_;
}

void Function::AddBBEdge(BI pred, BI succ) {
    AddBBPredecessor(succ, pred);
    AddBBSuccessor(pred, succ);
}

const std::unordered_map<BI, BasicBlock*> Function::BasicBlocks() const {
    return basic_block_map_;
}

const std::unordered_map<std::string, Variable*> Function::Variables() const {
    return variable_map_;
}

BasicBlock* Function::GetBB(BI bb_idx) const {
    return basic_block_map_.at(bb_idx);
}

BasicBlock* Function::CurrentBB() const { 
    return GetBB(CurrentBBIdx());
}

void Function::AddBBPredecessor(BI source, BI pred) {
    basic_block_map_[source]->AddPredecessor(pred);
}

void Function::AddBBSuccessor(BI source, BI pred) {
    basic_block_map_[source]->AddSuccessor(pred);
}

bool Function::HasEndedBB(BI bb_idx) const {
    return GetBB(bb_idx)->HasEnded();
}

void Function::AddExitBlock(BI bb_idx) {
    exit_blocks_.push_back(bb_idx);
}

void Function::AddBackEdge(BI from, BI to) {
    back_edges_.insert({from, to});
}

bool Function::IsBackEdge(BI from, BI to) const {
    if (back_edges_.find(from) == back_edges_.end()) {
        return false;
    }

    return back_edges_.at(from) == to;
}

bool Function::IsKilled(BI bb_idx) const {
    return is_killed_.find(bb_idx) != is_killed_.end();
}

void Function::KillBB(BI bb_idx) {
    is_killed_.insert(bb_idx);
}

// XXX:
// MakeMove is deprecated since the instruction is not required
// to be a part of the IR.
void Function::MakeMove(const std::string& var_name, VI expr_idx) {
    instruction_counter_++;

    Instruction* inst = new Instruction(T::INS_MOVE,
                                        CurrentBBIdx(),
                                        instruction_counter_);

    instruction_map_[instruction_counter_] = inst;
    instruction_order_.push_back(instruction_counter_);

    VI temp_value = CreateValue(V::VAL_VAR);
    GetValue(temp_value)->SetIdentifier(var_name);

    inst->AddOperand(expr_idx);
    inst->AddOperand(temp_value);

    CurrentBB()->AddInstruction(instruction_counter_, inst);
}

BI Function::GetBBForInstruction(II ins_idx) {
    return instruction_map_[ins_idx]->ContainingBB();
}

/*
 * These are hashes used for Common-Subexpression Elimination during
 * the IR building phase. The algorithm is such that, it allows us to check 
 * if we are building the same instruction again (since we almost 
 * navigate across the CFG in dominating order
 *
 * The hash is built such that:
 *
 * INSTRUCTION_ARG1_ARG2
 * OR
 * INSTRUCTION_ARG1
 *
 * Here, arguments are sorted and hence the generated hash will be unique.
 * We will just reuse the value generated earlier.
 */
std::string Function::HashInstruction(T insty) const {
    // Handle function calls here.
    // We could also get around calling the same functions here
    // I guess the conditions for that are that the function should not clobber
    // anything and should generate the same result for the same input
    // This is tough to check at this stage of the analysis however.
    return "NOTFOUND";
}

std::string Function::HashInstruction(T insty, VI arg_1) const {
    std::string return_str = "";
    return_str            += ins_to_str_.at(insty) + "_";
    return_str            += std::to_string(arg_1);

    return return_str;
}

std::string Function::HashInstruction(T insty, VI arg_1, VI arg_2) const {
    std::string return_str = "";
    return_str            += ins_to_str_.at(insty) + "_";

    if (arg_1 < arg_2) {
        return_str += std::to_string(arg_1) + "_";
        return_str += std::to_string(arg_2);
    } else {
        return_str += std::to_string(arg_2) + "_";
        return_str += std::to_string(arg_1);
    }

    return return_str;
}

bool Function::HashExists(const std::string& hash_str) const {
    return (hash_map_.find(hash_str) != hash_map_.end());
}

void Function::InsertHash(const std::string& hash_str, VI result) {
    hash_map_[hash_str] = result;
}

VI Function::GetHash(const std::string& hash_str) const {
    return hash_map_.at(hash_str);
}

/*
 * CSE here is on-the-fly CSE; to be done during AST construction. I guess it 
 * might still be possible to do this by some more bookkeeping but at this 
 * point it does not make sense to do that.
 */
bool Function::IsEliminable(T insty) const {
    return (insty == T::INS_NEG ||
            insty == T::INS_ADD ||
            insty == T::INS_SUB ||
            insty == T::INS_MUL ||
            insty == T::INS_DIV ||
            insty == T::INS_CMP);
}

/*
 * Create Instructions for the IR
 */

// Here, we need to remove Add GlobalBase, Mem_Location as a value which is 
// reused. This causes its live range to increase and hence the interference 
// with the rest of the variables.

// This function is never called standalone. Hence, not checking if instruction
// is eliminable. This is the Instruction generation API
//
// Things to take care of:
//
// 1. Instruction Counter is incremented.
// 2. Instruction is created with new instruction counter, current BB idx and 
// a type.
// 3. This is then pushed back "correctly" into the instruction order for the
// BB.
// 4. A Result Value is generated and returned.
VI Function::MakeInstruction(T insty) {
    instruction_counter_++;
    Instruction* inst = new Instruction(insty, CurrentBBIdx(), instruction_counter_);

    instruction_map_[instruction_counter_] = inst;
    instruction_order_.push_back(instruction_counter_);

    V vty = V::VAL_ANY;
    
    VI result = CreateValue(vty);
    inst->SetResult(result);

    CurrentBB()->AddInstruction(instruction_counter_, inst);

    return result;
}

VI Function::MakeInstruction(T insty, VI arg_1) {
    auto hash_str = HashInstruction(insty, arg_1);

    // Check if instruction can be removed
    if (IsEliminable(insty) &&
        arg_1 != 1) {
        if (HashExists(hash_str)) {
            LOG(INFO) << "Removed " << hash_str;
            return GetHash(hash_str);
        }
    }

    VI result = MakeInstruction(insty);

    CurrentInstruction()->AddOperand(arg_1);
    AddUsage(arg_1, instruction_counter_);

    if (IsEliminable(insty)) {
        InsertHash(hash_str, result);
    }

    return result;
}

VI Function::MakeInstruction(T insty, VI arg_1, VI arg_2) {
    auto hash_str = HashInstruction(insty, arg_1, arg_2);

    // Check if instruction can be removed
    if (IsEliminable(insty) &&
        // TODO: The reasoning behind this is that GlobalBase will create long
        // ranges and hence interfere with all values.
        (arg_1 != 1 || arg_2 != 1)) {
        if (HashExists(hash_str)) {
            LOG(INFO) << "Removed " << hash_str;
            return GetHash(hash_str);
        }
    }

    VI result = MakeInstruction(insty);

    CurrentInstruction()->AddOperand(arg_1);
    CurrentInstruction()->AddOperand(arg_2);

    AddUsage(arg_1, instruction_counter_);
    AddUsage(arg_2, instruction_counter_);

    if (IsEliminable(insty)) {
        InsertHash(hash_str, result);
    }

    return result;
}

// Special
// Add Instruction to front of instruction order.
VI Function::MakeInstructionFront(T insty) {
    instruction_counter_++;
    Instruction* inst = new Instruction(insty, CurrentBBIdx(), instruction_counter_);

    instruction_map_[instruction_counter_] = inst;
    instruction_order_.push_front(instruction_counter_);

    V vty = V::VAL_ANY;
    
    VI result = CreateValue(vty);
    inst->SetResult(result);

    CurrentBB()->AddInstructionFront(instruction_counter_, inst);

    return result;
}

// Special
// Make Phi Instruction
II Function::MakePhi() {
    instruction_counter_++;

    Instruction* inst = new Instruction(T::INS_PHI,
                                        CurrentBBIdx(),
                                        instruction_counter_);

    instruction_map_[instruction_counter_] = inst;
    instruction_order_.push_front(instruction_counter_);

    VI result = CreateValue(V::VAL_PHI);
    inst->SetResult(result);

    CurrentBB()->AddInstruction(instruction_counter_, inst);

    return instruction_counter_;
}

void Function::Visit(BI bb_idx, std::unordered_set<BI>& visited) {
    visited.insert(bb_idx);

    auto bb = GetBB(bb_idx);
    
    for (auto succ: bb->Successors()) {
        if (IsBackEdge(bb_idx, succ)) {
            Visit(succ, visited);
        } else if (visited.find(succ) == visited.end()) {
            Visit(succ, visited);
        }
    }

    postorder_cfg_.push_back(bb_idx);
}

// Static function?
std::vector<BI> Function::PostOrderCFG() {
    if (postorder_cfg_.size() != 0) {
        return postorder_cfg_;
    }

    std::unordered_set<BI> visited;
    // NOTE: BB with idx=1 is assumed to be the entry idx
    BI entry_idx = 1;

    // Here, we can check if there are any BBs which are not visited.
    // This is probably not going to happen since the way the IR is constructed
    // it does not allow for it.
    Visit(entry_idx, visited);

    rev_postorder_cfg_ = std::vector<BI>(postorder_cfg_.rbegin(),
                                         postorder_cfg_.rend());

    return postorder_cfg_;
}

std::vector<BI> Function::ReversePostOrderCFG() {
    if (postorder_cfg_.size() == 0) {
        PostOrderCFG();
    }

    return rev_postorder_cfg_;
}

std::vector<BI> Function::ExitBlocks() {
    return exit_blocks_;
}

Instruction* Function::GetInstruction(II ins_idx) const {
    return instruction_map_.at(ins_idx);
}

Instruction* Function::CurrentInstruction() const {
    return GetInstruction(instruction_counter_);
}

II Function::CurrentInstructionIdx() const {
    return instruction_counter_;
}

bool Function::IsActive(II ins_idx) const {
    return instruction_map_.at(ins_idx)->IsActive();
}

bool Function::IsRelational(T insty) const {
    return (insty == T::INS_BEQ ||
            insty == T::INS_BNE ||
            insty == T::INS_BLT ||
            insty == T::INS_BLE ||
            insty == T::INS_BGE);
}

bool Function::IsArithmetic(T insty) const {
    return (insty == T::INS_ADD ||
            insty == T::INS_SUB ||
            insty == T::INS_MUL ||
            insty == T::INS_DIV);
}


// Reduction functions. Try to check if we can reduce current operation at
// compile-time.
bool Function::IsReducible(VI idx_1, VI idx_2) const {
    // TODO: We should also make sure that we take into account values 
    // that are equal and not just constants
    return (GetValue(idx_1)->IsConstant() && GetValue(idx_2)->IsConstant());
}

VI Function::TryReduce(ArithmeticOperator op, VI idx_1, VI idx_2) {
    VI result = NOTFOUND;

    auto val_1 = GetValue(idx_1);
    auto val_2 = GetValue(idx_2);

    if (val_1->Type() == V::VAL_CONST &&
        val_2->Type() == V::VAL_CONST) {
    
        int c_1 = val_1->GetConstant();
        int c_2 = val_2->GetConstant();

        if (op == BINOP_MUL) {
            result = CreateConstant(c_1 * c_2);
        } else if (op == BINOP_ADD) {
            result = CreateConstant(c_1 + c_2);
        } else if (op == BINOP_SUB) {
            result = CreateConstant(c_1 - c_2);
        } else if (op == BINOP_DIV) {
            if (val_2 == 0) {
                LOG(ERROR) << "[IR] Divide by zero found!";
                exit(1);
            }
            result = CreateConstant(c_1 / c_2);
        } else {
            LOG(ERROR) << "[IR] Unknwon operation found!";
            exit(1);
        }
    } else {
        // Other cases where a constant might be generated.
        if (op == BINOP_DIV &&
            (val_1 == val_2)) {
            return CreateConstant(1);
        } else if (val_1->Type() == V::VAL_CONST &&
                   op == BINOP_MUL &&
                   val_1->GetConstant() == 1) {
            return idx_2;
        } else if (val_2->Type() == V::VAL_CONST &&
                   (op == BINOP_MUL || op == BINOP_DIV) &&
                   val_2->GetConstant() == 1) {
            return idx_1;
        }
    }

    return result;
}

int Function::ReduceCondition(RelationalOperator op, VI left, VI right) const {
    auto l = GetValue(left);
    auto r = GetValue(right);

    VI result = NOTFOUND;
    int THEN = 1;
    int ELSE = 2;

    if (l->Type() == V::VAL_CONST &&
        r->Type() == V::VAL_CONST) {

        int l_val = GetValue(left)->GetConstant();
        int r_val = GetValue(right)->GetConstant();

        switch (op) {
            case RELOP_EQ: {
                if (l_val == r_val) {
                    result = THEN;
                } else {
                    result = ELSE; 
                }
                break;
            }
            case RELOP_NEQ: {
                if (l_val != r_val) {
                    result = THEN;
                } else {
                    result = ELSE;
                }
                break;
            }
            case RELOP_LT: {
                if (l_val < r_val) {
                    result = THEN;
                } else {
                    result = ELSE;
                }
                break;
            }
            case RELOP_LTE: {
                if (l_val <= r_val) {
                    result = THEN;
                } else {
                    result = ELSE;
                }
                break;
            }
            case RELOP_GT: {
                if (l_val > r_val) {
                    result = THEN;
                } else {
                    result = ELSE;
                }
                break;
            }
            case RELOP_GTE: {
                if (l_val >= r_val) {
                    result = THEN;
                } else {
                    result = ELSE;
                }
                break;
            }
        }
    } else if (left == right) {
        if (op == RELOP_EQ ||
            op == RELOP_LTE ||
            op == RELOP_GTE) {
            result = THEN;
        } else {
            result = ELSE;
        }
    }

    return result;
}

VI Function::GetLocationValue(const std::string& var_name) const {
    return GetVariable(var_name)->GetLocationIdx();
}

VI Function::ResultForInstruction(II ins_idx) const {
    return instruction_map_.at(ins_idx)->Result();
}

void Function::AddArrContributor(II ins_idx, II result) {
    if (load_related_insts_.find(result) == load_related_insts_.end()) {
        load_related_insts_[result] = {};
    }

    load_related_insts_[result].insert(ins_idx);
}

/*
 * Function definitions for instructions
 */
Instruction::Instruction(T insty, BI containing_bb, II ins_idx) :
    ins_type_(insty),
    containing_bb_(containing_bb),
    ins_idx_(ins_idx),
    op_source_({}),
    is_active_(true) {}

void Instruction::AddOperand(VI val_idx) {
    operands_.push_back(val_idx);
}

/*
 * This function is being written so as to add the root of the read operand
 * in the phi
 */
void Instruction::AddOperand(VI val_idx, BI pred) {
    AddOperand(val_idx);
    op_source_[pred] = val_idx;
}

/*
 * Function definitions for BBs
 */
BasicBlock::BasicBlock(BI idx, B type) :
    idx_(idx),
    type_(type),
    is_sealed_(false),
    is_dead_(false),
    is_ended_(false) {}

void BasicBlock::AddPredecessor(BI pred_idx) {
    predecessors_.push_back(pred_idx);
}

void BasicBlock::AddSuccessor(BI succ_idx) {
    successors_.push_back(succ_idx);
}

void BasicBlock::AddInstruction(II idx, Instruction* inst) {
    instructions_[idx] = inst;
    
    if (inst->IsPhi()) {
        instruction_order_.push_front(idx);
    } else {
        instruction_order_.push_back(idx);
    }
}

void BasicBlock::AddInstructionFront(II idx, Instruction* inst) {
    instructions_[idx] = inst;
    
    instruction_order_.push_front(idx);
}

const std::vector<BI> BasicBlock::Predecessors() const {
    return predecessors_;
}

const std::vector<BI> BasicBlock::Successors() const {
    return successors_;
}

bool BasicBlock::HasActiveInstructions() const {
    Instruction* ins;
    for (auto ins_pair: instructions_) {
        ins = ins_pair.second;
        if (ins->IsActive()) {
            return true;
        }
    }

    return false;
}

II BasicBlock::GetPreviousInstruction(II ins_idx) const {
    auto it = std::find(instruction_order_.begin(), instruction_order_.end(), ins_idx);
    return (*it - 1);
}
