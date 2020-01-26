#include "Node.h"

using namespace papyrus;

using ExpressionNodePtr = std::unique_ptr<ExpressionNode>&; 
using DesignatorNodePtr = std::unique_ptr<DesignatorNode>&;
using IdentifierNodePtr = std::unique_ptr<IdentifierNode>&;

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

void DesignatorNode::AddArrayIndirection(ExpressionNodePtr arr_indirection) {
    indirections_.push_back(std::move(arr_indirection));
}

////////////////////////////////////
// FunctionCall Node
////////////////////////////////////
FunctionCallNode::FunctionCallNode(IdentifierNodePtr identifier) :
    identifier_(std::move(identifier)) {}

void FunctionCallNode::AddArgument(ExpressionNodePtr expression) {
    arguments_.push_back(std::move(expression));
}

////////////////////////////////////
// Relation Node
////////////////////////////////////
RelationNode::RelationNode(ExpressionNodePtr left_expr, RelationalOperator op, ExpressionNodePtr right_expr) :
    left_expr_(std::move(left_expr)),
    op_(op),
    right_expr_(std::move(right_expr)) {}

////////////////////////////////////
// Assignment Node
////////////////////////////////////
AssignmentNode::AssignmentNode(DesignatorNodePtr designator, ExpressionNodePtr value) :
    designator_(std::move(designator)),
    value_(std::move(value)) {}
