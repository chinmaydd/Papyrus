#ifndef PAPYRUS_LOADSTORE_REMOVER_H
#define PAPYRUS_LOADSTORE_REMOVER_H

#include "AnalysisPass.h"
#include "GlobalClobbering.h"

#include <stack>

namespace papyrus {

class LoadStoreRemover : public AnalysisPass {
public:
    LoadStoreRemover(IRConstructor& irc) : AnalysisPass(irc) {}
    void run();

private:
    std::unordered_map<std::string, bool> local_clobber_;
    std::unordered_map<std::string, std::unordered_map<std::string, bool> > global_clobber_;
};

} // namespace papyrus

#endif /* PAPYRUS_LOADSTORE_REMOVER_H */
