#ifndef PAPYRUS_INTERPROC_CALL_H
#define PAPYRUS_INTERPROC_CALL_H

#include "AnalysisPass.h"

namespace papyrus {

class InterprocCallAnalysis : public AnalysisPass {
public:
    InterprocCallAnalysis(IRConstructor& irc) : AnalysisPass(irc) {}
    void run();

    const std::unordered_map<std::string, std::unordered_map<std::string, bool> >& GetCallerInfo() const;
    const std::unordered_map<std::string, std::unordered_map<std::string, bool> >& GetCalleeInfo() const;

private:
    std::unordered_map<std::string, std::unordered_map<std::string, bool> > caller_info_;
    std::unordered_map<std::string, std::unordered_map<std::string, bool> > callee_info_;

    bool IsFunctionCall(T) const;
};

} // namespace papyrus

#endif /* PAPYRUS_INTERPROC_CALL_H */
