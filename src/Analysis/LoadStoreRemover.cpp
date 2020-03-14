#include "LoadStoreRemover.h"

using namespace papyrus;

#define NOTFOUND -1

/*
 * Collect global variable information across calls. For each call, this function
 * returns the values read and clobbered by the function. These variables (global)
 * are then not considered for any further optimizations.
 */
std::vector<std::string> LoadStoreRemover::GlobalsUsedAcrossCall(Instruction* ins) {
    std::vector<std::string> retvec;

    auto fn_oper = ins->Operands().at(0);
    auto fn_name = irc().GetValue(fn_oper)->Identifier();

    // There should be a much better way to handle this.
    if (global_clobber_.find(fn_name) != global_clobber_.end()) {
        for (auto var_name: global_clobber_.at(fn_name)) {
            retvec.push_back(var_name);
        }
    }

    if (load_deps_.find(fn_name) != load_deps_.end()) {
        for (auto var_name: load_deps_.at(fn_name)) {
            retvec.push_back(var_name);
        }
    }

    return retvec;
}

/* 
 * Optimizing function. 
 * Currently implements constant folding and CSE
 */
void LoadStoreRemover::CheckAndReduce(Instruction* ins) {
    auto ins_type = ins->Type();

    if (fn->IsArithmetic(ins_type)) {
        auto idx_1 = ins->Operands().at(0);
        auto idx_2 = ins->Operands().at(1);

        if (fn->IsReducible(idx_1, idx_2)) {
            auto old_idx  = ins->Result();
            auto new_idx  = fn->Reduce(idx_1, idx_2, ins_type);

            fn->ReplaceUse(old_idx, new_idx);
            ins->MakeInactive();
        } else {
           auto hash_str = fn->HashInstruction(ins_type, idx_1, idx_2);

           if (hash_map_.find(hash_str) != hash_map_.end()) {
               auto old_idx = ins->Result();
               auto new_idx = hash_map_.at(hash_str);
            
               if (old_idx != new_idx) {
                   fn->ReplaceUse(old_idx, new_idx);
                   ins->MakeInactive();
               }
           } else {
               hash_map_.insert({hash_str, ins->Result()});
           }
        }
    } // TODO: Relational nodes.
}

/* 
 * Check if we can seal the current BB. According to Braun et. al, we should
 * seal a BB only if its predecessors are sealed.
 */
bool LoadStoreRemover::CanSeal(BI bb_idx) {
    auto bb = fn->GetBB(bb_idx);

    if (bb->IsSealed()) {
        return false;
    }

    for (auto pred: bb->Predecessors()) {
        if (!fn->GetBB(pred)->IsSealed()) {
            return false;
        }
    }

    return true;
}

void LoadStoreRemover::run() {
    // Run GlobalClobbering Analysis
    // Run a different analysis to get this information
    GlobalClobbering gc = GlobalClobbering(irc());
    gc.run();

    global_clobber_ = gc.GetClobberStatus();
    load_deps_      = gc.GetReadDefStatus();

    fn = irc().GetFunction("main");

    // pseudo-globals will store all variables which are sort-of
    // untouched by other function calls.
    std::unordered_set<std::string> pseudo_globals;

    for (auto bb_idx: fn->ReversePostOrderCFG()) {
        auto bb = fn->GetBB(bb_idx);
        for (auto ins_idx: bb->InstructionOrder()) {
            auto ins = fn->GetInstruction(ins_idx);
            auto type = ins->Type();

            // Technically, here we can keep track of what variables are
            // read and what variables are written to. This would allow us to
            // perform further optimizations during Register Allocation to keep
            // the register live across the call.
            if (type == T::INS_CALL) {
                auto g_call = GlobalsUsedAcrossCall(ins);
                for (auto var_name: GlobalsUsedAcrossCall(ins)) {
                    pseudo_globals.insert(var_name);
                }
            }
        }
    }

    // Now that we have all the pseudo-globals that we need, we will remove the
    // loadgs and storegs from the others. This will piggyback on SSA generation
    // used up in during the ASTWalk.
    fn->UnsealAllBB();        

    // Create a reverse mapping from result to the variable name for which the 
    // result was created
    std::unordered_map<VI, std::string> reverse_mapping;

    // Employ a worklist-style algorithm which will traverse through all the BB
    // of the graph.
    std::stack<BI> worklist = {};
    std::unordered_set<BI> seen_once;

    auto entry_idx = 1;
    worklist.push(entry_idx);

    while (!worklist.empty()) {
        auto bb_idx = worklist.top();
        worklist.pop();

        auto bb = fn->GetBB(bb_idx);
        auto bb_type = bb->Type();

        fn->SetCurrentBB(bb_idx);

        for (auto ins_idx: bb->InstructionOrder()) {
            auto ins = fn->GetInstruction(ins_idx);
            auto ins_type = ins->Type();
            auto result = ins->Result();

            if (IsGlobalLoad(ins_type)) {
                auto var_idx = ins->Operands().at(0);
                auto var_val = fn->GetValue(var_idx);
                auto var_name = var_val->Identifier();

                // For each global load, check if it is a pseudo-global.
                // In that case, dont touch it.
                // Else, introduce a reverse mapping and discard the instruction
                // which defined it.
                if (pseudo_globals.find(var_name) == pseudo_globals.end()) {
                    var_val->RemoveUse(ins_idx);
                    auto adda_idx = bb->GetAddAForLS(ins_idx);
                    auto adda = fn->GetInstruction(adda_idx);
                    if (adda->Type() == T::INS_ADDA) {
                        adda->MakeInactive();
                    }

                    ins->MakeInactive();
                    reverse_mapping.insert({result, var_name});
                }
            } else if (IsGlobalStore(ins_type)) {
                auto var_idx = ins->Operands().at(1);
                auto var_val = fn->GetValue(var_idx);
                auto var_name = var_val->Identifier();

                // For each global store, check if it is a pseudo-global.
                // In that case, dont touch it.
                // Else, WriteVariable() definition into that block. This 
                // uses the earlier SSA Generation algorithm.
                if (pseudo_globals.find(var_name) == pseudo_globals.end()) {
                    var_val->RemoveUse(ins_idx);
                    auto adda_idx = bb->GetAddAForLS(ins_idx);
                    auto adda = fn->GetInstruction(adda_idx);
                    if (adda->Type() == T::INS_ADDA) {
                        adda->MakeInactive();
                    }

                    ins->MakeInactive();
                    fn->WriteVariable(var_name, ins->Operands().at(0));
                }
            } else {
                auto ins_oper = ins->Operands();
                // For all other instructions, check if they are using the LOADG
                // generated result. In that case, replace it with the SSA-based
                // ReadVariable() output.
                for (auto oper: ins_oper) {
                    if (reverse_mapping.find(oper) != reverse_mapping.end()) {
                        auto var_val = fn->GetValue(oper);
                        auto var_name = reverse_mapping.at(oper);
                        auto replacement = fn->ReadVariable(var_name, bb_idx);

                        ins->ReplaceUse(oper, replacement);
                        var_val->RemoveUse(ins_idx);
                    }
                }

                CheckAndReduce(ins);
            }
        }


        // This is ingenious part of this hacky-algorithm. In case of loop-heads;
        // we ideally want to explore the body of the loop and seal it only
        // after we have explored the entire body. In that case, we only mark the
        // loop_head as seen once and push the body but not the fall-through.
        // Once we visit this node again, we add the through and seal this block
        // as done in the earlier SSA algorithm.
        //
        // For all other BBs, we push its successors and seal the block.
        if (bb_type == B::BB_LOOPHEAD) {
            if (seen_once.find(bb_idx) == seen_once.end()) {
                seen_once.insert(bb_idx);
                auto loop_body = bb->Successors().at(1);
                fn->SealBB(loop_body);    // loop_body
                worklist.push(loop_body); // loop_body
            } else {
                // In case we are seeing the successor a second time, 
                // we push the through block.
                auto through = bb->Successors().at(0);
                worklist.push(through); // through
            }
        } else {
            // We check if we can seal the current BB. If not, push successor
            // and move on.
            if (CanSeal(bb_idx)) {
                fn->SealBB(bb_idx);
            }

            for (auto succ: bb->Successors()) {
                worklist.push(succ);
                // We check if we can seal the successor node.
                if (CanSeal(succ)) {
                    fn->SealBB(succ);
                }
            }
        }
    }
}
