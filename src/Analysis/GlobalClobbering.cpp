#include "GlobalClobbering.h"

using namespace papyrus;

bool GlobalClobbering::IsMemoryStore(T insty) const {
    return (insty == T::INS_STOREG);
}

void GlobalClobbering::Clobber(const std::string& fn_name, const std::string& var_name) {
    clobbered_vars_[fn_name][var_name] = true;
}

const std::unordered_map<std::string, std::unordered_map<std::string, bool> >& GlobalClobbering::GetClobberStatus() const {
    return clobbered_vars_;
}

void GlobalClobbering::run() {
    for (auto fn_pair: irc().Functions()) {
        auto fn_name = fn_pair.first;
        if (!irc().IsIntrinsic(fn_name)) {
            auto fn = fn_pair.second;
            for (auto bb_pair: fn->BasicBlocks()) {
                auto bb = bb_pair.second;
                for (auto inst_pair: bb->Instructions()) {
                    auto inst = inst_pair.second;
                    /*
                     * Perform two major checks:
                     * 1. Check is instruction is active.
                     * 2. Check if it is a store to a global variable
                     */
                    if (inst->IsActive() &&
                        IsMemoryStore(inst->Type())) {

                        auto operands = inst->Operands();
                        VI value_idx = operands.at(1);
                        auto val = irc().GetValue(value_idx);
                        // Clobber the global variable value here.
                        Clobber(fn_name, val->Identifier());
                    }
                }
            }
        }
    }
}
