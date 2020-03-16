#ifndef PAPYRUS_IGBUILDER_H
#define PAPYRUS_IGBUILDER_H

#include "AnalysisPass.h"

#include <unordered_map>

namespace papyrus {

using V = Value::ValueType;
using ValueSet  = std::unordered_set<VI>;
using BBLiveIn  = std::unordered_map<BI, ValueSet>;
using BBLiveOut = std::unordered_map<BI, ValueSet>;

class InterferenceGraph {
public:
    InterferenceGraph();
    void AddInterference(VI, VI);
    void PrintToConsole() const;

private:
    std::unordered_map<VI, std::unordered_set<VI> > ig_;
};

class IGBuilder : public AnalysisPass {
public:
    IGBuilder(IRConstructor&);
    void run();
    void AddInterference(VI, VI);

private:
    std::unordered_map<std::string, BBLiveIn> live_in_vars_;
    std::unordered_map<std::string, BBLiveOut> live_out_vars_;

    void ProcessBlock(Function*, BasicBlock*);

    BBLiveIn bb_live_in;
    ValueSet bb_live;
    std::unordered_set<BI> visited;
    int loop_depth_;

    InterferenceGraph& ig_;
};

} // namespace papyrus

#endif /* PAPYRUS_KILLVAR_H */
