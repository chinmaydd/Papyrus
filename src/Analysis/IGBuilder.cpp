#include "IGBuilder.h"

using namespace papyrus;

InterferenceGraph::InterferenceGraph() {}

void InterferenceGraph::AddInterference(VI source, VI dest) {
    if (ig_.find(source) == ig_.end()) {
        ig_[source] = {};
    }

    if (ig_.find(dest) == ig_.end()) {
        ig_[dest] = {};
    }

    ig_[source].insert(dest);
    ig_[dest].insert(source);
}

void InterferenceGraph::PrintToConsole() const {
    for (auto node: ig_) {
        std::cout << std::to_string(node.first) << ": ";
        auto neighbors = node.second;
        for (auto neighbor: neighbors) {
            std::cout << std::to_string(neighbor) << ", ";
        }
        std::cout << std::endl;
    }
}

IGBuilder::IGBuilder(IRConstructor& irc) : 
    AnalysisPass(irc),
    ig_(*new InterferenceGraph()) {}

void IGBuilder::AddInterference(VI source, VI dest) {
    ig_.AddInterference(source, dest);
}

void PrintValueSet(const ValueSet& vs) {
    std::cout << "**********************" << std::endl;
    for (auto elem: vs) {
        std::cout << std::to_string(elem) << ", ";
    }
    std::cout << std::endl << "**********************" << std::endl;
}

void IGBuilder::run() {
    for (auto fn_pair: irc().Functions()) {
        std::string fn_name = fn_pair.first;
        if (irc().IsIntrinsic(fn_name)) {
            continue;
        }

        auto fn = fn_pair.second;

        BBLiveIn  bb_live_in = {};
        BBLiveOut bb_live_out = {};

        //////////////////////////
        ValueSet bb_live = {};
        //////////////////////////
        
        auto exit_blocks = fn->ExitBlocks();

        // Technically, we should be looking at the CFG from the perspective
        // of all exit blocks.
        for (auto bb_idx: fn->PostOrderCFG()) {
            auto bb = fn->GetBB(bb_idx);

            bb_live = {};

            auto rev_ins_order = bb->InstructionOrder();
            auto bb_from = rev_ins_order.front();
            auto bb_end  = rev_ins_order.back();
            std::reverse(rev_ins_order.begin(), rev_ins_order.end());

            for (auto succ_idx: bb->Successors()) {
                // Check if succ exists in bb_live_in
                // This should not happen since succ should be visited
                // before current block in PostOrderCFG
                auto succ = fn->GetBB(succ_idx);
                auto succ_livein = bb_live_in[succ_idx];

                // Insert all live values at beginning of successor block
                // into current.
                bb_live.insert(succ_livein.begin(), succ_livein.end());

                // Choose operands from Phis of successors
                for (auto ins_idx: succ->InstructionOrder()) {
                    auto ins = fn->GetInstruction(ins_idx);
                    if (ins->Type() == T::INS_PHI && ins->IsActive()) {
                        auto op_source = ins->OpSource();
                        if (op_source.find(bb_idx) != op_source.end()) {
                            // Insert values flowing into successor phi from 
                            // current block.
                            bb_live.insert(op_source.at(bb_idx));
                        }
                    } else if (ins->Type() != T::INS_PHI) {
                        // This means we have crossed all Phi instructions
                        break;
                    }
                }
            }

            std::cout << "after adding interference from succ" << std::to_string(bb_idx) << std::endl;
            PrintValueSet(bb_live);

            for (auto ins_idx: rev_ins_order) {
                auto ins = fn->GetInstruction(ins_idx);

                // Do not process instruction if not active
                if (!ins->IsActive()) {
                    continue;
                }

                // TODO: Run analysis as to what instruction type this is
                // TODO: Or, we can perform some analysis on what kind of a value
                // it is.

                // Output operand
                auto result = ins->Result();
                bb_live.erase(result);

                // Input operand
                for (auto op: ins->Operands()) {
                    // Remove phi from live
                    if (ins->Type() != T::INS_PHI) {
                        bb_live.insert(op);
                    }
                }

                // Add interferences for all live values
                for (auto it = bb_live.begin(); it != bb_live.end(); it++) {
                    for (auto sub_it = std::next(it); sub_it != bb_live.end(); sub_it++) {
                        AddInterference(*it, *sub_it);
                    }
                }

                PrintValueSet(bb_live);
            }

            bb_live_in[bb_idx] = bb_live;
        }

        // ig_.PrintToConsole();
        LOG(ERROR) << "DONE for " << fn_name;
    }
}
