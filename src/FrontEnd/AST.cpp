#include "AST.h"

using namespace papyrus;

using ExpressionNodePtr   = std::unique_ptr<ExpressionNode>&; 
using DesignatorNodePtr   = std::unique_ptr<DesignatorNode>&;
using IdentifierNodePtr   = std::unique_ptr<IdentifierNode>&;
using StatementNodePtr    = std::unique_ptr<StatementNode>&;
using RelationNodePtr     = std::unique_ptr<RelationNode>&;
using StatSequenceNodePtr = std::unique_ptr<StatSequenceNode>&;
using NumNodePtr          = std::unique_ptr<NumNode>&;
using TypeDeclNodePtr     = std::unique_ptr<TypeDeclNode>&;
using VarDeclNodePtr      = std::unique_ptr<VarDeclNode>&;

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

void ExpressionNode::SetPrimaryTerm(ExpressionNodePtr primary) {
    primary_term_ = std::move(primary);
}

void ExpressionNode::SetSecondaryTerm(ExpressionNodePtr secondary) {
    // Do not set secondary term if primary is not set.
    // XXX: Is this the best way to handdle this case?
    if (primary_term_ == nullptr) {
        return;
    }

    secondary_term_  = std::move(secondary);
    is_binary_ = true;
}

void ExpressionNode::SetOperation(ArithmeticOperator op) {
    op_ = op;
}

////////////////////////////////////
// Designator Node
////////////////////////////////////
DesignatorNode::DesignatorNode(IdentifierNodePtr identifier) :
    identifier_(std::move(identifier)) {
}

////////////////////////////////////
// VarIdentifier Node
////////////////////////////////////

////////////////////////////////////
// ArrayIdentifier Node
////////////////////////////////////
void ArrayIdentifierNode::AddIndirectionToArray(ExpressionNodePtr indirection) {
    indirections_.push_back(std::move(indirection));
}

////////////////////////////////////
// Statement Node
////////////////////////////////////

//////////////////////////////////////
// FunctionCall Node
////////////////////////////////////
FunctionCallNode::FunctionCallNode(IdentifierNodePtr identifier) :
    identifier_(std::move(identifier)) {}

void FunctionCallNode::AddArgument(ExpressionNodePtr expression) {
    arguments_.push_back(std::move(expression));
}

//////////////////////////////////////
// Assignment Node
////////////////////////////////////
AssignmentNode::AssignmentNode(DesignatorNodePtr designator, ExpressionNodePtr value) :
    designator_(std::move(designator)),
    value_(std::move(value)) {}

////////////////////////////////////
// ITE Node
////////////////////////////////////
ITENode::ITENode(RelationNodePtr relation, StatSequenceNodePtr if_sequence) :
    relation_(std::move(relation)),
    if_sequence_(std::move(if_sequence)) {}

void ITENode::AddElseClause(StatSequenceNodePtr else_sequence) {
    else_sequence_ = std::move(else_sequence);
}

////////////////////////////////////
// Return Node
////////////////////////////////////
ReturnNode::ReturnNode(ExpressionNodePtr return_expression) :
    return_expression_(std::move(return_expression)) {}

////////////////////////////////////
// While Node
////////////////////////////////////
WhileNode::WhileNode(RelationNodePtr loop_condition, StatSequenceNodePtr statement_sequence) :
    loop_condition_(loop_condition),
    statement_sequence_(statement_sequence) {}

////////////////////////////////////
// StatSequence Node
////////////////////////////////////
void StatSequenceNode::AddStatementToSequence(StatementNodePtr statement) {
    statements_.push_back(std::move(statement));

}

////////////////////////////////////
// Relation Node
////////////////////////////////////
RelationNode::RelationNode(ExpressionNodePtr left_expr, RelationalOperator op, ExpressionNodePtr right_expr) :
    left_expr_(std::move(left_expr)),
    op_(op),
    right_expr_(std::move(right_expr)) {}

////////////////////////////////////
// TypeDecl Node
////////////////////////////////////
TypeDeclNode::TypeDeclNode(IdentifierNodePtr identifier) :
    identifier_(std::move(identifier)) {}

////////////////////////////////////
// VarIdentifier Node
////////////////////////////////////

////////////////////////////////////
// ArrIdentifier Node
////////////////////////////////////
ArrIdentifierNode::ArrIdentifierNode(IdentifierNodePtr identifier) :
    TypeDeclNode(identifier) {}

void ArrIdentifierNode::AddArrDimension(NumNodePtr dimension) {
    dimensions_.push_back(std::move(dimension));
}

////////////////////////////////////
// VarDecl Node
////////////////////////////////////
VarDeclNode::VarDeclNode(TypeDeclNodePtr type_declaration, IdentifierNodePtr identifier) :
    type_declaration_(std::move(type_declaration)),
    identifiers_{std::move(identifier)} {}

////////////////////////////////////
// FormalParam Node
////////////////////////////////////
FormalParamNode::AddFormalParam(IdentifierNodePtr identifier) {
    parameters_.push_back(std::move(identifier));
}

////////////////////////////////////
// FunctionBody Node
////////////////////////////////////
FunctionBodyNode::FunctionBodyNode(VarDeclNodePtr var_declarations, StatSequenceNodePtr func_statement_seq) :
    var_declarations_(std::move(var_declarations)),
    func_statement_sequence(std::move(func_statement_seq)) {}

////////////////////////////////////
// FunctionDecl Node
////////////////////////////////////
FunctionDeclNode(FormalParamNodePtr formal_parameters, FunctionBodyNodePtr func_body) :
    formal_parameters_(std::move(formal_parameters)),
    func_body_(std::move(func_body)) {}

////////////////////////////////////
// Computation Node
////////////////////////////////////
void ComputationNode::AddGlobalVariableDeclarations(VarDeclNodePtr var_declaration) {
    variable_declarations_.push_back(std::move(var_declaration));
}

void ComputationNode::AddFunctionDeclarations(FunctionDeclNodePtr func_declaration) {
    function_declarations_.push_back(std::move(func_declaration));
}

void ComputationNode::SetComputationBody(StatSequenceNodePtr computation_body) {
    computation_body_ = std::move(computation_body);
}
