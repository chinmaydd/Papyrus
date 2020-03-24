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
    std::unordered_map<BI, bool> is_kill_;
    std::unordered_map<BI, std::unordered_set<std::string> > active_defs_;
};
} // namespace papyrus

#endif /* PAPYRUS_ARRAYLSREMOVER_H */
