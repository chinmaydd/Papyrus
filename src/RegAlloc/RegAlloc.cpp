#include "RegAlloc.h"

using namespace papyrus;

RegAllocator::RegAllocator(IRConstructor& irc, IGBuilder& igb) :
    AnalysisPass(irc),
    igb_(igb) {}

void RegAllocator::CalculateSpillCosts() {
    auto ig = igb().IG();
}

void RegAllocator::ColorIG() {
    // Check if it belongs to a cluster;
    // val_to_cluster
    // Color all values to same color
    
    for (auto cluster_pair: igb().IG().ClusterNeighbors()) {
        auto cluster_id = cluster_pair.first;
        auto cluster_members = cluster_pair.second;



    }
}

void RegAllocator::Run() {
    // Should this be done on-demand?
    CalculateSpillCosts();
    Color();
}
