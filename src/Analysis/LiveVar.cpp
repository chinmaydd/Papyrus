#include "LiveVar.h"

using namespace papyrus;

void LiveVar::run() {
    for (auto fn_pair: irc().Functions()) {
        std::string fn_name = fn_pair.first;
        if (irc().IsIntrinsic(fn_name)) {
            continue;
        }

        auto fn = fn_pair.second;
        ValueSet prev_live_in  = {};
        ValueSet prev_live_out = {};
        ValueSet diff;

        BBLiveIn bb_live_in   = {};
        BBLiveOut bb_live_out = {};

        for (auto bb_idx: fn->PostOrderCFG()) {
            auto bb = fn->GetBB(bb_idx);
            auto ins_order(bb->InstructionOrder());
            std::reverse(ins_order.begin(), ins_order.end());

            LiveIn live_in   = {};
            LiveOut live_out = {};

            prev_live_in  = {};
            prev_live_out = {};

            bool at_end = true;

            for (auto ins_idx: ins_order) {
                if (at_end) {
                    at_end = false;
                    for (auto succ: bb->Sucessors()) {
                        live_out[ins_idx] = {};
                        if (bb_live_in.find(succ) != bb_live_in.end()) {
                            live_out[ins_idx].insert(bb_live_in.at(succ));
                        }
                    }
                } else {
                    // live_in[s] = use[s] U (live_out[s] - def[s])
                    // live_out[s] = U(succ) live_in[succ]
                    live_out[ins_idx] = prev_live_in;
                    live_in[ins_idx] = {};
                }

                auto ins = fn->GetInstruction(ins_idx);
                if (!ins->IsActive()) {
                    continue;
                }
                
                for (auto oper: ins->Operands()) {
                    live_in[ins_idx].insert(oper);
                }

                auto result = ins->Result();
                live_out[ins_idx].erase(result);

                prev_live_in = live_in[ins_idx];
            }

        }
    }
    LOG(ERROR) << "[ANALYSIS] LiveVar analysis done!";
}
