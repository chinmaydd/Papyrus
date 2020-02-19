#include "IRConstructor.h"

using namespace papyrus;

IRConstructor::IRConstructor(ASTConstructor& astconst, IRCtxInfo& irctx) :
    astconst_(astconst),
    irctx_(irctx) {}

void IRConstructor::construct() {}
