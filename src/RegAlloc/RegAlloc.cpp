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

void RegAllocator::Run() {
   // Get IGMap from IGBuilder
   igb_.Run();
   ig_map_ = GetIGMap();

   // Well, would you look at that!
   // No registers required.
   if (ig_map_.size() == 0) {
       // This would have ideally annotated the IR
       return;
   }

   // Keep track of nodes which have been removed
   removed_nodes = {};
   while (ig_map_.size() > 1) {
       auto node_idx = GetNodeToColor();
       auto neighbors = ig_map_.at(node_idx);

       RemoveFromMap(node_idx, neighbors);
       removed_nodes.push({node_idx, neighbors});
   }

   // Neighbors of node which is left;
   auto node_idx  = (*ig_map_.begin()).first;
   auto neighbors = (*ig_map_.begin()).second;

   auto c = GetColor(neighbors);
   coloring_[node_idx] = c;

   while (!removed_nodes.empty()) {
       auto next_node_pair = removed_nodes.top();
       auto node_idx = next_node_pair.first;

       AddNodeToMap(node_idx, next_node_pair.second);

       auto neighbors = ig_map_.at(node_idx);
       auto c = GetColor(neighbors);
       coloring_[node_idx] = c;

       auto fn_name = irc().GetValue(node_idx)->Function();
       if (max_spills_.find(fn_name) == max_spills_.end()) {
           max_spills_[fn_name] = 0;
       }

       max_spills_[fn_name] = std::max(max_spills_[fn_name], c - NUM_REG);

       removed_nodes.pop();
   }

   // DEBUGGING
   for (auto col_pair: coloring_) {
       auto vi    = col_pair.first;
       auto val   = irc().GetValue(vi);
       auto color = col_pair.second;
       // LOG(ERROR) << std::to_string(vi) << ":" << ColorString(color) << ":" << val->Function();
       // if (color > NUM_REG) {
       //     LOG(ERROR) << std::to_string(vi) << ":" << ColorString(color);
       // }
    }

   LOG(ERROR) << "Done coloring";
}

VI RegAllocator::GetNodeToColor() {
    VI color_node_idx = NOTFOUND;
    for (auto node_pair: ig_map_) {
        auto node_idx = node_pair.first;
        auto neighbors = node_pair.second;

        if (neighbors.size() < NUM_REG) {
            color_node_idx = node_idx;
        }
    }

    if (color_node_idx == NOTFOUND) {
        color_node_idx = GetNodeToSpill();
    }

    return color_node_idx;
}

VI RegAllocator::GetNodeToSpill() {
    // TODO: Compute the best node to spill.
    // Let us for now assume that we spill the first node we see.
    return (*ig_map_.begin()).first;
}

void RegAllocator::AddNodeToMap(VI node_idx, std::unordered_set<VI>& neighbors) {
    ig_map_[node_idx] = {};
    for (auto neighbor: neighbors) {
        if (ig_map_.find(neighbor) != ig_map_.end()) {
            ig_map_.at(neighbor).insert(node_idx);
            ig_map_.at(node_idx).insert(neighbor);
        }
    }
}

void RegAllocator::RemoveFromMap(VI node_idx, std::unordered_set<VI>& neighbors) {
    for (auto neighbor: neighbors) {
        ig_map_.at(neighbor).erase(node_idx);
    }

    ig_map_.erase(node_idx);
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

// void RegAllocator::CalculateSpillCosts() {
//     long double cost;
// 
//     for (auto val_pair: ig_map_) {
//         auto val_idx = val_pair.first;
//         auto neighbors = val_pair.second;
// 
//         // Here, we are currently spilling only values with neigh > 6
//         if (neighbors.size() >= NUM_REG) {
//             coloring_[val_idx] = C::COL_GRAY;
// 
//             // SpillCost calculation : 10^(loop_depth) / Degree
//             auto val = irc().GetValue(val_idx);
//             auto depth  = val->LoopDepth();
//             auto degree = neighbors.size();
// 
//             cost = pow(10, depth) / (degree * 1.0);
//             val->SetSpillCost(cost);
//             
//             spill_costs_.push_back({val_idx, cost});
//         }
//     }
// }

