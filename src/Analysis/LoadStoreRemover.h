#ifndef PAPYRUS_LOADSTORE_REMOVER_H
#define PAPYRUS_LOADSTORE_REMOVER_H

#include "AnalysisPass.h"
#include "GlobalClobbering.h"

#include <queue>
#include <assert.h>

namespace papyrus {

using BI = int;
using VI = int;

class LoadStoreRemover : public AnalysisPass {
public:
    LoadStoreRemover(IRConstructor& irc) : 
        hash_map_({}),
        AnalysisPass(irc) {}
    void run();

private:
    std::vector<std::string> GlobalsUsedAcrossCall(Instruction*);
    void CheckAndReduce(Instruction*);
    bool CanSeal(BI);

    VarMap global_clobber_;
    VarMap load_deps_;

    std::unordered_map<std::string, VI> hash_map_;
    std::unordered_set<std::string> visited_;
    Function* fn;
    
};

} // namespace papyrus

#endif /* PAPYRUS_LOADSTORE_REMOVER_H */
