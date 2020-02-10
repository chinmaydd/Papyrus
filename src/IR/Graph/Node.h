#ifndef PAPYRUS_NODE_H
#define PAPYRUS_NODE_H

#include "Papyrus/Logger/Logger.h"
#include "Value.h"

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

protected:
    NodeIndex node_idx_;
    NodeType node_type_;
};

} // namespace papyrus

#endif /* PAPYRUS_NODE_H */
