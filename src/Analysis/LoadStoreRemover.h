#ifndef PAPYRUS_LOADSTORE_REMOVER_H
#define PAPYRUS_LOADSTORE_REMOVER_H

#include "AnalysisPass.h"
#include "GlobalClobbering.h"

#include <queue>

namespace papyrus {

using BI = int;
using VI = int;

class LoadStoreRemover : public AnalysisPass {
public:
    LoadStoreRemover(IRConstructor& irc) : AnalysisPass(irc) {}
    void run();

private:
    bool IsMemoryStore(Instruction*) const;
    VarMap global_clobber_;
    VarMap load_deps_;
};

} // namespace papyrus

#endif /* PAPYRUS_LOADSTORE_REMOVER_H */
