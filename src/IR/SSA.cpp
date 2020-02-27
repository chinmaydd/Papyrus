#include "IR.h"

using namespace papyrus;

#define NOTFOUND -1

void Instruction::ReplaceUse(VI replacee_idx, VI replacer_idx) {
    std::replace(operands_.begin(), operands_.end(), replacee_idx, replacer_idx);
}

void Value::RemoveUse(II ins_idx) {
    uses_.erase(std::remove(uses_.begin(), uses_.end(), ins_idx), uses_.end());
}

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
            return phi_ins;
        }
        same = op;
    }

    if (same == NOTFOUND) {
        same = CreateValue(V::VAL_ANY);
    }

    auto phi_val = GetValue(result);
    phi_val->RemoveUse(phi_ins);
    ins->MakeInactive();

    for (auto use_idx: phi_val->GetUsers()) {
        ins = GetInstruction(use_idx);
        ins->ReplaceUse(result, same);
    }

    for (auto use_idx: phi_val->GetUsers()) {
        TryRemoveTrivialPhi(use_idx);
    }

    return same;
}

bool Function::IsPhi(II ins_idx) const {
    return instruction_map_.at(ins_idx)->IsPhi();
}

VI Function::AddPhiOperands(const std::string& var_name, II phi_ins) {
    if (!IsActive(phi_ins)) {
        return NOTFOUND;
    }

    auto bb = GetBB(GetBBForInstruction(phi_ins));
    auto ins = GetInstruction(phi_ins);

    for (auto pred: bb->Predecessors()) {
        ins->AddOperand(ReadVariable(var_name, pred));
    }

    return TryRemoveTrivialPhi(phi_ins);
}

VI Function::ReadVariableRecursive(const std::string& var_name, BI bb_idx) {
    BI cur_bb_idx = CurrentBBIdx();
    VI result = NOTFOUND;
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

VI Function::ReadVariable(const std::string& var_name, BI bb_idx) {
    if (local_defs_[var_name].find(bb_idx) != local_defs_[var_name].end()) {
        return local_defs_[var_name][bb_idx];
    } else {
        return ReadVariableRecursive(var_name, bb_idx);
    }
}

void Function::WriteVariable(const std::string& var_name, BI bb_idx, VI val_idx) {
    local_defs_[var_name][bb_idx] = val_idx;
}

void Function::WriteVariable(const std::string& var_name, VI val_idx) {
    WriteVariable(var_name, CurrentBBIdx(), val_idx);
}

void Function::SealBB(BI bb_idx) {
    SetCurrentBB(bb_idx);

    Instruction *ins;
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
                // Get Value and find users
                val = GetValue(val_idx);
                for (auto user: val->GetUsers()) {
                    GetInstruction(user)->ReplaceUse(val_idx, result);
                }
            }
        }
    }

    basic_block_map_.at(bb_idx)->Seal();
}
