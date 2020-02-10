#ifndef PAPYRUS_GRAPH_H
#define PAPYRUS_GRAPH_H

#include "Papyrus/Logger/Logger.h"
#include "Node.h"
#include "BasicBlock.h"

#include <unordered_map>
#include <vector>

using NodeIndex = int;

namespace papyrus {
class Graph {
public:
    NodeIndex CreateNode(NodeType);
    NodeIndex AddNode(Node*);
    Node* GetNodeFromIndex(NodeIndex);
    void RemoveNode(NodeIndex);

    void AddSuccessor(NodeIndex, NodeIndex);
    void RemoveSuccessor(NodeIndex, NodeIndex);
    bool IsSuccessor(NodeIndex, NodeIndex);

private:
    NodeIndex counter_;
    std::unordered_map<NodeIndex, Node*> nodes_;
    std::unordered_map<NodeIndex, std::vector<NodeIndex> > adj_list_;
};

} // namespace papyrus

#endif /* PAPYRUS_GRAPH_H */
