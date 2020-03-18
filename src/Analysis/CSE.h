#ifndef PAPYRUS_CSE_H
#define PAPYRUS_CSE_H

#include "AnalysisPass.h"

namespace papyrus {

class CSE : public AnalysisPass {
public:
    CSE(IRConstructor&);
    void Run();
};

} // namespace papyrus

#endif /* PAPYRUS_CSE_H */
