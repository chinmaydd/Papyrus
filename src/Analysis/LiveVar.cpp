#include "LiveVar.h"

using namespace papyrus;

void LiveVar::run() {
    for (auto fn_pair: irc().Functions()) {
        std::string fn_name = fn_pair.first;
        if (irc().IsIntrinsic(fn_name)) {
            continue;
        }

        auto fn = fn_pair.second;
        live_vars_[fn_name] = {};
        ValueSet live_set = {};
        VI mark_for_removal;

        for (auto bb_idx: fn->ReversePostOrderCFG()) {
            auto bb = fn->GetBB(bb_idx);
            auto ins_order = bb->InstructionOrder();

            for (auto it = ins_order.rbegin(); it != ins_order.rend(); it++) {
                auto ins_idx = *it;
                auto ins = fn->GetInstruction(ins_idx);
                if (!ins->IsActive()) {
                    continue;
                }

                live_set.erase(mark_for_removal);
                mark_for_removal = ins->Result();

                for (auto operand: ins->Operands()) {
                    if (fn->GetValue(operand)->RequiresReg()) {
                        live_set.insert(operand);
                    }
                }

                live_vars_[fn_name][ins_idx] = ValueSet(live_set);
            }
        }
    }

    LOG(ERROR) << "[ANALYSIS] LiveVar analysis done!";
}

LocalLiveVar LiveVar::LiveVarsInFunction(const std::string& func_name) const {
    return live_vars_.at(func_name);
}
