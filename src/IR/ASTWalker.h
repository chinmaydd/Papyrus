#ifndef PAPYRUS_ASTWALKER_H
#define PAPYRUS_ASTWALKER_H

#include "IRContextInfo.h"
#include "FrontEnd/AST.h"

namespace papyrus {
class ASTWalker {
    bool are_globals_declared_;
    Type current_type_;

    Type getCurrentType();
};
} // namespace papyrus

#endif
