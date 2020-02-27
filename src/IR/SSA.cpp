#include "IR.h"

using namespace papyrus;

#define NOTFOUND -1

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
    auto result = NOTFOUND;
    auto bb     = GetBB(bb_idx);

    if (!bb->IsSealed()) {
        SetCurrentBB(bb_idx);
        result = MakePhi();
        incomplete_phis_[bb_idx][var_name] = result;
    } else if (bb->Predecessors().size() == 1) {
        result = ReadVariable(var_name, bb->Predecessors()[0]);
    } else {
        SetCurrentBB(bb_idx);
        II phi_ins = MakePhi();
        WriteVariable(var_name, bb_idx, phi_ins);
        result = AddPhiOperands(var_name, phi_ins);
    }

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
    if (incomplete_phis_.find(bb_idx) != incomplete_phis_.end()) {
        for (auto var_map: incomplete_phis_.at(bb_idx)) {
            auto var_name = var_map.first;
            auto ins_idx  = var_map.second;

            AddPhiOperands(var_name, ins_idx);
        }
    }

    basic_block_map_.at(bb_idx)->Seal();
}
