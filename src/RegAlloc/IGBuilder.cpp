#include "IGBuilder.h"

using namespace papyrus;

// Helper to print out valueset to console
void PrintValueSet(const ValueSet& vs) {
    std::cout << "**********************" << std::endl;
    for (auto elem: vs) {
        std::cout << std::to_string(elem) << ", ";
    }
    std::cout << std::endl << "**********************" << std::endl;
}

/*
 * Method definitions for InterferenceGraph
 */
InterferenceGraph::InterferenceGraph() :
    cluster_id_(0) {}

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

bool InterferenceGraph::Interferes(VI arg_1, VI arg_2) {
    // Maybe there should be a default "add" mode
    if (ig_.find(arg_1) == ig_.end()) {
        ig_[arg_1] = {};
    }

    if (ig_.find(arg_2) == ig_.end()) {
        ig_[arg_2] = {};
    }

    return (ig_.at(arg_1).find(arg_2) != ig_.at(arg_1).end());
}

void InterferenceGraph::RegisterMerge(const std::vector<VI>& values) {
    // We have to merge all the operands in the Phi
    std::unordered_set<VI> neighbors;
    cluster_id_++;

    for (auto val: values) {
        if (val_to_cluster_.find(val) == val_to_cluster_.end()) {
            val_to_cluster_[val] = {};
        }

        val_to_cluster_[val].insert(cluster_id_);
        cluster_members_[cluster_id_].insert(val);
    }
}

void InterferenceGraph::Merge() {
    for (auto val_pair: ig_) {
        auto val_idx   = val_pair.first;
        auto neighbors = val_pair.second;
        if (val_to_cluster_.find(val_idx) != val_to_cluster_.end()) {
            // It is part of atleast a cluster
            auto cluster_ids = val_to_cluster_.at(val_idx);
            for (auto cluster_id: cluster_ids) {
                for (auto neighbor: neighbors) {
                    cluster_neighbors_[cluster_id].insert(neighbor);
                }
            }
        } 

        // TODO: Add else. This is the condition when the value is not part
        // of any cluster. This implies that it is probably not involved
        // in a phi. We take those values into account; but only after we have
        // colored all of those in the cluster because that is how Phi works;
        // We will have to color all nodes involved in a Phi with the same
        // color, allowing us to eliminate the it.
    }
}

ClusterDS& InterferenceGraph::ClusterNeighbors() {
    return cluster_neighbors_;
}

ClusterDS& InterferenceGraph::ClusterMembers() {
    return cluster_members_;
}

/*
 * Method definitions for IGBuilder
 */
IGBuilder::IGBuilder(IRConstructor& irc) : 
    AnalysisPass(irc),
    loop_depth_(0),
    ig_(*new InterferenceGraph()) {}

void IGBuilder::AddInterference(VI source, VI dest) {
    ig_.AddInterference(source, dest);
}

InterferenceGraph& IGBuilder::IG() {
    return ig_;
}

void IGBuilder::CoalesceNodes(Function* fn) {
    for (auto bb_pair: fn->BasicBlocks()) {
        auto bb_idx = bb_pair.first;
        auto bb = bb_pair.second;

        for (auto ins_idx: bb->InstructionOrder()) {
            auto ins = fn->GetInstruction(ins_idx);

            if (!ins->IsActive()) {
                continue;
            }

            if (ins->Type() == T::INS_PHI) {
                auto result = ins->Result();
                auto arg_1  = ins->Operands().at(0);
                auto arg_2  = ins->Operands().at(1);

                if (!ig_.Interferes(arg_1, result) &&
                    !ig_.Interferes(arg_2, result) &&
                    !ig_.Interferes(arg_1, arg_2)) {
                    ig_.RegisterMerge({arg_1, arg_2, result});
                } else {
                    LOG(ERROR) << std::to_string(arg_1) << ":" << std::to_string(arg_2);
                }
            } else {
                // We would have seen all phis till now
                break;
            }

            // We should also check for coalescing nodes which are not
            // really dependent on each other. This however is more costly 
            // and will be handled by graph coloring?
            //
            // The literature on this suggested to merge non-interfering nodes
            // if the resultant node had a degree of < k.
            //
            // This it to be implemented at a later time.
        }
    }

    ig_.Merge();
}

void IGBuilder::ProcessBlock(Function* fn, BasicBlock* bb) {
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
                loop_depth_++;
                ProcessBlock(fn, succ);
                loop_depth_--;
            }
        } else {
            ProcessBlock(fn, succ);
        }
    }

    bb_live = {};

    auto rev_ins_order = bb->InstructionOrder();

    if (rev_ins_order.size() != 0) {
        auto bb_from = rev_ins_order.front();
        auto bb_end  = rev_ins_order.back();
        std::reverse(rev_ins_order.begin(), rev_ins_order.end());
    }

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
                    auto val_idx = op_source.at(bb_idx);
                    auto val = fn->GetValue(val_idx);

                    if (val->Type() == V::VAL_CONST) {
                        // TODO: Decide what to do here.
                    } else {
                        bb_live.insert(op_source.at(bb_idx));
                    }
                }
            } else if (ins->Type() != T::INS_PHI) {
                // This means we have crossed all Phi instructions
                break;
            }
        }
    }

    for (auto ins_idx: rev_ins_order) {
        auto ins = fn->GetInstruction(ins_idx);

        // Do not process instruction if not active
        if (!ins->IsActive()) {
            continue;
        }

        // Output operand
        auto result = ins->Result();

        // Add depth to the result
        fn->GetValue(result)->SetDepth(loop_depth_);
        bb_live.erase(result);

        if (ins->Type() != T::INS_PHI) {
            for (auto op: ins->Operands()) {
                auto val = fn->GetValue(op);

                // The question is -> how to handle constants. It is extremely
                // cheap to spill constants since we need only one instruction
                // to load them back into a register. Let us assume that for 
                // now and load them such that when we scan the successors and pull
                // the value from the join node; we add a "move val register" 
                // to the end of the BB
                //
                // The other issue with constants is that if left alone, they start
                // interfering with every other value.
                if (val->Type() != V::VAL_BRANCH ||
                    val->Type() != V::VAL_GLOBALBASE ||
                    val->Type() != V::VAL_LOCALBASE ||
                    val->Type() != V::VAL_FUNC) {
                    bb_live.insert(op);
                }
            }
        }

        // Add interferences for all live values
        for (auto it = bb_live.begin(); it != bb_live.end(); it++) {
            for (auto sub_it = std::next(it); sub_it != bb_live.end(); sub_it++) {
                AddInterference(*it, *sub_it);
            }
        }
    }

    bb_live_in[bb_idx] = bb_live;
    visited.insert(bb_idx);
}

void IGBuilder::Run() {
    for (auto fn_pair: irc().Functions()) {
        std::string fn_name = fn_pair.first;
        if (irc().IsIntrinsic(fn_name)) {
            continue;
        }

        auto fn = fn_pair.second;
        visited = {};

        //////////////////////////
        bb_live = {};
        bb_live_in = {};
        //////////////////////////
        
        auto entry_idx = 1;
        auto bb = fn->GetBB(entry_idx);

        ProcessBlock(fn, bb);

        live_in_vars_[fn_name] = bb_live_in;

        CoalesceNodes(fn);
    }

    LOG(ERROR) << "IG Built";
}
