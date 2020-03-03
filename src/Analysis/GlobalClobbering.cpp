#include "GlobalClobbering.h"

using namespace papyrus;

GlobalClobbering::GlobalClobbering(IRConstructor& irc) :
    AnalysisPass(irc) {

    InterprocCallAnalysis ipc(irc);
    ipc.run();
    callee_info_ = ipc.GetCalleeInfo();
}

bool GlobalClobbering::IsMemoryStore(T insty) const {
    return (insty == T::INS_STOREG);
}

bool GlobalClobbering::IsMemoryLoad(T insty) const {
    return (insty == T::INS_LOADG);
}

bool GlobalClobbering::IsFunctionCall(T insty) const {
    return (insty == T::INS_CALL);
}

void GlobalClobbering::Clobber(const std::string& fn_name, const std::string& var_name) {
    clobbered_vars_[fn_name][var_name] = true;
}

void GlobalClobbering::ReadDef(const std::string& fn_name, const std::string& var_name) {
    read_vars_[fn_name][var_name] = true;
}

const std::unordered_map<std::string, std::unordered_map<std::string, bool> >& GlobalClobbering::GetClobberStatus() const {
    return clobbered_vars_;
}

const std::unordered_map<std::string, std::unordered_map<std::string, bool> >& GlobalClobbering::GetReadDefStatus() const {
    return read_vars_;
}

void GlobalClobbering::Visit(const std::string& fn_name) {
    visited_[fn_name] = true;

    for (auto callee_pair: callee_info_[fn_name]) {
        auto callee = callee_pair.first;
        if (!visited_[callee]) {
            Visit(callee);
        }
    }

    auto fn = irc().GetFunction(fn_name);
    // Traverse the function control flow in reverse postorder
    for (auto bb_idx: fn->ReversePostOrderCFG()) {
        auto bb = fn->GetBB(bb_idx);
        // InstructionOrder() returns instructions inside the BB in linear order
        for (auto ins_idx: bb->InstructionOrder()) {
            auto inst = fn->GetInstruction(ins_idx);

            if (inst->IsActive() && IsMemoryStore(inst->Type())) {
                // Perform two major checks:
                // 1. Check if instruction is active.
                // 2. Check if it is a store to a global variable
                auto operands = inst->Operands();
                VI value_idx = operands.at(1);
                auto val = irc().GetValue(value_idx);

                // Clobber the global variable value here.
                Clobber(fn_name, val->Identifier());
            } else if (inst->IsActive() && IsMemoryLoad(inst->Type())) {
                // 1. Check if instruction is active
                // 2. Check if it is a load from a global variable
                auto operands = inst->Operands();
                VI value_idx = operands.at(0);
                auto val = irc().GetValue(value_idx);

                // Add ReadDef
                ReadDef(fn_name, val->Identifier());
            }
        }
    }

     // Here, the assumption is that we would have already seen
     // all the potential callee's of this function. This can now 
     // allow us to merge global clobbering information into that 
     // of the current function
     for (auto callee_pair: callee_info_[fn_name]) {
         auto callee = callee_pair.first;

         if (clobbered_vars_.find(callee) != clobbered_vars_.end()) {
             for (auto clob_pair: clobbered_vars_[callee]) {
                 clobbered_vars_[fn_name][clob_pair.first] = true;
             }
         }

         if (read_vars_.find(callee) != read_vars_.end()) {
             for (auto read_pair: read_vars_[callee]) {
                 read_vars_[fn_name][read_pair.first] = true;
             }
         }
     }
}

void GlobalClobbering::run() {

    for (auto fn_pair: irc().Functions()) {
        auto fn_name = fn_pair.first;
        if (!irc().IsIntrinsic(fn_name)) {
            visited_[fn_name] = false;
        }
    }

    // Topological Sort
    // Visit all callee's of a function before you visit itself
    for (auto fn_pair: irc().Functions()) {
        auto fn_name = fn_pair.first;
        if (!irc().IsIntrinsic(fn_name)) {
            if (!visited_[fn_name]) {
                Visit(fn_name);
            }
        }
    }
}
