#ifndef PAPYRUS_CSE_H
#define PAPYRUS_CSE_H

#include "AnalysisPass.h"

namespace papyrus {

class CSE : public AnalysisPass {
public:
    CSE(IRConstructor& irc) : AnalysisPass(irc) {}
    void run();

};

} // namespace papyrus

#endif /* PAPYRUS_CSE_H */
