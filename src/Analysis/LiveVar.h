#ifndef PAPYRUS_LIVEVAR_H
#define PAPYRUS_LIVEVAR_H

#include "AnalysisPass.h"

#include <unordered_map>

namespace papyrus {

using V = Value::ValueType;
using ValueSet = std::unordered_set<VI>;
using LiveIn  = std::unordered_map<II, ValueSet>;
using LiveOut = std::unordered_map<II, ValueSet>;
using BBLiveIn = std::unordered_map<BI, LiveIn>;
using BBLiveOut = std::unordered_map<BI, LiveOut>;

class LiveVar : public AnalysisPass {
public:
    LiveVar(IRConstructor& irc) : AnalysisPass(irc) {}
    void run();

private:
    std::unordered_map<std::string, BBLiveIn> live_in_vars_;
    std::unordered_map<std::string, BBLiveOut> live_out_vars_;
};
} // namespace papyrus

#endif /* PAPYRUS_KILLVAR_H */
