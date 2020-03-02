#ifndef PAPYRUS_ANALYSIS_PASS_H
#define PAPYRUS_ANALYSIS_PASS_H

#include "IR/IR.h"
#include "IR/IRConstructor.h"

namespace papyrus {

class AnalysisPass {
public:
    AnalysisPass(IRConstructor& irc) : irc_(irc) {}
    virtual void run() = 0;

protected:
    IRConstructor& irc_;
    inline IRConstructor& irc() { return irc_; }
};

} // namespace papyrus

#endif /* PAPYRUS_ANALYSISPASS_H */
