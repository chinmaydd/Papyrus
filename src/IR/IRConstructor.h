#ifndef PAPYRUS_IRCONSTRUCTOR_H
#define PAPYRUS_IRCONSTRUCTOR_H

#include "Papyrus/Logger/Logger.h"
#include "FrontEnd/ASTConstructor.h"
#include "Graph.h"

namespace papyrus {
class IRConstructor {
public:
    IRConstructor(ASTConstructor&, IRCtxInfo&);
    void construct();

private:
    ASTConstructor& astconst_;
    IRCtxInfo& irctx_;
};

} // namespace papyrus

#endif /* PAPYRUS_IRCONSTRUCTOR_H */
