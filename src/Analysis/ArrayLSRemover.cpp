#include "ArrayLSRemover.h"

using namespace papyrus;


void ArrayLSRemover::Recurse(Function* fn, Instruction *ins) {
    // TODO: Fix this!
}

void ArrayLSRemover::Run() {
    auto fn = irc().GetFunction("main");

    for (auto bb_idx: fn->ReversePostOrderCFG()) {
        auto bb = fn->GetBB(bb_idx);
        hash_val_ = {};
        kill_status_ = {};
        live_ = {};

        for (auto ins_idx: bb->InstructionOrder()) {
            auto ins = fn->GetInstruction(ins_idx);

            if (ins->Type() == T::INS_STORE) {
                auto result  = ins->Result();
                auto loc_idx = ins->Operands().at(1);
                auto ident = fn->GetValue(loc_idx)->Identifier();

                if (fn->store_hash_.find(result) != fn->store_hash_.end()) {
                    hash_val_[ident] = fn->store_hash_[result];
                    live_[fn->store_hash_[result]] = result;
                    kill_status_[ident] = false;
                }
            } else if (ins->Type() == T::INS_LOAD) {
                auto result = ins->Result();
                auto loc_idx = ins->Operands().at(0);
                auto ident = fn->GetValue(loc_idx)->Identifier();

                if (kill_status_.find(ident) != kill_status_.end() &&
                    !kill_status_[ident]) {

                    if (fn->load_hash_.find(result) != fn->load_hash_.end()) {
                        auto temp_hash = fn->load_hash_[result];

                        if (hash_val_.find(ident) != hash_val_.end()) {
                            if (temp_hash == hash_val_[ident]) {
                                // match
                                ins->MakeInactive();
                                Recurse(fn, ins);
                                fn->ReplaceUse(result, live_[temp_hash]);
                            }
                        }
                    }
                }
            } else if (ins->Type() == T::INS_CALL) {
                kill_status_ = {};
                hash_val_ = {};
                live_ = {};
            }
        }
    }
}
