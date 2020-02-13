#include "ASTWalker.h"

using namespace papyrus;

// NOTE: Ideally, we should have a visit function for each node. But 
// since the AST is a little too detailed, we will only have it for select
// nodes. It makes sense to have that kind of a model only when we are parsing
// Expr and Statement into the AST.

//////////////////////////////////////////////
void IRCtxInfo::AddGlobalVariable(GlobalVariable* var) {
    globals_[var->GetIdentifier()] = var;
}

//////////////////////////////////////////////
Variable::Variable(bool is_array, const std::string& identifier, const std::vector<int>& dimensions) :
    identifier_(identifier),
    is_array_(is_array),
    dimensions_(dimensions) {}

GlobalVariable::GlobalVariable(const std::string& identifier, bool is_array, const std::vector<int>& dimensions) :
    Variable(is_array, identifier, dimensions) {}

// Here, we are assuming that ctx is already initialised with 
// global base and other auxiliary information. We will look
// at this later as and when the need arises.
void ComputationNode::GenerateIR(IRCtxInfo& ctx) {
    // Let us first parse global variables.
    LOG(INFO) << "[IR] Parsing Global Variables";

    bool temp_is_array;
    std::vector<int> temp_dimensions;
    std::string identifier;
    for (auto variable_decl: variable_declarations_) {
        temp_is_array = variable_decl->IsArray();
        temp_dimensions = variable_decl->GetDimensions();

        for (auto identifier: variable_decl->GetIdentifiers()) {
            ctx.AddGlobalVariable(new GlobalVariable(identifier->GetIdentifierName(), temp_is_array, temp_dimensions));
        }
    }
}
