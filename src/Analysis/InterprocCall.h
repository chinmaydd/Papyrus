#ifndef PAPYRUS_INTERPROC_CALL_H
#define PAPYRUS_INTERPROC_CALL_H

#include "AnalysisPass.h"

namespace papyrus {

using VarMap = std::unordered_map<std::string, std::unordered_set<std::string> >;

/*
 * Create a topological sorted list of what function calls what. This helps us
 * develop better intuitions when creating other analysis passes which might
 * modify globals
 */

class InterprocCallAnalysis : public AnalysisPass {
public:
    InterprocCallAnalysis(IRConstructor& irc) : AnalysisPass(irc) {}
    void Run();

    const VarMap& GetCallerInfo() const;
    const VarMap& GetCalleeInfo() const;

private:
    VarMap caller_info_;
    VarMap callee_info_;

    bool IsFunctionCall(T) const;
};

} // namespace papyrus

#endif /* PAPYRUS_INTERPROC_CALL_H */
