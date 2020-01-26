#ifndef PAPYRUS_ASTNODE_H
#define PAPYRUS_ASTNODE_H

#include "Papyrus/Logger/Logger.h"
#include "Operation.h"

#include <string>
#include <memory>
#include <vector>


// The difference between Parse Trees and Abstract Syntax Trees is very different.
// Here, we will try to build an AST instead of a Parse Tree

namespace papyrus {
class ASTNode {
    virtual void Print();
};

class IdentifierNode : public ASTNode {
public:
    IdentifierNode(std::string&);

    const std::string& GetIdentifierName() const { return identifier_name_; }
private:
    std::string identifier_name_;
};

class ConstantNode : public ASTNode {
public:
    ConstantNode(long int);
private:
    long int value_;
};

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

class DesignatorNode : public ASTNode {
public:
    DesignatorNode(std::unique_ptr<IdentifierNode>&);
    void AddArrayIndirection(std::unique_ptr<ExpressionNode>&);

private:
    std::unique_ptr<IdentifierNode> identifier_;
    std::vector<std::unique_ptr<ExpressionNode> > indirections_;
};

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

class RelationNode : public ASTNode {
public:
    RelationNode(std::unique_ptr<ExpressionNode>&, RelationalOperator, std::unique_ptr<ExpressionNode>&);
private:
    std::unique_ptr<ExpressionNode> left_expr_;
    RelationalOperator op_;
    std::unique_ptr<ExpressionNode> right_expr_;
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
    explicit StatSequenceNode();
    void AddStatement(std::unique_ptr<StatementNode>&);
private:
    std::vector<std::unique_ptr<StatementNode> > statements_;
};

class ITENode : public StatementNode {
public:
    ITENode(std::unique_ptr<RelationNode>&, std::unique_ptr<StatSequenceNode>&);
    void AddElseClaus(std::unique_ptr<StatSequenceNode>&);
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
    std::unique_ptr<RelationNode>& relation_;
    std::unique_ptr<StatSequenceNode>& statement_sequence_;
};

} // end namespace papyrus

#endif
