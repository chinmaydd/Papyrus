#ifndef PAPYRUS_REGALLOC_H
#define PAPYRUS_REGALLOC_H

#include "IGBuilder.h"

#include <stack>
#include <math.h>
#include <sstream>
#include <iterator>

namespace papyrus {

const int NUM_REG = 6;
using CostPair = std::pair<VI, long double>;

class RegAllocator : public AnalysisPass {
public:
    enum Color {
        COL_WHITE = 0, // equivalent of NONE

        COL_RED,
        COL_GREEN,
        COL_BLUE,
        COL_YELLOW,
        COL_ORANGE,
        COL_CYAN, // 6

        COL_GRAY_1,  // representative of spill - 1
        COL_GRAY_2,  // representative of spill - 2
        COL_GRAY_3,  // representative of spill - 2
        COL_GRAY_4,  // representative of spill - 2
        COL_GRAY_5,  // representative of spill - 2
    };

    RegAllocator(IRConstructor&, IGBuilder&);

    void Run();

private:
    IGBuilder& igb_;
    inline IGBuilder& igb() { return igb_; }
    inline InterferenceGraph& IG() { return igb().IG(); }
    inline IGMap& GetIGMap() { return igb().GetIG(); }

    Color GetColor(std::unordered_set<VI>&);

    void RemoveFromMap(VI, std::unordered_set<VI>&);
    void AddNodeToMap(VI, std::unordered_set<VI>&);

    VI GetNodeToColor();
    VI GetNodeToSpill();

    const std::vector<Color> colors_ = {
        Color::COL_RED,
        Color::COL_GREEN,
        Color::COL_BLUE,
        Color::COL_YELLOW,
        Color::COL_ORANGE,
        Color::COL_CYAN,
        Color::COL_GRAY_1,
        Color::COL_GRAY_2,
        Color::COL_GRAY_3,
        Color::COL_GRAY_4,
        Color::COL_GRAY_5,
    };

    std::vector<std::pair<VI, long double> > spill_costs_;
    std::unordered_map<VI, Color> coloring_;
    IGMap& ig_map_;

    std::stack<std::pair<VI, std::unordered_set<VI> > > removed_nodes;
    std::unordered_map<std::string, int> max_spills_;
};

} // namespace papyrus

#endif /* PAPYRUS_REGALLOC_H */
