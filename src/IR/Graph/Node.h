#ifndef PAPYRUS_NODE_H
#define PAPYRUS_NODE_H

#include "Papyrus/Logger/Logger.h"
#include "Value.h"

using ValueIndex = int;
using NodeIndex = int;

class Node {
private:
    NodeData* data;
    NodeType* type;
    NodeIdx predecessors;
    NodeIdx successors;
}




























namespace papyrus {
class Node {
public:
    enum NodeType {
        NODE_ROOT,
        NODE_BB,
        NODE_OP,
        NODE_PHI,
        NODE_UNREACHABLE,
        NODE_REMOVED
    };
    Node(NodeType);

    const NodeType GetNodeType() const { return node_type_; }
    const NodeIndex GetNodeIndex() const { return node_idx_; }

private:
    NodeIndex node_idx_;
    NodeType node_type_;
    std::vector<ValueIndex> operands_;
    NodeIndex bb_idx_;
    std::vector<NodeIndex> successors_;
    std::vector<NodeIndex> predecessors_;
};

} // namespace papyrus

#endif /* PAPYRUS_NODE_H */
