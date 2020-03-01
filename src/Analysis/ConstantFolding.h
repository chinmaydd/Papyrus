#ifndef PAPYRUS_CONSTANT_FOLDING_H
#define PAPYRUS_CONSTANT_FOLDING_H

#include "AnalysisPass.h"

namespace papyrus {

class ConstantFolding : public AnalysisPass {
public:
    ConstantFolding(IRConstructor& irc) : AnalysisPass(irc) {}
    void run();
};

} // namespace papyrus

#endif /* PAPYRUS_CONSTANT_FOLDING_H */
