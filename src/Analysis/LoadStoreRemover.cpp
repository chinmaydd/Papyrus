#include "LoadStoreRemover.h"

using namespace papyrus;

#define NOTFOUND -1

void LoadStoreRemover::run() {
    // Run GlobalClobbering Analysis
    GlobalClobbering gc = GlobalClobbering(irc());
    gc.run();

    auto global_clobber_ = gc.GetClobberStatus();
    auto load_deps_      = gc.GetReadDefStatus();

    for (auto fn_pair: irc().Functions()) {
        auto fn_name = fn_pair.first;
        
        if (irc().IsIntrinsic(fn_name)) {
            continue;
        }

        // TODO: Can we remove this constraint on running this analysis only 
        // for main? Technically we can since we can assume that the global
        // lives across the function boundary
        //
        // There is no reason why this should not work actually. Since each 
        // initial load in the BB is maintained
        // if (fn_name == "main") {
        auto fn = fn_pair.second;

        std::unordered_map<std::string, std::pair<bool, VI> > local_clobber;
        VI result;

        for (auto bb_idx: fn->ReversePostOrderCFG()) {
            auto bb = fn->GetBB(bb_idx);
            local_clobber = {};

            for (auto ins_idx: bb->InstructionOrder()) {
                // Check all instructions in a BB in order
                auto ins = fn->GetInstruction(ins_idx);
                if (ins->Type() == T::INS_LOADG) {
                    // If the instruction is a LOADG, we check if we have
                    // clobbered the already loaded definition of the var
                    // in the current BB. If not, we use it.
                    auto val_idx = ins->Operands().at(0);
                    auto var_name = irc().GetValue(val_idx)->Identifier();

                    // If variable is not present in the map, it means
                    // we have not yet encountered a definition we can use
                    // This means, we do have to LOADG
                    if (local_clobber.find(var_name) == local_clobber.end()) {
                        // Save the current definition
                        // Keep the LOADG instruction intact
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
                                // NOTE: Check again whether we can actually
                                // replace all the users of this instruction
                                // The reasoning behind this is that, if
                                // there was indeed any loads/stores to the 
                                // global, that would have generated new values
                                // and hence would have indeed used a different
                                // generated result.
                                fn->GetInstruction(user)->ReplaceUse(result, current_def);
                                val->RemoveUse(user);
                            }

                            // Finally, make the LOADG instruction inactive
                            ins->MakeInactive();
                        } else {
                            // This means, the variable was clobbered. Load
                            // new definition. Keep the LOADG intact
                            local_clobber[var_name] = {false, result};
                        }
                    }
                } else if (ins->Type() == T::INS_STOREG) {
                    auto val_idx = ins->Operands().at(1);
                    auto var_name = irc().GetValue(val_idx)->Identifier();

                    local_clobber[var_name] = {false, ins->Operands().at(0)};
                    // NOTE: STOREG result is not really used by anyone.
                    // It creates an unncessary value but is required to keep
                    // the IR sane (and me) sane.
                    ins->MakeInactive();
                } else if (ins->Type() == T::INS_CALL) {
                    auto val_idx = ins->Operands().at(0);
                    auto fn_name = irc().GetValue(val_idx)->Identifier();

                    // We need to make sure that before the call, we write 
                    // all regs to memory. This is conservative in that
                    // the value can live in the register. And if the register
                    // uses the value, we can just use this register instead.
                    

                    // Executed after call
                    // Take into account the kill introduced by functions
                    // on global vars.
                    for (auto clob_var: global_clobber_[fn_name]) {
                        local_clobber[clob_var] = {true, NOTFOUND};
                    }
                }
            }
        }
        //}
    }
}
