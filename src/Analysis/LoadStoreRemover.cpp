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

void LoadStoreRemover::run() {
    // Run GlobalClobbering Analysis
    // Run a different analysis to get this information
    GlobalClobbering gc = GlobalClobbering(irc());
    gc.run();

    global_clobber_ = gc.GetClobberStatus();
    load_deps_      = gc.GetReadDefStatus();

    auto fn = irc().GetFunction("main");

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
    auto entry_idx = 1;
    std::stack<BI> worklist;
    std::unordered_set<BI> seen_once;
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

            if (gc.IsGlobalLoad(ins_type)) {
                auto var_idx = ins->Operands().at(0);
                auto var_val = fn->GetValue(var_idx);
                auto var_name = var_val->Identifier();

                if (pseudo_globals.find(var_name) == pseudo_globals.end()) {
                    var_val->RemoveUse(ins_idx);
                    ins->MakeInactive();

                    reverse_mapping.insert({result, var_name});
                }
            } else if (gc.IsGlobalStore(ins_type)) {
                auto var_idx = ins->Operands().at(1);
                auto var_val = fn->GetValue(var_idx);
                auto var_name = var_val->Identifier();

                if (pseudo_globals.find(var_name) == pseudo_globals.end()) {
                    var_val->RemoveUse(ins_idx);
                    ins->MakeInactive();

                    fn->WriteVariable(var_name, ins->Operands().at(0));
                }
            } else {
                auto ins_oper = ins->Operands();
                for (auto oper: ins_oper) {
                    if (reverse_mapping.find(oper) != reverse_mapping.end()) {
                        auto var_val = fn->GetValue(oper);
                        auto var_name = reverse_mapping.at(oper);
                        auto replacement = fn->ReadVariable(var_name, bb_idx);

                        ins->ReplaceUse(oper, replacement);
                        var_val->RemoveUse(ins_idx);
                    }
                }
            }
        }

        if (bb_type == B::BB_LOOPHEAD) {
            if (seen_once.find(bb_idx) == seen_once.end()) {
                seen_once.insert(bb_idx);
                worklist.push(bb->Successors().at(1)); // loop_body
            } else {
                fn->SealBB(bb_idx);
                worklist.push(bb->Successors().at(0)); // through
            }
        } else {
            fn->SealBB(bb_idx);
            for (auto succ: bb->Successors()) {
                worklist.push(succ);
            }
        }
    }
}
