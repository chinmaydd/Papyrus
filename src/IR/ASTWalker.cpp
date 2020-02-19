#include "ASTWalker.h"

using namespace papyrus;

// NOTE: Ideally, we should have a visit function for each node. But 
// since the AST is a little too detailed, we will only have it for select
// nodes. It makes sense to have that kind of a model only when we are parsing
// Expr and Statement into the AST.
//

//////////////////////////////

void IRCtxInfo::AddFunction(Function* func) {
    functions_[func->GetFunctionName()] = func;
}

//////////////////////////////

ValueIndex GenerateExpressionIR(const ExpressionNode* expr, Graph *ir, IRCtxInfo& ctx) {
    return -1;
}

// Variable* CreateVariableFromDesignator(const DesignatorNode* desig_node, Function* func, IRCtxInfo& ctx) {
//     Variable* var;
//     if (desig_node->GetDesignatorType() == DesignatorType::DESIG_VAR) {
//         //
//     } else {
//         //
//     }
// 
//     return nullptr;
// }

ValueIndex GenerateStorage(const DesignatorNode* desig_node, Function* func, IRCtxInfo& ctx) {

}

void GenerateStatementIR(StatementNode* statement, Function *func, IRCtxInfo& ctx) {
    Graph* ir = func->GetIR();

    switch(statement->GetStatementType()) {
        case StatementType::STAT_FUNCCALL:
            {}
        case StatementType::STAT_ASSIGN: 
            {
                AssignmentNode* assign_node = static_cast<AssignmentNode*>(statement);
                ValueIndex expr = GenerateExpressionIR(assign_node->GetAssignedExpression(), ir, ctx);

                ValueIndex store = GenerateStorage(assign_node->GetDesignator(), func, ctx);
                // ir->AddInstruction(INS_STORE, val_idx, var_store);
                // ir->WriteVariable(var->GetIdentifierName(), ir->CurrentBB(), val_idx);
            }
    }
}

void GenerateFunctionIR(Function *func, FunctionBodyNode* func_body, IRCtxInfo& ctx) {
    Graph *ir = new Graph();
    func->SetIR(ir);

    auto begin = func_body->GetStatementBegin();
    auto end = func_body->GetStatementEnd();

    StatementNode* statement;
    for (auto it = begin; it != end; it++) {
        if (ir->CurrentNode()->IsEntryNode()) {
            ir->CreateNewBB();
        }

        statement = *it;
        GenerateStatementIR(statement, func, ctx);
    }

    ir->CreateExit();
}

void FunctionDeclNode::GenerateIR(IRCtxInfo& ctx) {
    Function* func = new Function(identifier_->GetIdentifierName());

    // XXX: How do we handle formal parameters? Are they supposed to be 
    // considered as local variables in this case?
    // Lets keep this as a TODO for now.
    //
    // XXX: For formal parameter passing, also think about how to perform
    // type checking.

    // XXX: FIX THIS! (Sometime later)
    // This is extremely terrible design. Ideally, we would want to 
    // walk over the AST and have functions which generate relevant
    // parts of the IR as we walk over it.
    // For example:
    // Function->GenerateIR(ctx);
    GenerateFunctionIR(func, func_body_, ctx);

    ctx.AddFunction(func);

    return;
}

// Here, we are assuming that ctx is already initialised with 
// global base and other auxiliary information. We will look
// at concretizing this later as and when the need arises.
void ComputationNode::GenerateIR(IRCtxInfo& ctx) {
    // Let us first parse global variables.
    // LOG(INFO) << "[IR] Parsing Global Variables";

    // bool t_is_array;
    // std::vector<int> t_dimensions;
    // for (auto variable_decl: variable_declarations_) {
    //     t_is_array = variable_decl->IsArray();
    //     t_dimensions = variable_decl->GetDimensions();

    //     for (auto t_identifier: variable_decl->GetIdentifiers()) {
    //         ctx.AddGlobalVariable(new Variable(t_identifier->GetIdentifierName(), t_dimensions, t_is_array, true));
    //     }
    // }

    LOG(INFO) << "[IR] Parsing Functions";

    for (auto function_decl: function_declarations_) {
        function_decl->GenerateIR(ctx);
    }
}
