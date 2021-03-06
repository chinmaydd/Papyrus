#ifndef PAPYRUS_ARRAYLSREMOVER_H
#define PAPYRUS_ARRAYLSREMOVER_H

#include "AnalysisPass.h"

#include <stack>

namespace papyrus {
class ArrayLSRemover : public AnalysisPass {
public:
    ArrayLSRemover(IRConstructor& irc) : AnalysisPass(irc) {}
    void Run();

private:
    std::unordered_map<BI, std::unordered_set<std::string> > active_defs_;
    std::unordered_map<BI, std::unordered_map<std::string, VI> > hash_val;
    std::unordered_map<std::string, VI> all_defs_;

    std::string LSHash(const Instruction*);


};
} // namespace papyrus

#endif /* PAPYRUS_ARRAYLSREMOVER_H */
