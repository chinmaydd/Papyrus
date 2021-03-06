#include "DCE.h"

using namespace papyrus;

bool DCE::CanRemove(T insty) {
    if (insty == T::INS_READ ||
        insty == T::INS_WRITEX ||
        insty == T::INS_WRITENL ||
        insty == T::INS_CALL ||
        insty == T::INS_RET ||
        insty == T::INS_END) {
        return false;
    } else {
        return true;
    }
}

DCE::DCE(IRConstructor& irc) :
    AnalysisPass(irc) {}

void DCE::ProcessBlock(Function* fn, BasicBlock* bb) {
    auto bb_idx = bb->Idx();
    if (visited.find(bb_idx) != visited.end()) {
        // Already visited this BB
        return;
    }

    for (auto succ_idx: bb->Successors()) {
        auto succ = fn->GetBB(succ_idx);

        if (succ->Type() == B::BB_LOOPHEAD) {
            if (fn->IsBackEdge(bb_idx, succ_idx)) {
                // This implies we are at loop end
                // Back edge found. Decide later.
                continue;
            } else {
                ProcessBlock(fn, succ);
            }
        } else {
            ProcessBlock(fn, succ);
        }
    }

    auto rev_ins_order = bb->InstructionOrder();
    // LOG(ERROR) << std::to_string(bb_idx);

    if (rev_ins_order.size() != 0) {
        auto bb_from = rev_ins_order.front();
        auto bb_end  = rev_ins_order.back();
        std::reverse(rev_ins_order.begin(), rev_ins_order.end());
    }
    
    for (auto ins_idx: rev_ins_order) {
        auto ins = fn->GetInstruction(ins_idx);
        auto insty = ins->Type();
        auto result = ins->Result();

        if (CanRemove(insty) &&
            non_dead.find(result) == non_dead.end()) {
            inactive_ins.insert(ins_idx);
            ins->MakeInactive();
        } else {
            for (auto operand: ins->Operands()) {
                non_dead.insert(operand);
            }
        }
    }

    visited.insert(bb_idx);
}

void DCE::ConstructReverseDominanceFrontier(const std::unordered_map<BI, BI>& dom_f) {
    reverse_dominance_frontier_ = {};

    for (auto dom_pair: dom_f) {
        auto parent = dom_pair.first;
        auto frontier = dom_pair.second;

        reverse_dominance_frontier_[frontier] = parent;
    }
}

void DCE::Run() {
    for (auto fn_pair: irc().Functions()) {
        std::string fn_name = fn_pair.first;
        if (irc().IsIntrinsic(fn_name)) {
            continue;
        }

        auto fn = fn_pair.second;

        fn->ComputeDominatorTree();
        auto dom = fn->DominatorTree();

        fn->ComputeDominanceFrontier();
        auto dom_f = fn->DominanceFrontier();

        visited = {};

        auto entry_idx = 1;
        auto bb = fn->GetBB(entry_idx);

        ProcessBlock(fn, bb);
    }
}
