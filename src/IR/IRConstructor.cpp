#include "IRConstructor.h"

using namespace papyrus;

using ASTC   = ASTConstructor;
using IRCTX  = IRCtxInfo;
using IRC    = IRConstructor;

using ValueIndex = int;

IRConstructor::IRConstructor(ASTC& astconst, IRCTX& irctx) :
    astconst_(astconst),
    irctx_(irctx) {}

ValueIndex ConstantNode::GenerateIR(IRC* irc) const {
    ConstantValue* cval = new ConstantValue(value_);

    ValueIndex retval = irc->GetIRCtxInfo()
                            .GetCurrentFunction()
                            ->AddValue(cval);

    return retval;
}

ValueIndex FactorNode::GenerateIR(IRC* irc) const {
    ValueIndex result = -1;
    switch(factor_type_) {
        case FACT_DESIGNATOR: {
        }
        case FACT_NUMBER: {
            const ConstantNode* cnstn = static_cast<const ConstantNode*>(factor_node_);
            result = cnstn->GenerateIR(irc);
        }
        case FACT_EXPR: {
        }
        case FACT_FUNCCALL: {
        }
    }

    return result;
}

ValueIndex TermNode::GenerateIR(IRC* irc) const {
    ValueIndex primary_idx = primary_factor_->GenerateIR(irc);

    // TODO: Change
    return primary_idx;
}

ValueIndex ExpressionNode::GenerateIR(IRC* irc) const {
    ValueIndex primary_term = primary_term_->GenerateIR(irc);

    // TODO: Change
    return primary_term;
}
    
ValueIndex DesignatorNode::GenerateIR(IRC* irc) const {
    ValueIndex result;
    switch(desig_type_) {
        case DESIG_VAR: {
            // GetLocalBase() which has to be a value
            // the function value_map_
            // AddInstruction AddA + 4*Word
            // Word count can be retrieved from function
            // result = irc->GetIRCtxInfo()
            //             .GetCurrentFunction();
        }
        case DESIG_ARR: {
        }
    }

    return result;
}

void AssignmentNode::GenerateIR(IRC* irc) const {
    ValueIndex expr_idx = value_->GenerateIR(irc);
    ValueIndex mem_location = designator_->GenerateIR(irc);

    irc->GetIRCtxInfo()
        .AddInstruction(Instruction::INS_STORE, expr_idx, mem_location);
}

void FunctionCallNode::GenerateIR(IRC* irc) const {
    // Push onto stack and then call?
}

void StatementNode::GenerateIR(IRC* irc) const {
    switch(statement_type_) {
        case StatementType::STAT_ASSIGN: {
            const AssignmentNode* assgn = static_cast<const AssignmentNode*>(this);
            assgn->GenerateIR(irc);
        }
        case StatementType::STAT_FUNCCALL: {
            const FunctionCallNode* funcn = static_cast<const FunctionCallNode*>(this);
            funcn->GenerateIR(irc);
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

    // TODO: Parse variables and formal parameters
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
