#ifndef PAPYRUS_REGALLOC_H
#define PAPYRUS_REGALLOC_H

#include "IGBuilder.h"

#include <stack>
#include <math.h>
#include <sstream>
#include <iterator>
#include <cassert>

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
        COL_CYAN,    // NUM_REG

        // Spills are assumed to be < 5
        COL_GRAY_1,  // representative of spill - 1
        COL_GRAY_2,  // representative of spill - 2
        COL_GRAY_3,  // representative of spill - 3
        COL_GRAY_4,  // representative of spill - 4
        COL_GRAY_5,  // representative of spill - 5
    };

    RegAllocator(IRConstructor&, IGBuilder&);
    void Run();

    std::unordered_map<VI, Color> Coloring() const;

private:
    IGBuilder& igb_;
    inline IGBuilder& igb() { return igb_; }
    inline InterferenceGraph& IG() { return igb().IG(); }
    inline IGMap& GetIGMap() { return igb().GetIG(); }

    Color GetColor(std::unordered_set<VI>&);

    void TryColoringSpilledVals();
    void CalculateSpillCosts();
    void ColorIG();
    void AnnotateIR();

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

    std::unordered_map<std::string, int> max_spills_;
};

using Color = RegAllocator::Color;

} // namespace papyrus

#endif /* PAPYRUS_REGALLOC_H */
