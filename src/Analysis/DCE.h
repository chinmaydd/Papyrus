#ifndef PAPYRUS_DCE_H
#define PAPYRUS_DCE_H

#include "AnalysisPass.h"

namespace papyrus {

/*
 * Under construction
 */

class DCE : public AnalysisPass {
public:
    DCE(IRConstructor&);
    void Run();

private:
    void ProcessBlock(Function *fn, BasicBlock* bb);
    bool CanRemove(T);

    void ConstructReverseDominanceFrontier(const std::unordered_map<BI, BI>&);

    std::unordered_set<BI> visited;
    std::unordered_set<VI> non_dead;
    std::unordered_set<II> inactive_ins;

    std::unordered_map<BI, BI> reverse_dominance_frontier_;
};

} // namespace papyrus

#endif /* PAPYRUS_DCE_H */
