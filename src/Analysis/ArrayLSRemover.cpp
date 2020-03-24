#include "ArrayLSRemover.h"

using namespace papyrus;

void ArrayLSRemover::Run() {
    bool is_killed;
    bool all_pred;
    std::stack<BI> worklist = {};
    std::unordered_set<BI> seen_once;

    for (auto fn_pair: irc().Functions()) {
        auto fn_name = fn_pair.first;
        auto fn      = fn_pair.second;

        if (irc().IsIntrinsic(fn_name)) {
            continue;
        }

        is_kill_ = {};
        active_defs_ = {};

        for (auto bb_idx: fn->ReversePostOrderCFG()) {
            auto bb = fn->GetBB(bb_idx);

            is_killed = false;
            auto pred = bb->Predecessors();
            // Technically, if we have only one predecessor we should not really
            // care and use the is_kill status from that BB
            if (pred.size() > 1) {
                for (auto pred: bb->Predecessors()) {
                    is_killed |= is_kill_[pred];
                }
            }

            active_defs_[bb_idx] = {};
            if (!is_killed) {
                for (auto pred: bb->Predecessors()) {
                    active_defs_[bb_idx].insert(active_defs_[pred].begin(), 
                                                active_defs_[pred].end());
                }
            }

            is_kill_[bb_idx] = false;
            for (auto ins_idx: bb->InstructionOrder()) {
                auto ins    = fn->GetInstruction(ins_idx);
                auto type   = ins->Type();
                auto result = ins->Result();

                if (type == T::INS_LOAD) {
                    if (fn->load_hash_.find(result) != fn->load_hash_.end()) {
                        auto hash_str = fn->load_hash_[result];
                        if (active_defs_[bb_idx].find(hash_str) != active_defs_[bb_idx].end()) {

                        }
                    }
                } else if (type == T::INS_STORE) {
                    active_defs_[bb_idx] = {};
                    // This is fairly conservative. Could we make this better?
                    is_kill_[bb_idx] = true;

                    if (fn->store_hash_.find(result) != fn->store_hash_.end()) {
                        auto hash_str = fn->store_hash_[result];
                        active_defs_[bb_idx].insert(hash_str);
                    }
                }
            }
        }
    }
}
