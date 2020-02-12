#ifndef PAPYRUS_FUNCTION_H
#define PAPYRUS_FUNCTION_H

#include "Graph/Graph.h"

#include <vector>

using ValueIndex = int;
using NodeIndex = int;

namespace papyrus {
class Function {

private:
    Graph* graph_;
    ValueMap* value_map_;
    std::vector<NodeIndex> bb_indices_;

    

}


} // namespace papyrus

#endif /* PAPYRUS_FUNCTION_H */
