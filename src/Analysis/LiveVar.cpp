#include "LiveVar.h"

using namespace papyrus;

void LiveVar::run() {
    for (auto fn_pair: irc().Functions()) {
        std::string fn_name = fn_pair.first;
        if (irc().IsIntrinsic(fn_name)) {
            continue;
        }

        auto fn = fn_pair.second;
        for (auto bb_idx: fn->ReversePostOrderCFG()) {
            auto bb = fn->GetBB(bb_idx);
            auto ins_order = bb->InstructionOrder();

            live_vars_[fn_name] = {};
            ValueMap live_set = {};

            for (auto it = ins_order.rbegin(); it != ins_order.rend(); it++) {
                auto ins_idx = *it;
                auto ins = fn->GetInstruction(ins_idx);
                auto result = ins->Result();

                live_vars_[fn_name][ins_idx] = live_set;
            }
        }
    }
}

LocalLiveVar LiveVar::LiveVarsInFunction(const std::string& func_name) const {
    return live_vars_.at(func_name);
}
