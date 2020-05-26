#include "ArrayLSRemover.h"

using namespace papyrus;

std::string ArrayLSRemover::LSHash(const Instruction* ins) {
    auto ins_type = ins->Type();
    std::string retval = "";
    VI location;

    if (ins_type == T::INS_LOAD) {
        location = ins->Operands().at(0);
    } else {
        location = ins->Operands().at(1);
    }

    auto var = irc().GetValue(location)->Identifier();
    retval += var + "_" + std::to_string(location);

    return retval;
}

void ArrayLSRemover::Run() {
    std::unordered_map<BI, int> visited;
    std::stack<BI> worklist;
    std::unordered_set<II> mark_for_inactive;
    bool to_explore;
    BI entry_idx = 1;

    for (auto fn_pair: irc().Functions()) {
        auto fn_name = fn_pair.first;
        auto fn      = fn_pair.second;

        if (irc().IsIntrinsic(fn_name)) {
            continue;
        }

        all_defs_ = {};
        worklist = {};
        worklist.push(entry_idx);

        while (!worklist.empty()) {
            auto bb_idx = worklist.top();
            worklist.pop();

            auto bb = fn->GetBB(bb_idx);
            auto bb_type = bb->Type();

            // If bb_type is loop head, we first explore the loop body and
            // then the loop through.
            if (bb_type == B::BB_LOOPHEAD) {
                worklist.push(bb->Successors().at(0)); // through
                worklist.push(bb->Successors().at(1)); // body
            } else {
                for (auto succ: bb->Successors()) {
                    auto succ_bb = fn->GetBB(succ);
                    auto succ_type = succ_bb->Type();

                    // If successor is a loop head, we check if it has been visited -
                    // 0 time - push onto worklist and explore
                    // 1 time - push onto worklist and explore
                    // 2 times - dont explore
                    //
                    // The idea is that we need to explore it 2 times to reach 
                    // a fixed-point for analysis
                    if (succ_type == B::BB_LOOPHEAD) {
                        if (visited.find(succ) != visited.end() &&
                            visited[succ] == 2) {
                            continue;
                        } else {
                            worklist.push(succ);
                        }
                    } else {
                        worklist.push(succ);
                    }
                }
            }

            // If predecessors of a block are not explored, first wait for
            // them to get explored
            to_explore = true;
            auto pred = bb->Predecessors();
            for (auto pred_idx: bb->Predecessors()) {
                if (visited.find(pred_idx) == visited.end()) {
                    to_explore = false;
                }
            }

            if (!to_explore && bb_type != B::BB_LOOPHEAD) {
                continue;
            }

            // If this is the first loop visit of the loop head, set a flag
            // For other cases, set visited
            auto first_loop_visit = false;
            if (bb_type == B::BB_LOOPHEAD) {
                if (visited.find(bb_idx) != visited.end()) {
                    if (visited[bb_idx] == 2) {
                        continue;
                    } else {
                        visited[bb_idx] += 1;
                    }
                } else {
                    first_loop_visit = true;
                    visited[bb_idx] = 1;
                }
            } else {
                if (visited.find(bb_idx) != visited.end()) {
                    continue;
                } else {
                    visited[bb_idx] = 1;
                }
            }

            // If analyzing loop for first time, check flow from pred and use that
            // If analyzing loop for second time (after body is done) check flow
            // from pred and back edge and see if they match. If not, go through
            // the process again and
            // If analyzing loop for third time, we should have fixed point
            if (pred.size() > 1) {
                if (bb_type == B::BB_LOOPHEAD && first_loop_visit) {
                    active_defs_[bb_idx] = active_defs_[pred.at(0)];
                } else {
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
                }
            } else if (bb_idx != 1) { // not entry
                active_defs_[bb_idx] = active_defs_[pred.at(0)];
            }

            LOG(INFO) << "Exploring " << std::to_string(bb_idx);

            mark_for_inactive = {};
            for (auto ins_idx: bb->InstructionOrder()) {
                auto ins    = fn->GetInstruction(ins_idx);
                auto type   = ins->Type();
                auto result = ins->Result();

                // If we are not exploring loop for the first time, check all loads
                // and stores
                if (!first_loop_visit) {
                    if (type == T::INS_LOAD) {
                        auto hash_str = LSHash(ins);
                        if (active_defs_[bb_idx].find(hash_str) != active_defs_[bb_idx].end()) {
                            // This implies we can remove the load.
                            // Make the instruction inactive and make all dependent
                            // instructions also inactive
                            ins->MakeInactive();
                            fn->ReplaceUse(result, hash_val[hash_str]);
                            auto related_insts = fn->LoadRelatedInsts();
                            if (related_insts.find(ins_idx) != related_insts.end()) {
                                for (auto inact_ins_idx: related_insts[ins_idx]) {
                                    mark_for_inactive.insert(inact_ins_idx);
                                }
                            }
                        }
                    } else if (type == T::INS_STORE) {
                        auto location_val = ins->Operands().at(1);
                        auto var_name = irc().GetValue(location_val)->Identifier();

                        bool is_arr;
                        if (fn->IsVariableLocal(var_name)) {
                            is_arr = fn->GetVariable(var_name)->IsArray();
                        } else {
                            is_arr = irc().GetGlobal(var_name)->IsArray();
                        }

                        if (is_arr) {
                            // Remove current definitions of the variable
                            active_defs_[bb_idx].erase(var_name);

                            auto hash_str = LSHash(ins);

                            // Add current definition for future loads
                            active_defs_[bb_idx].insert(hash_str);
                            hash_val[hash_str] = ins->Operands().at(0);
                        }
                    } else if (type == T::INS_KILL) {
                        // Find the variable being killed
                        auto location_val = ins->Operands().at(0);
                        auto var_name = irc().GetValue(location_val)->Identifier();

                        // Find current active definition of the variable
                        std::string req_hash_str;
                        for (auto hash_str: active_defs_[bb_idx]) {
                            if (hash_str.rfind(var_name + "_", 0) == 0) {
                                req_hash_str = hash_str;
                            }
                        }

                        // Kill current active defintion
                        active_defs_[bb_idx].erase(req_hash_str);
                        ins->MakeInactive();
                    } else {
                        // CSE while building the SSA might not remove all
                        // redundant values. The assumption here is that while
                        // exploring the SSA again, we can remove redundant 
                        // values and hence instructions.
                        auto curr_hash = ins->HashOfInstruction();
                        auto ins_type = ins->Type();
                        if (fn->IsEliminable(ins_type) &&
                            all_defs_.find(curr_hash) != all_defs_.end() &&
                            result != all_defs_[curr_hash]) {
                            ins->MakeInactive();
                            fn->ReplaceUse(result, all_defs_[curr_hash]);
                        } else {
                            // Add current instruction and result to hashmap
                            all_defs_[curr_hash] = result;
                        }
                    }
                } else {
                    // Same as earlier stub
                    auto curr_hash = ins->HashOfInstruction();
                    auto ins_type = ins->Type();
                    if (fn->IsEliminable(ins_type) &&
                        all_defs_.find(curr_hash) != all_defs_.end() &&
                        result != all_defs_[curr_hash]) {
                        ins->MakeInactive();
                        fn->ReplaceUse(result, all_defs_[curr_hash]);
                    } else {
                        all_defs_[curr_hash] = result;
                    }
                }
            }

            // Remove all instructions marked as inactive
            for (auto inact_ins_idx: mark_for_inactive) {
                auto ins = fn->GetInstruction(inact_ins_idx);
                auto ins_type = ins->Type();
                auto result = ins->Result();
                auto resval = fn->GetValue(result);

                if (resval->GetUsers().size() == 0 &&
                    fn->IsEliminable(ins_type)) {
                    ins->MakeInactive();
                }
            }
        }
    }
}
