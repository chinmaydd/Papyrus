#include "IR.h"

using namespace papyrus;

#define NOTFOUND -1
            
/*
 * This module implements the SSA generation algorithm as described in
 * "Simple and Efficient Construction of the Static Single Assignment Form"
 * by Braun et. al. This is a single-pass SSA generation algorithm which does
 * not require the building of a CFG or a dominator tree beforehand. We can 
 * directly go from AST -> SSA by maintaining additional data structures
 * and information which track currently defined values and their uses.
 *
 * Refer: https://link.springer.com/chapter/10.1007/978-3-642-37051-9_6
 */

// Utility functions for SSA creation
void Instruction::ReplaceUse(VI replacee_idx, VI replacer_idx) {
    std::replace(operands_.begin(), operands_.end(), replacee_idx, replacer_idx);
}

void Value::RemoveUse(II ins_idx) {
    uses_.erase(std::remove(uses_.begin(), uses_.end(), ins_idx), uses_.end());
}

bool Function::IsPhi(II ins_idx) const {
    return instruction_map_.at(ins_idx)->IsPhi();
}

/* Remove trivial Phis. There are multiple reasons why this could happen. One
 * of which is if there are same values flowing into the BB leading to a Phi
 * with the same operands.
 */
VI Function::TryRemoveTrivialPhi(II phi_ins) {
    auto same = NOTFOUND;
    auto ins  = GetInstruction(phi_ins);

    if (!ins->IsActive()) {
        return NOTFOUND;
    }

    auto result = ins->Result();

    for (auto op: ins->Operands()) {
        if (op == same || op == result) {
            continue;
        }
        if (same != NOTFOUND) {
            // this was earlier phi_ins
            return result;
        }
        same = op;
    }

    // Check if we can remove generation of a value when it is not
    // defined.
    if (same == NOTFOUND) {
        same = CreateValue(V::VAL_ANY);
    }

    ReplaceUse(result, same);
    ins->MakeInactive();

    for (auto use_idx: GetValue(result)->GetUsers()) {
        TryRemoveTrivialPhi(use_idx);
    }

    return same;
}

void Function::ReplaceUse(VI old_idx, VI new_idx) {
    auto old_val = GetValue(old_idx);
    auto new_val = GetValue(new_idx);

    for (auto use_idx: old_val->GetUsers()) {
        auto ins = GetInstruction(use_idx);
        ins->ReplaceUse(old_idx, new_idx);
        new_val->AddUsage(use_idx);
    }
}

/*
 * Here, we try and query predecessors of a block for definitions of a variable.
 * If found, we add it as an operand to the Phi
 *
 * Eventually, we then try and remove trivial Phis.
 */
VI Function::AddPhiOperands(const std::string& var_name, II phi_ins) {
    if (!IsActive(phi_ins)) {
        return NOTFOUND;
    }

    auto bb = GetBB(GetBBForInstruction(phi_ins));
    auto ins = GetInstruction(phi_ins);

    for (auto pred: bb->Predecessors()) {
        ins->AddOperand(ReadVariable(var_name, pred), pred);
    }

    return TryRemoveTrivialPhi(phi_ins);
}

/*
 * This function places the Phis which are responsible for maintaining
 * the SSA property of the IR. There are 3 major cases to handle here:
 *
 * 1. If the block is not sealed, we create an incomplete phi and assume
 *    that we will be using the result generated due to this as the 
 *    variable definition in that block.
 * 2. If the block is sealed and there is only one predecessor, we could
 *    query for the definition in the predecessor since that will flow
 *    into this block as well.
 * 3. Otherwise, it implies that there are multiple predecessors. For this
 *    case, we create a Phi and try and add operands to it based on the 
 *    knowledge of its predecessors.
 *
 * Finally, we write the result generated as a result of either of the above
 * operations and use it as the definition of the variable for that block.
 */
VI Function::ReadVariableRecursive(const std::string& var_name, BI bb_idx) {
    BI cur_bb_idx = CurrentBBIdx();
    VI result  = NOTFOUND;
    BI phi_ins = NOTFOUND;
    auto bb    = GetBB(bb_idx);

    if (!bb->IsSealed()) {
        SetCurrentBB(bb_idx);
        phi_ins = MakePhi();
        result = ResultForInstruction(phi_ins);
        incomplete_phis_[bb_idx][var_name] = phi_ins;
    } else if (bb->Predecessors().size() == 1) {
        result = ReadVariable(var_name, bb->Predecessors()[0]);
    } else {
        SetCurrentBB(bb_idx);
        phi_ins = MakePhi();
        result = ResultForInstruction(phi_ins);
        WriteVariable(var_name, bb_idx, result);
        result = AddPhiOperands(var_name, phi_ins);
    }

    SetCurrentBB(cur_bb_idx);
    WriteVariable(var_name, bb_idx, result);
    return result;
}

/*
 * ReadVariable is the "main" sort-of function for reading values which 
 * are being used in expressions. If a definition of the variable is not
 * found in the current block, ReadVariableRecursive() is called to pull
 * up definitions from its predecessors.
 */
VI Function::ReadVariable(const std::string& var_name, BI bb_idx) {
    if (local_defs_[var_name].find(bb_idx) != local_defs_[var_name].end()) {
        return local_defs_[var_name][bb_idx];
    } else {
        return ReadVariableRecursive(var_name, bb_idx);
    }
}

/*
 * The WriteVariable operation is pretty straightforward. We rewrite the
 * current definition of the variable in the block to be the new value
 */
void Function::WriteVariable(const std::string& var_name, VI val_idx) {
    WriteVariable(var_name, CurrentBBIdx(), val_idx);
}

void Function::WriteVariable(const std::string& var_name, BI bb_idx, VI val_idx) {
    local_defs_[var_name][bb_idx] = val_idx;
}

/*
 * Sealing a BB is an explicit action to be carried out when all the 
 * predecessors of a particular BB are added. This means there are no
 * more predecessors which "could" be potentially added later. On sealing
 * the BB, we search in the predecessors for the values flowing in and
 * fill up the incomplete Phi instructions added at the beginning of the BB.
 * 
 * Later, we also remove Phis which might have become trivial.
 *
 * Finally, we redirect the use of all values involved in the Phi as operands
 * and use the Phi instead
 */
void Function::SealBB(BI bb_idx) {
    SetCurrentBB(bb_idx);

    Instruction *ins, *user_ins;
    Value* val;

    if (incomplete_phis_.find(bb_idx) != incomplete_phis_.end()) {
        for (auto var_map: incomplete_phis_.at(bb_idx)) {
            auto var_name = var_map.first;
            auto ins_idx  = var_map.second;

            AddPhiOperands(var_name, ins_idx);

            ins = GetInstruction(ins_idx);
            if (!ins->IsActive()) {
                continue;
            }

            auto result = ins->Result();
            for (auto val_idx: ins->Operands()) {
                // Find the users of the particular value
                // and redirect their use to be that of the Phi instead
                // NOTE: This is to be done for values in the block
                // under consideration since otherwise, it could
                // potentially lead to other issues.
                val = GetValue(val_idx);
                for (auto user: val->GetUsers()) {
                    user_ins = GetInstruction(user);
                    if (user_ins->ContainingBB() == bb_idx) {
                        GetInstruction(user)->ReplaceUse(val_idx, result);
                        val->RemoveUse(user);
                    }
                }
            }
        }
    }

    basic_block_map_.at(bb_idx)->Seal();
}

/*
 * This function is introduced to run the SSA Pass once again for 
 * global vars.
 */
void Function::UnsealAllBB() {
    for (auto bb_idx: PostOrderCFG()) {
        GetBB(bb_idx)->Unseal();
    }
}
