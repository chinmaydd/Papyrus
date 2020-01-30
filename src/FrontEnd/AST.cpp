#include "AST.h"

using namespace papyrus;

////////////////////////////////////
// IdentifierNode
////////////////////////////////////
IdentifierNode::IdentifierNode(const std::string& identifier_name) :
    identifier_name_(identifier_name) {}

////////////////////////////////////
// ConstantNode
////////////////////////////////////
ConstantNode::ConstantNode(long int value) :
    value_(value) {}

////////////////////////////////////
// FactorNode
////////////////////////////////////
FactorNode::FactorNode(DesignatorNode* desig) :
    factor_node_(static_cast<ValueNode*>(desig)),
    factor_type_(FactorType::FACT_DESIGNATOR) {}

FactorNode::FactorNode(ConstantNode* constant) :
    factor_node_(static_cast<ValueNode*>(constant)),
    factor_type_(FactorType::FACT_NUMBER) {}

FactorNode::FactorNode(ExpressionNode* expr) :
    factor_node_(static_cast<ValueNode*>(expr)),
    factor_type_(FactorType::FACT_EXPR) {}

FactorNode::FactorNode(FunctionCallNode* func_call) :
    factor_node_(static_cast<ValueNode*>(func_call)),
    factor_type_(FactorType::FACT_FUNCCALL) {}

////////////////////////////////////
// TermNode
////////////////////////////////////
TermNode::TermNode(FactorNode* factor) :
    primary_factor_(factor) {}

void TermNode::AddSecondaryFactor(ArithmeticOperator op, FactorNode* factor) {
    secondary_factors_.push_back({op, factor});
}

////////////////////////////////////
// ExpressionNode
////////////////////////////////////
ExpressionNode::ExpressionNode(TermNode* term) :
    primary_term_(term) {}

void ExpressionNode::AddSecondaryTerm(ArithmeticOperator op, TermNode* term) {
    secondary_terms_.push_back({op, term});
}

////////////////////////////////////
// DesignatorNode
////////////////////////////////////
DesignatorNode::DesignatorNode(IdentifierNode* identifier) :
    identifier_(identifier) {
}

////////////////////////////////////
// VarIdentifierNode
////////////////////////////////////
VarIdentifierNode::VarIdentifierNode(IdentifierNode* identifier) :
    DesignatorNode(identifier) {}

////////////////////////////////////
// ArrayIdentifierNode
////////////////////////////////////
ArrIdentifierNode::ArrIdentifierNode(IdentifierNode* identifier) :
    DesignatorNode(identifier) {}

void ArrIdentifierNode::AddIndirectionToArray(ExpressionNode* indirection) {
    indirections_.push_back(indirection);
}

////////////////////////////////////
// StatementNode
////////////////////////////////////

//////////////////////////////////////
// FunctionCallNode
////////////////////////////////////
FunctionCallNode::FunctionCallNode(IdentifierNode* identifier) :
    identifier_(identifier),
    StatementNode() {}

void FunctionCallNode::AddArgument(ExpressionNode* expression) {
    arguments_.push_back(expression);
}

//////////////////////////////////////
// AssignmentNode
////////////////////////////////////
AssignmentNode::AssignmentNode(DesignatorNode* designator, ExpressionNode* value) :
    designator_(designator),
    value_(value),
    StatementNode() {}

////////////////////////////////////
// ITENode
////////////////////////////////////
ITENode::ITENode(RelationNode* relation, StatSequenceNode* if_sequence) :
    relation_(relation),
    if_sequence_(if_sequence),
    StatementNode() {}

void ITENode::AddElseClause(StatSequenceNode* else_sequence) {
    else_sequence_ = else_sequence;
}

////////////////////////////////////
// ReturnNode
////////////////////////////////////
void ReturnNode::AddReturnExpression(ExpressionNode* return_expression) {
    return_expression_ = return_expression;
}

////////////////////////////////////
// WhileNode
////////////////////////////////////
WhileNode::WhileNode(RelationNode* loop_condition, StatSequenceNode* statement_sequence) :
    loop_condition_(loop_condition),
    statement_sequence_(statement_sequence),
    StatementNode() {}

////////////////////////////////////
// StatSequenceNode
////////////////////////////////////
void StatSequenceNode::AddStatementToSequence(StatementNode* statement) {
    statements_.push_back(statement);
}

////////////////////////////////////
// RelationNode
////////////////////////////////////
RelationNode::RelationNode(ExpressionNode* left_expr, RelationalOperator op, ExpressionNode* right_expr) :
    left_expr_(left_expr),
    op_(op),
    right_expr_(right_expr) {}

////////////////////////////////////
// TypeDeclNode
////////////////////////////////////
void TypeDeclNode::AddArrayDimension(ConstantNode* dimension) {
    dimensions_.push_back(dimension);
}

void TypeDeclNode::SetIfArray(bool is_array) {
    is_array_ = is_array;
}

////////////////////////////////////
// VarDeclNode
////////////////////////////////////
VarDeclNode::VarDeclNode(TypeDeclNode* type_declaration, IdentifierNode* identifier) :
    type_declaration_(type_declaration) {
        identifiers_.push_back(identifier);
}

void VarDeclNode::AddIdentifierDecl(IdentifierNode* identifier) {
    identifiers_.push_back(identifier);
}

////////////////////////////////////
// FormalParamNode
////////////////////////////////////
void FormalParamNode::AddFormalParam(IdentifierNode* identifier) {
    parameters_.push_back(identifier);
}

////////////////////////////////////
// FunctionBodyNode
////////////////////////////////////
void FunctionBodyNode::AddVariableDecl(VarDeclNode* var_decl) {
    var_declarations_.push_back(var_decl);
}

void FunctionBodyNode::SetFunctionBodyStatSequence(StatSequenceNode* stat) {
    func_statement_sequence_ = stat;
}

////////////////////////////////////
// FunctionDeclNode
////////////////////////////////////
FunctionDeclNode::FunctionDeclNode(IdentifierNode* identifier, FunctionBodyNode* func_body) :
    identifier_(identifier),
    func_body_(func_body) {}

void FunctionDeclNode::AddFormalParam(FormalParamNode* formal_param) {
    formal_parameters_ = formal_param;
}

////////////////////////////////////
// ComputationNode
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
