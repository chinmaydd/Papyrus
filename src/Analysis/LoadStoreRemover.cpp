#include "LoadStoreRemover.h"

using namespace papyrus;

void LoadStoreRemover::run() {
    // GlobalClobbering gc = GlobalClobbering(irc());
    // gc.run();
    // global_clobber_ = gc.GetClobberStatus();

    for (auto fn_pair: irc().Functions()) {
        auto fn_name = fn_pair.first;
        if (fn_name == "main") {
            auto fn = fn_pair.second;
            // Assumption made that BB with idx 1 is entry.
            // XXX: This could however, change.
            auto entry_bb = fn->BasicBlocks().at(1);

            std::stack<BasicBlock*> worklist;
            std::unordered_map<BI, bool> visited;
            std::unordered_map<std::string, std::pair<bool, VI> > local_clobber;
            VI result;

            worklist.push(entry_bb);

            while (!worklist.empty()) {
                auto bb = worklist.top();
                worklist.pop();

                visited[bb->Idx()] = true;
                local_clobber = {};

                for (auto ins_idx: bb->InstructionOrder()) {

                    // Check all instructions in a BB
                    auto ins = fn->GetInstruction(ins_idx);
                    if (ins->Type() == T::INS_LOADG) {
                        // If the instruction is a LOADG, we check if we have
                        // clobbered the already loaded definition of the var
                        // in the current BB. If not, we use it.

                        auto val_idx = ins->Operands().at(0);
                        auto var_name = irc().GetValue(val_idx)->Identifier();

                        // If variable is not present in the map, it means
                        // we have not yet encountered a definition we can use
                        if (local_clobber.find(var_name) == local_clobber.end()) {
                            // Save the current definition
                            local_clobber[var_name] = {false, val_idx};
                        } else {
                            auto clob_pair = local_clobber[var_name];
                            result = ins->Result();
                            
                            if (!clob_pair.first) {
                                // If variable is not yet clobbed, redirect all
                                // uses of the LOADG() to use the current loaded
                                // definition.
                                VI current_def = clob_pair.second;

                                auto val = fn->GetValue(result);

                                for (auto user: val->GetUsers()) {
                                    // TODO: Check again
                                    fn->GetInstruction(user)->ReplaceUse(result, current_def);
                                    val->RemoveUse(user);
                                }

                                // Finally, make the instruction inactive
                                ins->MakeInactive();
                            } else {
                                // This means, the variable was clobbered. Load
                                // new definition
                                local_clobber[var_name] = {false, result};
                            }
                        }
                    } else if (ins->Type() == T::INS_STOREG) {
                        auto val_idx = ins->Operands().at(1);
                        auto var_name = irc().GetValue(val_idx)->Identifier();

                        local_clobber[var_name] = {false, ins->Operands().at(0)};
                        // Assumption here is that STOREGs result is not
                        // used by anyone.
                        ins->MakeInactive();
                    } else if (ins->Type() == T::INS_CALL) {
                        // Write all.
                    }
                }

                // Write all.
                for (auto succ_idx: bb->Successors()) {
                    if (visited.find(succ_idx) == visited.end()) {
                        worklist.push(fn->BasicBlocks().at(succ_idx));
                    }
                }
            }
        }
    }
}
