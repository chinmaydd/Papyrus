#ifndef PAPYRUS_LIVEVAR_H
#define PAPYRUS_LIVEVAR_H

#include "AnalysisPass.h"

#include <unordered_map>

namespace papyrus {

using ValueMap = std::unordered_map<VI, bool>;
using LocalLiveVar = std::unordered_map<II, ValueMap>;

class LiveVar : public AnalysisPass {
public:
    LiveVar(IRConstructor& irc) : AnalysisPass(irc) {}
    void run();

    LocalLiveVar LiveVarsInFunction(const std::string&) const;

private:
    std::unordered_map<std::string, LocalLiveVar> live_vars_;

};
} // namespace papyrus

#endif /* PAPYRUS_KILLVAR_H */
