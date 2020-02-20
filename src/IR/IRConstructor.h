#ifndef PAPYRUS_IRCONSTRUCTOR_H
#define PAPYRUS_IRCONSTRUCTOR_H

#include "Papyrus/Logger/Logger.h"
#include "FrontEnd/ASTConstructor.h"
#include "IR.h"

namespace papyrus {
class ASTConstructor;
class IRCtxInfo;

class IRConstructor {
public:
    IRConstructor(ASTConstructor&, IRCtxInfo&);
    void construct();

    IRCtxInfo& GetIRCtxInfo() { return irctx_; }
    ASTConstructor& GetASTConst() { return astconst_; }

private:
    ASTConstructor& astconst_;
    IRCtxInfo& irctx_;
};

} // namespace papyrus

#endif /* PAPYRUS_IRCONSTRUCTOR_H */
