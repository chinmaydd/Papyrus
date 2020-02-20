#include "IRConstructor.h"

using namespace papyrus;

using ASTC   = ASTConstructor;
using IRCTX  = IRCtxInfo;
using IRC    = IRConstructor;

using ValueIndex = int;

IRConstructor::IRConstructor(ASTC& astconst, IRCTX& irctx) :
    astconst_(astconst),
    irctx_(irctx) {}

ValueIndex ExpressionNode::GenerateIR(IRC* irc) const {
    return -1;
}
    
ValueIndex DesignatorNode::GenerateIR(IRC* irc) const {
    return -1;
}

void AssignmentNode::GenerateIR(IRC* irc) const {
    ValueIndex expr_idx = value_->GenerateIR(irc);

    ValueIndex mem_location = designator_->GenerateIR(irc);

    // irc->GetIRCtxInfo().AddInstruction(InstructionType::INS_STORE, expr_idx, mem_location);
}

void StatementNode::GenerateIR(IRC* irc) const {
    switch(statement_type_) {
        case StatementType::STAT_ASSIGN: {
            const AssignmentNode* assgn = static_cast<const AssignmentNode*>(this);
            assgn->GenerateIR(irc);
        }
        default: {
        }
    }
}

void FunctionBodyNode::GenerateIR(IRC* irc) const {
    LOG(INFO) << "[IR] Parsing function body for: " << irc->GetIRCtxInfo().GetCurrentFunction()->GetFunctionName();

    StatementNode* statement;
    for (auto it = GetStatementBegin(); it != GetStatementEnd(); it++) {
        statement = *it;
        statement->GenerateIR(irc);
    }
}

void FunctionDeclNode::GenerateIR(IRC* irc) const {
    LOG(INFO) << "[IR] Parsing function: " << identifier_->GetIdentifierName();

    std::string func_name = identifier_->GetIdentifierName();
    Function* func = new Function(func_name);
    irc->GetIRCtxInfo().SetCurrentFunction(func);

    // Parse vars and formal params
    auto local_sym_table = irc->GetASTConst().GetLocalSymTable(func_name);
    for (auto table_entry: local_sym_table) {
    }

    func_body_->GenerateIR(irc);

    irc->GetIRCtxInfo().ClearCurrentFunction();
}

void ComputationNode::GenerateIR(IRC* irc) const {
    LOG(INFO) << "[IR] Parsing functions..";

    for (auto funcn: function_declarations_) {
        funcn->GenerateIR(irc);
    }
}

void IRConstructor::construct() {
    const ComputationNode* root = astconst_.GetRoot();
    root->GenerateIR(this);
}
