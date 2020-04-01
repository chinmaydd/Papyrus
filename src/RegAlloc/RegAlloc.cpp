#include "RegAlloc.h"

using namespace papyrus;

using C = RegAllocator::Color;

std::stringstream ConvertVecToString(const std::vector<VI>& my_vector) {
    std::stringstream result;
    std::copy(my_vector.begin(), my_vector.end(), std::ostream_iterator<VI>(result, " "));

    return result;
}

std::stringstream ConvertVecToString(const std::unordered_set<VI>& my_vector) {
    std::stringstream result;
    std::copy(my_vector.begin(), my_vector.end(), std::ostream_iterator<VI>(result, " "));

    return result;
}

std::string ColorString(C col) {
    std::string s = "UNKNOWN";
    switch(col) {
        case C::COL_WHITE:  s = "WHITE";  break;

        case C::COL_RED:    s = "RED";    break;
        case C::COL_GREEN:  s = "GREEN";  break;
        case C::COL_BLUE:   s = "BLUE";   break;
        case C::COL_YELLOW: s = "YELLOW"; break;
        case C::COL_ORANGE: s = "ORANGE"; break;
        case C::COL_CYAN:   s = "CYAN";   break;

        case C::COL_GRAY_1:   
        case C::COL_GRAY_2:   
        case C::COL_GRAY_3:   
        case C::COL_GRAY_4:   
        case C::COL_GRAY_5: s = "GRAY";   break;
    }
    return s;
}

std::unordered_map<VI, Color> RegAllocator::Coloring() const {
    return coloring_;
}

VI Function::CreateMove(BI bb_idx, VI val_idx, int reg) {
    SetCurrentBB(bb_idx);
    instruction_counter_++;

    Instruction* inst = new Instruction(T::INS_MOVE, CurrentBBIdx(), instruction_counter_);

    instruction_map_[instruction_counter_] = inst;

    ///////////////////////////////////////////
    V vty = V::VAL_ANY;
    auto val = new Value(vty);

    value_counter_ = value_map_->size() + 1;
    value_counter_++;
    value_map_->emplace(value_counter_, val);
    auto reg_idx = value_counter_;
    ///////////////////////////////////////////

    auto back_ins = GetInstruction(instruction_order_.back());
    if (back_ins->Type() == T::INS_BRA) {
        auto temp = instruction_order_.back();
        instruction_order_.pop_back();
        instruction_order_.push_back(instruction_counter_);
        instruction_order_.push_back(temp);
    } else {
        instruction_order_.push_back(instruction_counter_);
    }

    CurrentInstruction()->AddOperand(val_idx);
    CurrentInstruction()->AddOperand(reg_idx);

    AddUsage(val_idx, instruction_counter_);
    AddUsage(reg_idx, instruction_counter_);

    CurrentBB()->AddInstruction(instruction_counter_, inst);

    //////////////////////////////////////////
    vty = V::VAL_ANY;
    val = new Value(vty);

    value_counter_ = value_map_->size() + 1;
    value_counter_++;
    value_map_->emplace(value_counter_, val);
    auto result = value_counter_;
    inst->SetResult(result);
    ///////////////////////////////////////////

    return reg_idx;
}

BI Instruction::FindSource(VI val_idx) const {
    BI found = -1;
    for (auto bb_pair: op_source_) {
        if (bb_pair.second == val_idx) {
            found = bb_pair.first;
            break;
        }
    }

    return found;
}

void RegAllocator::AnnotateIR() {
    for (auto fn_pair: irc().Functions()) {
        auto fn_name = fn_pair.first;

        if (irc().IsIntrinsic(fn_name)) {
            continue;
        }

        auto fn = fn_pair.second;

        for (auto bb_idx: fn->ReversePostOrderCFG()) {
            auto bb = fn->GetBB(bb_idx);
            for (auto ins_idx: bb->InstructionOrder()) {
                auto ins = fn->GetInstruction(ins_idx);

                if (ins->Type() == T::INS_PHI &&
                    ins->IsActive()) {
                    // Phi handling

                    // Make Phi Inactive
                    ins->MakeInactive();

                    auto arg_1  = ins->Operands().at(0);
                    auto arg_2  = ins->Operands().at(1);
                    auto result = ins->Result();

                    auto c_1 = coloring_.at(arg_1);
                    auto c_2 = coloring_.at(arg_2);
                    auto c_3 = coloring_.at(result);

                    if (c_1 == c_3) {
                        auto val_1   = fn->GetValue(arg_1);
                        auto prev_bb = ins->FindSource(arg_1); // new
                        if (val_1->IsConstant()) {
                            auto reg = fn->CreateMove(prev_bb, arg_1, c_3);
                            coloring_[reg] = c_1;
                        }
                    } else if (c_1 != c_3) {
                        auto prev_bb = ins->FindSource(arg_1); // new
                        auto reg = fn->CreateMove(prev_bb, arg_1, c_3);
                        coloring_[reg] = c_3;
                    }

                    if (c_2 == c_3) {
                        auto val_2   = fn->GetValue(arg_2);
                        auto prev_bb = ins->FindSource(arg_2); // new
                        if (val_2->IsConstant()) {
                            auto reg = fn->CreateMove(prev_bb, arg_2, c_3);
                            coloring_[reg] = c_1;
                        }
                    } else if (c_2 != c_3) {
                        auto prev_bb = ins->FindSource(arg_2); // new
                        auto reg = fn->CreateMove(prev_bb, arg_2, c_3);
                        coloring_[reg] = c_3;
                    }
                } else {
                    // All of these instructions will require their
                    // operand to be in a register
                    if (ins->Type() == T::INS_RET) {
                    } else if (ins->Type() == T::INS_ARG) {
                    }
                    // For now, it is assumed that we have infinite colors.
                }
            }
        }
    }
}

// I would have ideally preferred a bitvector-based implementation since we need
// to model if a neighbor has a specific color or not. Each bit signifies whether
// the color is used up by a neighbor.
void RemoveColor(std::vector<C>& avail_colors, C col) {
  avail_colors.erase(std::remove(avail_colors.begin(), avail_colors.end(), col),
                     avail_colors.end());
}

RegAllocator::RegAllocator(IRConstructor& irc, IGBuilder& igb) :
    AnalysisPass(irc),
    igb_(igb),
    ig_map_(GetIGMap()) {}

void RegAllocator::CalculateSpillCosts() {
    long double cost;

    for (auto val_pair: ig_map_) {
        auto val_idx = val_pair.first;
        auto neighbors = val_pair.second;

        if (coloring_.find(val_idx) != coloring_.end() &&
            coloring_.at(val_idx) > NUM_REG) {

            // SpillCost calculation : 10^(loop_depth) / Degree
            auto val = irc().GetValue(val_idx);
            auto depth  = val->LoopDepth();
            auto degree = neighbors.size();

            cost = pow(10, depth) / (degree * 1.0);
            val->SetSpillCost(cost);
            
            spill_costs_.push_back({val_idx, cost});
        }
    }
}

C RegAllocator::GetColor(std::unordered_set<VI>& neighbors) {
    Color c;
    auto avail_colors(colors_);
    for (auto neighbor: neighbors) {
        if (coloring_.find(neighbor) != coloring_.end()) {
            RemoveColor(avail_colors, coloring_.at(neighbor));
        }
    }

    if (avail_colors.size() == 0) {
        LOG(ERROR) << "Too many spills!";
        exit(1);
    } else {
        c = avail_colors.front();
    }

    return c;
}

void RegAllocator::ColorIG() {
    // Let us first color clusters. The thought process behind this is that
    // the values inside a cluster are a part of the Phi and hence
    // we should color them with the same color ideally to remove it.
    for (auto cluster_pair: IG().ClusterNeighbors()) {
        auto cluster_neighbors = cluster_pair.second;

        auto c = GetColor(cluster_neighbors);
        
        auto cluster_id = cluster_pair.first;
        auto cluster_members = IG().ClusterMembers()[cluster_id];

        for (auto member: cluster_members) {
            coloring_[member] = c;
        }
    }

    for (auto val_pair: ig_map_) {
        auto val_idx = val_pair.first;

        if (coloring_.find(val_idx) == coloring_.end()) {
            auto neighbors = val_pair.second;
            auto c = GetColor(neighbors);

            coloring_[val_idx] = c;
        }
    }
}

void RegAllocator::TryColoringSpilledVals() {
    std::sort(spill_costs_.begin(), spill_costs_.end(), 
              [](CostPair& left, CostPair& right) {
        return left.second > right.second;
    });
    
    for (auto val_pair: spill_costs_) {
        auto val_idx = val_pair.first;
        auto neighbors = ig_map_.at(val_idx);

        auto c = GetColor(neighbors);
        coloring_[val_idx] = c;
    }
}

void RegAllocator::Run() {
    igb_.Run();
    ig_map_ = GetIGMap();

    if (ig_map_.size() == 0) {
        return;
    }

    ColorIG();
    CalculateSpillCosts();
    TryColoringSpilledVals();

    AnnotateIR();
}
