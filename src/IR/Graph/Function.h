#ifndef PAPYRUS_FUNCTION_H
#define PAPYRUS_FUNCTION_H

#include "Graph/Graph.h"

#include <vector>

using BBIndex = int;

namespace papyrus {
class Function {

private:
    Graph* graph_;
    std::vector<BBIndex> bb_indices_;
    

}


} // namespace papyrus

#endif /* PAPYRUS_FUNCTION_H */
