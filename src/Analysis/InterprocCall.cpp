#include "InterprocCall.h"

using namespace papyrus;

bool InterprocCallAnalysis::IsFunctionCall(T insty) const {
    return (insty == T::INS_CALL);
}

void InterprocCallAnalysis::run() {
    for (auto fn_pair: irc().Functions()) {
        auto fn_name = fn_pair.first;
        if (!irc().IsIntrinsic(fn_name)) {
            auto fn = fn_pair.second;

            caller_info_[fn_name] = {};
            callee_info_[fn_name] = {};

            for (auto bb_pair: fn->BasicBlocks()) {
                auto bb = bb_pair.second;
                for (auto inst_pair: bb->Instructions()) {
                    auto inst = inst_pair.second;

                    // Collect all function call information
                    if (inst->IsActive() &&
                        IsFunctionCall(inst->Type())) {

                        auto caller = fn_name;
                        VI callee_vi = inst->Operands().at(0);
                        auto callee = irc().GetValue(callee_vi)->Identifier();

                        caller_info_[callee][caller] = true;
                        callee_info_[caller][callee] = true;
                    }
                }
            }
        }
    }
}

const std::unordered_map<std::string, std::unordered_map<std::string, bool> >& InterprocCallAnalysis::GetCallerInfo() const {
    return caller_info_;
}

const std::unordered_map<std::string, std::unordered_map<std::string, bool> >& InterprocCallAnalysis::GetCalleeInfo() const {
    return callee_info_;
}
