#ifndef PAPYRUS_REGALLOC_H
#define PAPYRUS_REGALLOC_H

#include "IGBuilder.h"

#include <math.h>
#include <sstream>
#include <iterator>

namespace papyrus {

const int NUM_REG = 6;
using CostPair = std::pair<VI, long double>;

class RegAllocator : public AnalysisPass {
public:
    enum Color {
        COL_WHITE, // equivalent of NONE

        COL_RED,
        COL_GREEN,
        COL_BLUE,
        COL_YELLOW,
        COL_ORANGE,
        COL_CYAN,

        COL_GRAY,  // representative of spill

        COL_BLACK, // equivalent of ANY
    };

    RegAllocator(IRConstructor&, IGBuilder&);

    void Run();

private:
    IGBuilder& igb_;
    inline IGBuilder& igb() { return igb_; }
    inline InterferenceGraph& IG() { return igb().IG(); }
    inline IGMap& GetIGMap() { return igb().GetIG(); }

    Color GetColor(std::unordered_set<VI>&);

    void TryColoringSpilledVals();
    void CalculateSpillCosts();
    void ColorIG();

    const std::vector<Color> colors_ = {
        Color::COL_RED,
        Color::COL_GREEN,
        Color::COL_BLUE,
        Color::COL_YELLOW,
        Color::COL_ORANGE,
        Color::COL_CYAN
    };

    std::vector<std::pair<VI, long double> > spill_costs_;
    std::unordered_map<VI, Color> coloring_;
    IGMap& ig_map_;
};

} // namespace papyrus

#endif /* PAPYRUS_REGALLOC_H */
