#ifndef PAPYRUS_ASTNODE_H
#define PAPYRUS_ASTNODE_H

#include "Papyrus/Logger/Logger.h"
#include "Operation.h"

#include <string>
#include <memory>
#include <vector>

////////////////////////////////
////////////////////////////////
namespace papyrus {
class ASTNode {
};

////////////////////////////////
////////////////////////////////

class IdentifierNode : public ASTNode {
public:
    IdentifierNode(std::string&);

    const std::string& GetIdentifierName() const { return identifier_name_; }

protected:
    std::string identifier_name_;
};

class ConstantNode : public ASTNode {
public:
    ConstantNode(long int);
    const long int GetConstantValue() const { return value_; }
protected:
    long int value_;
};

////////////////////////////////
////////////////////////////////

class ExpressionNode : public ASTNode {
public:
    enum ExpressionType {
        EXPR_NONE = 0,
        EXPR_TERM = 1,
        EXPR_FACTOR = 2,
        EXPR_DESIGNATOR = 3,
        EXPR_CONSTANT = 4,
        EXPR_EXPRESSION = 5,
        EXPR_FUNCCALL = 6,
        EXPR_UNKNOWN = 7,
    };

    explicit ExpressionNode();
    void SetPrimaryTerm(std::unique_ptr<ExpressionNode>&);
    void SetSecondaryTerm(std::unique_ptr<ExpressionNode>&);
    void SetOperation(ArithmeticOperator);

private:
    std::unique_ptr<ExpressionNode> primary_term_;
    std::unique_ptr<ExpressionNode> secondary_term_;
    ArithmeticOperator op_;
    bool is_binary_;
};

////////////////////////////////
////////////////////////////////

class DesignatorNode : public ASTNode {
protected:
    std::unique_ptr<IdentifierNode> identifier_;
    DesignatorNode(std::unique_ptr<IdentifierNode>&);
};

class VarIdentifierNode : public DesignatorNode {
};

class ArrIdentifierNode : public DesignatorNode {
public:
    void AddIndirectionToArray(std::unique_ptr<ExpressionNode>&);

private:
    std::vector<std::unique_ptr<ExpressionNode> > indirections_;
};

////////////////////////////////
////////////////////////////////

class RelationNode : public ASTNode {
public:
    RelationNode(std::unique_ptr<ExpressionNode>&, RelationalOperator, std::unique_ptr<ExpressionNode>&);
private:
    std::unique_ptr<ExpressionNode> left_expr_;
    RelationalOperator op_;
    std::unique_ptr<ExpressionNode> right_expr_;
};

////////////////////////////////
////////////////////////////////

class StatementNode : public ASTNode {
};

class FunctionCallNode : public ExpressionNode, public StatementNode {
public:
    FunctionCallNode(std::unique_ptr<IdentifierNode>&);
    void AddArgument(std::unique_ptr<ExpressionNode>&);

private:
    std::unique_ptr<IdentifierNode> identifier_;
    std::vector<std::unique_ptr<ExpressionNode> > arguments_;
};

class AssignmentNode : public StatementNode {
public:
    AssignmentNode(std::unique_ptr<DesignatorNode>&, std::unique_ptr<ExpressionNode>&);
private:
    std::unique_ptr<DesignatorNode> designator_;
    std::unique_ptr<ExpressionNode> value_;
};

class StatSequenceNode : public ASTNode {
public:
    void AddStatementToSequence(std::unique_ptr<StatementNode>&);
private:
    std::vector<std::unique_ptr<StatementNode> > statements_;
};

class ITENode : public StatementNode {
public:
    ITENode(std::unique_ptr<RelationNode>&, std::unique_ptr<StatSequenceNode>&);
    void AddElseClause(std::unique_ptr<StatSequenceNode>&);
private:
    std::unique_ptr<RelationNode> relation_;
    std::unique_ptr<StatSequenceNode> if_sequence_;
    std::unique_ptr<StatSequenceNode> else_sequence_;
};

class ReturnNode : public StatementNode {
public:
    ReturnNode(std::unique_ptr<ExpressionNode>&);
private:
    std::unique_ptr<ExpressionNode> return_expression_;
};

class WhileNode : public StatementNode {
public:
    WhileNode(std::unique_ptr<RelationNode>&, std::unique_ptr<StatSequenceNode>&);
private:
    std::unique_ptr<RelationNode> loop_condition_;
    std::unique_ptr<StatSequenceNode> statement_sequence_;
};

////////////////////////////////
////////////////////////////////

class TypeDeclNode : public ASTNode {
protected:
    TypeDeclNode(std::unique_ptr<IdentifierNode>&);
    std::unique_ptr<IdentifierNode> identifier_;
};

class VarIdentifierDeclNode : public TypeDeclNode {
};

class ArrIdentifierDeclNode : public TypeDeclNode {
public:
    ArrIdentifierDeclNode(std::unique_ptr<IdentifierNode>&);
    void AddArrDimension(std::unique_ptr<ConstantNode>&);
private:
    std::vector<std::unique_ptr<ConstantNode> > dimensions_;
};

class VarDeclNode : public ASTNode {
public:
    VarDeclNode(std::unique_ptr<TypeDeclNode>&, std::unique_ptr<IdentifierNode>&);
    void AddIdentifierDecl(std::unique_ptr<IdentifierNode>&);
private:
    std::unique_ptr<TypeDeclNode> type_declaration_;
    std::vector<std::unique_ptr<IdentifierNode> > identifiers_;
};

////////////////////////////////
////////////////////////////////

class FormalParamNode : public ASTNode {
public:
    void AddFormalParam(std::unique_ptr<IdentifierNode>&);
private:
    std::vector<std::unique_ptr<IdentifierNode> > parameters_;
};

class FunctionBodyNode : public ASTNode {
public:
    FunctionBodyNode(std::unique_ptr<VarDeclNode>&, std::unique_ptr<StatSequenceNode>&);
private:
    std::unique_ptr<VarDeclNode> var_declarations_;
    std::unique_ptr<StatSequenceNode> func_statement_sequence_;
};

class FunctionDeclNode : public ASTNode {
public:
    FunctionDeclNode(std::unique_ptr<FormalParamNode>&, std::unique_ptr<FunctionBodyNode>&);
private:
    std::unique_ptr<FormalParamNode> formal_parameters_;
    std::unique_ptr<FunctionBodyNode> func_body_;
};

////////////////////////////////
////////////////////////////////

class ComputationNode : public ASTNode {
public:
    void AddGlobalVariableDeclarations(std::unique_ptr<VarDeclNode>&);
    void AddFunctionDeclarations(std::unique_ptr<FunctionDeclNode>&);
    void SetComputationBody(std::unique_ptr<StatSequenceNode>&);
private:
    std::vector<std::unique_ptr<VarDeclNode> > variable_declarations_;
    std::vector<std::unique_ptr<FunctionDeclNode> > function_declarations_;
    std::unique_ptr<StatSequenceNode> computation_body_;
};

} // end namespace papyrus

#endif
