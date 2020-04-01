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

    std::unordered_set<BI> visited;
    std::unordered_set<VI> non_dead;
    std::unordered_set<II> inactive_ins;
};

} // namespace papyrus

#endif /* PAPYRUS_DCE_H */
