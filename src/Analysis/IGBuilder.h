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
    bool Interferes(VI, VI);

    void AddInterference(VI, VI);
    void PrintToConsole() const;
    void RegisterMerge(const std::vector<VI>&);
    void Merge();

    VI FindRegAlias(VI);

private:
    std::unordered_map<VI, std::unordered_set<VI> > ig_;
    // int here is the cluster ID
    std::unordered_map<VI, int> val_to_cluster_;
    std::unordered_map<int, std::unordered_set<VI> > cluster_neighbors_;

    int cluster_id_;
};

class IGBuilder : public AnalysisPass {
public:
    IGBuilder(IRConstructor&);
    void run();
    void AddInterference(VI, VI);
    InterferenceGraph& IG();

private:
    std::unordered_map<std::string, BBLiveIn> live_in_vars_;
    std::unordered_map<std::string, BBLiveOut> live_out_vars_;

    void ProcessBlock(Function*, BasicBlock*);
    void CoalesceNodes(Function*);

    BBLiveIn bb_live_in;
    ValueSet bb_live;
    std::unordered_set<BI> visited;
    int loop_depth_;

    InterferenceGraph& ig_;
};

} // namespace papyrus

#endif /* PAPYRUS_KILLVAR_H */
