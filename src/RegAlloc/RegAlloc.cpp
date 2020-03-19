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
        case C::COL_RED:    s = "RED";    break;
        case C::COL_GREEN:  s = "GREEN";  break;
        case C::COL_BLUE:   s = "BLUE";   break;
        case C::COL_YELLOW: s = "YELLOW"; break;
        case C::COL_ORANGE: s = "ORANGE"; break;
        case C::COL_CYAN:   s = "CYAN";   break;
        case C::COL_GRAY:   s = "GRAY";   break;
    }
    return s;
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

        // Here, we are currently spilling all of the values.
        if (neighbors.size() >= NUM_REG) {
            coloring_[val_idx] = C::COL_GRAY;

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
        c = C::COL_GRAY;
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

        // LOG(ERROR) << ConvertVecToString(cluster_members).str();

        for (auto member: cluster_members) {
            coloring_[member] = c;
        }
    }

    for (auto val_pair: ig_map_) {
        auto neighbors = val_pair.second;
        auto c = GetColor(neighbors);

        auto val_idx = val_pair.first;
        coloring_[val_idx] = c;
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

    // Now that we know what to spill, we need to introduce relevant 
    // instructions.
}

void RegAllocator::Run() {
    ColorIG();
    CalculateSpillCosts();
    TryColoringSpilledVals();

    for (auto col_pair: coloring_) {
        auto vi    = col_pair.first;
        auto color = col_pair.second;

        // LOG(ERROR) << std::to_string(vi) << ":" << ColorString(color);
        if (color == C::COL_GRAY) {
            LOG(ERROR) << "Value: " << std::to_string(vi);
        }
    }
}
