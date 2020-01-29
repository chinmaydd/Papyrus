#include "AST.h"

using namespace papyrus;

////////////////////////////////////
// Identifier Node
////////////////////////////////////
IdentifierNode::IdentifierNode(std::string& identifier_name) :
    identifier_name_(identifier_name) {}

////////////////////////////////////
// Constant Node
////////////////////////////////////
ConstantNode::ConstantNode(long int value) :
    value_(value) {}

////////////////////////////////////
// Expression Node
////////////////////////////////////
ExpressionNode::ExpressionNode() :
    primary_term_(nullptr),
    secondary_term_(nullptr),
    op_(BINOP_NONE),
    is_binary_(false) {}

void ExpressionNode::SetPrimaryTerm(ExpressionNode* primary) {
    primary_term_ = primary;
}

void ExpressionNode::SetSecondaryTerm(ExpressionNode* secondary) {
    // Do not set secondary term if primary is not set.
    // XXX: Is this the best way to handdle this case?
    if (primary_term_ == nullptr) {
        return;
    }

    secondary_term_  = secondary;
    is_binary_ = true;
}

void ExpressionNode::SetOperation(ArithmeticOperator op) {
    op_ = op;
}

////////////////////////////////////
// Designator Node
////////////////////////////////////
DesignatorNode::DesignatorNode(IdentifierNode* identifier) :
    identifier_(identifier) {
}

////////////////////////////////////
// VarIdentifier Node
////////////////////////////////////
VarIdentifierNode::VarIdentifierNode(IdentifierNode* identifier) :
    DesignatorNode(identifier) {}

////////////////////////////////////
// ArrayIdentifier Node
////////////////////////////////////
ArrIdentifierNode::ArrIdentifierNode(IdentifierNode* identifier) :
    DesignatorNode(identifier) {}

void ArrIdentifierNode::AddIndirectionToArray(ExpressionNode* indirection) {
    indirections_.push_back(indirection);
}

////////////////////////////////////
// Statement Node
////////////////////////////////////

//////////////////////////////////////
// FunctionCall Node
////////////////////////////////////
FunctionCallNode::FunctionCallNode(IdentifierNode* identifier) :
    identifier_(identifier),
    StatementNode() {}

void FunctionCallNode::AddArgument(ExpressionNode* expression) {
    arguments_.push_back(expression);
}

//////////////////////////////////////
// Assignment Node
////////////////////////////////////
AssignmentNode::AssignmentNode(DesignatorNode* designator, ExpressionNode* value) :
    designator_(designator),
    value_(value),
    StatementNode() {}

////////////////////////////////////
// ITE Node
////////////////////////////////////
ITENode::ITENode(RelationNode* relation, StatSequenceNode* if_sequence) :
    relation_(relation),
    if_sequence_(if_sequence),
    StatementNode() {}

void ITENode::AddElseClause(StatSequenceNode* else_sequence) {
    else_sequence_ = else_sequence;
}

////////////////////////////////////
// Return Node
////////////////////////////////////
ReturnNode::ReturnNode(ExpressionNode* return_expression) :
    return_expression_(return_expression),
    StatementNode() {}

////////////////////////////////////
// While Node
////////////////////////////////////
WhileNode::WhileNode(RelationNode* loop_condition, StatSequenceNode* statement_sequence) :
    loop_condition_(loop_condition),
    statement_sequence_(statement_sequence),
    StatementNode() {}

////////////////////////////////////
// StatSequence Node
////////////////////////////////////
void StatSequenceNode::AddStatementToSequence(StatementNode* statement) {
    statements_.push_back(statement);
}

////////////////////////////////////
// Relation Node
////////////////////////////////////
RelationNode::RelationNode(ExpressionNode* left_expr, RelationalOperator op, ExpressionNode* right_expr) :
    left_expr_(left_expr),
    op_(op),
    right_expr_(right_expr) {}

////////////////////////////////////
// TypeDecl Node
////////////////////////////////////
void TypeDeclNode::AddArrayDimension(ConstantNode* dimension) {
    dimensions_.push_back(dimension);
}

void TypeDeclNode::SetIfArray(bool is_array) {
    is_array_ = is_array;
}

////////////////////////////////////
// VarDecl Node
////////////////////////////////////
VarDeclNode::VarDeclNode(TypeDeclNode* type_declaration, IdentifierNode* identifier) :
    type_declaration_(type_declaration) {
        identifiers_.push_back(identifier);
}

void VarDeclNode::AddIdentifierDecl(IdentifierNode* identifier) {
    identifiers_.push_back(identifier);
}

////////////////////////////////////
// FormalParam Node
////////////////////////////////////
void FormalParamNode::AddFormalParam(IdentifierNode* identifier) {
    parameters_.push_back(identifier);
}

////////////////////////////////////
// FunctionBody Node
////////////////////////////////////
void FunctionBodyNode::AddVariableDecl(VarDeclNode* var_decl) {
    var_declarations_.push_back(var_decl);
}

void FunctionBodyNode::SetFunctionBodyStatSequence(StatSequenceNode* stat) {
    func_statement_sequence_ = stat;
}

////////////////////////////////////
// FunctionDecl Node
////////////////////////////////////
FunctionDeclNode::FunctionDeclNode(IdentifierNode* identifier, FormalParamNode* formal_parameters, FunctionBodyNode* func_body) :
    identifier_(identifier),
    formal_parameters_(formal_parameters),
    func_body_(func_body) {}

////////////////////////////////////
// Computation Node
////////////////////////////////////
void ComputationNode::AddGlobalVariableDecl(VarDeclNode* var_declaration) {
    variable_declarations_.push_back(var_declaration);
}

void ComputationNode::AddFunctionDecl(FunctionDeclNode* func_declaration) {
    function_declarations_.push_back(func_declaration);
}

void ComputationNode::SetComputationBody(StatSequenceNode* computation_body) {
    computation_body_ = computation_body;
}
