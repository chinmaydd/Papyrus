#include "GlobalClobbering.h"

using namespace papyrus;

GlobalClobbering::GlobalClobbering(IRConstructor& irc) :
    AnalysisPass(irc) {}

void GlobalClobbering::Clobber(const std::string& fn_name, const std::string& var_name) {
    clobbered_vars_[fn_name].insert(var_name);
}

void GlobalClobbering::ReadDef(const std::string& fn_name, const std::string& var_name) {
    read_vars_[fn_name].insert(var_name);
}

const VarMap& GlobalClobbering::GetClobberStatus() const {
    return clobbered_vars_;
}

const VarMap& GlobalClobbering::GetReadDefStatus() const {
    return read_vars_;
}

void GlobalClobbering::Visit(const std::string& fn_name) {
    visited_.insert(fn_name);

    for (auto callee: callee_info_[fn_name]) {
        if (visited_.find(callee) == visited_.end()) {
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

            if (inst->IsActive() && IsGlobalStore(inst->Type())) {
                // Perform two major checks:
                // 1. Check if instruction is active.
                // 2. Check if it is a store to a global variable
                auto operands = inst->Operands();
                auto value_idx = operands.at(1);
                auto val = irc().GetValue(value_idx);

                // Clobber the global variable value here.
                Clobber(fn_name, val->Identifier());
            } else if (inst->IsActive() && IsGlobalLoad(inst->Type())) {
                // 1. Check if instruction is active
                // 2. Check if it is a load from a global variable
                auto operands = inst->Operands();
                auto value_idx = operands.at(0);
                auto val = irc().GetValue(value_idx);

                auto ident = val->Identifier();
                if (ident != "") {
                    // Add ReadDef; 
                    // NOTE: ident is empty when formal params are involved.
                    ReadDef(fn_name, ident);
                }
            }
        }
    }

     // Here, the assumption is that we would have already seen
     // all the potential callee's of this function. This can now 
     // allow us to merge global clobbering information into that 
     // of the current function
     for (auto callee: callee_info_[fn_name]) {
         if (clobbered_vars_.find(callee) != clobbered_vars_.end()) {
             for (auto clob_var: clobbered_vars_[callee]) {
                 clobbered_vars_[fn_name].insert(clob_var);
             }
         }

         if (read_vars_.find(callee) != read_vars_.end()) {
             for (auto read_var: read_vars_[callee]) {
                 read_vars_[fn_name].insert(read_var);
             }
         }
     }
}

void GlobalClobbering::Run() {

    InterprocCallAnalysis ipc(irc());
    ipc.Run();
    callee_info_ = ipc.GetCalleeInfo();

    // Topological Sort
    // Visit all callee's of a function before you visit itself
    for (auto fn_pair: irc().Functions()) {
        auto fn_name = fn_pair.first;
        if (irc().IsIntrinsic(fn_name) || fn_name == "main") {
            continue;
        } 

        if (visited_.find(fn_name) == visited_.end()) {
            Visit(fn_name);
        }
    }
}
