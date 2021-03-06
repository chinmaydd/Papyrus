#ifndef PAPYRUS_IGBUILDER_H
#define PAPYRUS_IGBUILDER_H

#include "Analysis/AnalysisPass.h"

#include <unordered_map>

namespace papyrus {

using V = Value::ValueType;
using ValueSet  = std::unordered_set<VI>;
using BBLiveIn  = std::unordered_map<BI, ValueSet>;
using BBLiveOut = std::unordered_map<BI, ValueSet>;
using ClusterDS = std::unordered_map<int, std::unordered_set<VI> >;
using IGMap     = std::unordered_map<VI, std::unordered_set<VI> >;

class InterferenceGraph {
public:
    InterferenceGraph();
    bool Interferes(VI, VI);

    void AddInterference(VI, VI);
    void CreateNode(VI);
    void PrintToConsole() const;
    void RegisterMerge(const std::vector<VI>&);
    void Merge();

    ClusterDS& ClusterNeighbors();
    ClusterDS& ClusterMembers();

    IGMap& IG() { return ig_; }

    VI FindRegAlias(VI);

private:
    IGMap ig_;
    // int here is the cluster ID
    // val can be a part of multiple clusters
    std::unordered_map<VI, std::unordered_set<int>> val_to_cluster_;
    ClusterDS cluster_neighbors_;
    ClusterDS cluster_members_;

    int cluster_id_;
};

class IGBuilder : public AnalysisPass {
public:
    IGBuilder(IRConstructor&);

    void Run();
    void AddInterference(VI, VI);
    void CreateNode(VI);

    InterferenceGraph& IG();

    IGMap& GetIG() { return ig_.IG(); }

private:
    std::unordered_map<std::string, BBLiveIn> live_in_vars_;
    std::unordered_map<std::string, BBLiveOut> live_out_vars_;

    void ProcessBlock(const Function*, const BasicBlock*);
    void CoalesceNodes(const Function*);
    bool RequiresReg(const Instruction*, const Value*);

    BBLiveIn bb_live_in;
    ValueSet bb_live;
    std::unordered_set<BI> visited;
    int loop_depth_;

    InterferenceGraph& ig_;
};

} // namespace papyrus

#endif /* PAPYRUS_KILLVAR_H */
