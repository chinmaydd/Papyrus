#include "ArrayLSRemover.h"

using namespace papyrus;

void ArrayLSRemover::Run() {
    bool all_pred;
    hash_val = {};
    std::unordered_set<BI> seen_once;
    std::unordered_set<II> mark_for_inactive;

    for (auto fn_pair: irc().Functions()) {
        auto fn_name = fn_pair.first;
        auto fn      = fn_pair.second;

        if (irc().IsIntrinsic(fn_name)) {
            continue;
        }

        active_defs_ = {};
        hash_val = {};

        for (auto bb_idx: fn->ReversePostOrderCFG()) {
            auto bb = fn->GetBB(bb_idx);

            auto pred = bb->Predecessors();

            // Technically, if we have only one predecessor we should not really
            // care and use the is_kill status from that BB
            //
            // Here, we can check if we encounter a KILL and use that!
            if (pred.size() > 1) {
                auto pred_1 = active_defs_[pred.at(0)];
                auto pred_2 = active_defs_[pred.at(1)];
                if (pred_1 == pred_2) {
                    active_defs_[bb_idx] = pred_1;
                } else {
                    active_defs_[bb_idx] = {};
                    for (auto elem: pred_1) {
                        if (pred_2.find(elem) != pred_2.end()) {
                            active_defs_[bb_idx].insert(elem);
                        }
                    }
                }
            } else if (bb_idx != 1) { // not entry
                active_defs_[bb_idx] = active_defs_[pred.at(0)];
            }

            mark_for_inactive = {};
            for (auto ins_idx: bb->InstructionOrder()) {
                auto ins    = fn->GetInstruction(ins_idx);
                auto type   = ins->Type();
                auto result = ins->Result();

                if (type == T::INS_LOAD) {
                    if (fn->load_hash_.find(result) != fn->load_hash_.end()) {
                        auto hash_str = fn->load_hash_[result];
                        if (active_defs_[bb_idx].find(hash_str) != active_defs_[bb_idx].end()) {
                            ins->MakeInactive();
                            fn->ReplaceUse(result, hash_val[hash_str]);
                            auto related_insts = fn->load_related_insts_;
                            if (related_insts.find(ins_idx) != related_insts.end()) {
                                for (auto inact_ins_idx: related_insts[ins_idx]) {
                                    mark_for_inactive.insert(inact_ins_idx);
                                }
                            }
                        }
                    }
                } else if (type == T::INS_STORE) {
                    auto location_val = ins->Operands().at(1);
                    auto var_name = irc().GetValue(location_val)->Identifier();
                    active_defs_[bb_idx].erase(var_name);

                    if (fn->store_hash_.find(result) != fn->store_hash_.end()) {
                        auto hash_str = fn->store_hash_[result];
                        active_defs_[bb_idx].insert(hash_str);
                        hash_val[hash_str] = ins->Operands().at(0);
                    }
                } else if (type == T::INS_KILL) {
                    // Let us first handle this
                    auto location_val = ins->Operands().at(0);
                    auto var_name = irc().GetValue(location_val)->Identifier();

                    std::string req_hash_str;
                    for (auto hash_str: active_defs_[bb_idx]) {
                        if (hash_str.rfind(var_name + "_", 0) == 0) {
                            req_hash_str = hash_str;
                        }
                    }
                    active_defs_[bb_idx].erase(req_hash_str);
                    mark_for_inactive.insert(ins_idx);
                }
            }

            for (auto inact_ins_idx: mark_for_inactive) {
                auto ins = fn->GetInstruction(inact_ins_idx);
                auto result = ins->Result();
                auto resval = fn->GetValue(result);

                if (resval->GetUsers().size() <= 1) {
                    ins->MakeInactive();
                }
            }
        }
    }
}
