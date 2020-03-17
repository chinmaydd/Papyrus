#ifndef PAPYRUS_REGALLOC_H
#define PAPYRUS_REGALLOC_H

#include "IGBuilder.h"

namespace papyrus {

const int NUM_REG = 6;

// Should this inherit from AnalysisPass?
// Technically this is a AnalysisPass
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

        COL_BLACK, // equivalent of ANY
    };

    RegAllocator(IRConstructor&, IGBuilder&);

    void Run();
    void CalculateSpillCosts();
    void ColorIG();

private:
    IGBuilder& igb_;
    inline IGBuilder& igb() { return igb_; }

    const std::vector<Color> colors_ = {
        Color::COL_RED,
        Color::COL_GREEN,
        Color::COL_BLUE,
        Color::COL_YELLOW,
        Color::COL_ORANGE,
        Color::COL_CYAN
    };

    std::unordered_map<VI, int> spill_costs_;
    std::unordered_map<VI, Color> coloring_;
};

} // namespace papyrus

#endif /* PAPYRUS_REGALLOC_H */
