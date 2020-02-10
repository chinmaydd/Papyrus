#ifndef PAPYRUS_IRCONSTRUCTOR_H
#define PAPYRUS_IRCONSTRUCTOR_H

#include "Papyrus/Logger/Logger.h"
#include "FrontEnd/AST.h"
#include "Graph/Graph.h"

namespace papyrus {
class IRConstructor {
public:
    IRConstructor(const ComputationNode*);

private:
    const ComputationNode* ast_root_;
};

} // namespace papyrus

#endif /* PAPYRUS_IRCONSTRUCTOR_H */
