#include "ASTWalker.h"

using namespace papyrus;

// NOTE: Ideally, we should have a visit function for each node. But 
// since the AST is a little too detailed, we will only have it for select
// nodes. It makes sense to have that kind of a model only when we are parsing
// Expr and Statement into the AST.
//

//////////////////////////////

void IRCtxInfo::AddGlobalVariable(GlobalVariable* var) {
    globals_[var->GetIdentifierName()] = var;
}

void IRCtxInfo::AddFunction(Function* func) {
    functions_[func->GetFunctionName()] = func;
}

//////////////////////////////

Variable::Variable(bool is_array, const std::string& identifier, const std::vector<int>& dimensions) :
    identifier_(identifier),
    is_array_(is_array),
    dimensions_(dimensions) {}

GlobalVariable::GlobalVariable(const std::string& identifier, bool is_array, const std::vector<int>& dimensions) :
    Variable(is_array, identifier, dimensions) {}

LocalVariable::LocalVariable(const std::string& identifier, bool is_array, const std::vector<int>& dimensions) :
    Variable(is_array, identifier, dimensions) {}

//////////////////////////////

void GenerateFunctionIR(Function *func, FunctionBodyNode* func_body, IRCtxInfo& ctx) {
    Graph *ir = new Graph();

    auto begin = func_body->GetStatementBegin();
    auto end = func_body->GetStatementEnd();
    for (auto statement = begin; statement != end; statement++) {


    }
}

void FunctionDeclNode::GenerateIR(IRCtxInfo& ctx) {
    Function* func = new Function(identifier_->GetIdentifierName());

    // XXX: How do we handle formal parameters? Are they supposed to be 
    // considered as local variables in this case?
    // Lets keep this as a TODO for now.

    bool t_is_array;
    std::vector<int> t_dimensions;
    for (auto variable_decl: func_body_->GetLocals()) {
        t_is_array = variable_decl->IsArray();
        t_dimensions = variable_decl->GetDimensions();

        for (auto t_identifier: variable_decl->GetIdentifiers()) {
            func->AddLocalVariable(new LocalVariable(t_identifier->GetIdentifierName(), t_is_array, t_dimensions));
        }
    }

    // XXX: FIX THIS! (Sometime later)
    GenerateFunctionIR(func, func_body_, ctx);

    ctx.AddFunction(func);

    return;
}

// Here, we are assuming that ctx is already initialised with 
// global base and other auxiliary information. We will look
// at concretizing this later as and when the need arises.
void ComputationNode::GenerateIR(IRCtxInfo& ctx) {
    // Let us first parse global variables.
    LOG(INFO) << "[IR] Parsing Global Variables";

    bool t_is_array;
    std::vector<int> t_dimensions;
    for (auto variable_decl: variable_declarations_) {
        t_is_array = variable_decl->IsArray();
        t_dimensions = variable_decl->GetDimensions();

        for (auto t_identifier: variable_decl->GetIdentifiers()) {
            ctx.AddGlobalVariable(new GlobalVariable(t_identifier->GetIdentifierName(), t_is_array, t_dimensions));
        }
    }

    LOG(INFO) << "[IR] Parsing Functions";

    for (auto function_decl: function_declarations_) {
        function_decl->GenerateIR(ctx);
    }
}
