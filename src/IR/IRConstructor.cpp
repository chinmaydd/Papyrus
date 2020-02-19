#include "IRConstructor.h"

using namespace papyrus;

using ASTC   = ASTConstructor;
using IRCTX  = IRCtxInfo;
using IRC    = IRConstructor;

IRConstructor::IRConstructor(ASTC& astconst, IRCTX& irctx) :
    astconst_(astconst),
    irctx_(irctx) {}

void FunctionDeclNode::GenerateIR(IRC* irc) const {
}

void ComputationNode::GenerateIR(IRC* irc) const {
    LOG(INFO) << "[IR] Parsing functions";

    for (auto function_decl: function_declarations_) {
        function_decl->GenerateIR(irc);
    }
}

void IRConstructor::construct() {
    const ComputationNode* root = astconst_.GetRoot();
    root->GenerateIR(this);
}
