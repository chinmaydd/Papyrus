#ifndef PAPYRUS_IRCONSTRUCTOR_H
#define PAPYRUS_IRCONSTRUCTOR_H

#include "Papyrus/Logger/Logger.h"

namespace papyrus {
class IRConstructor {
public:
    IRConstructor(const ComputationNode*);

private:
    const ComputationNode* root;
};

} // namespace papyrus

#endif
