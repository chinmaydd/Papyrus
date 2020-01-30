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
class ASTNode {};

class ValueNode : public ASTNode {};
////////////////////////////////
////////////////////////////////

class IdentifierNode : public ASTNode {
public:
    IdentifierNode(const std::string&);

    const std::string& GetIdentifierName() const { return identifier_name_; }

protected:
    std::string identifier_name_;
};

class ConstantNode : public ValueNode {
public:
    ConstantNode(long int);
    long int GetConstantValue() const { return value_; }

protected:
    long int value_;
};

////////////////////////////////
////////////////////////////////

class DesignatorNode;
class ExpressionNode;
class FunctionCallNode;

////////////////////////////////
////////////////////////////////

class FactorNode : public ASTNode {
public:
    enum FactorType {
        FACT_DESIGNATOR = 0,
        FACT_NUMBER = 1,
        FACT_EXPR = 2,
        FACT_FUNCCALL = 3,
    };

    FactorNode(DesignatorNode*);
    FactorNode(ConstantNode*);
    FactorNode(ExpressionNode*);
    FactorNode(FunctionCallNode*);

private:
    ValueNode* factor_node_;
    FactorType factor_type_;
};

class TermNode : public ASTNode {
public:
    TermNode(FactorNode*);
    void AddSecondaryFactor(ArithmeticOperator, FactorNode*);

private:
    FactorNode* primary_factor_;
    std::vector<std::pair<ArithmeticOperator, FactorNode*> > secondary_factors_;
};

class ExpressionNode : public ValueNode {
public:
    ExpressionNode(TermNode*);
    void AddSecondaryTerm(ArithmeticOperator, TermNode*);

private:
    TermNode* primary_term_;
    std::vector<std::pair<ArithmeticOperator, TermNode*> > secondary_terms_;
};

////////////////////////////////
////////////////////////////////

class DesignatorNode : public ValueNode {
protected:
    IdentifierNode* identifier_;
    DesignatorNode(IdentifierNode*);
};

class VarIdentifierNode : public DesignatorNode {
public:
    VarIdentifierNode(IdentifierNode*);
};

class ArrIdentifierNode : public DesignatorNode {
public:
    ArrIdentifierNode(IdentifierNode*);
    void AddIndirectionToArray(ExpressionNode*);

private:
    std::vector<ExpressionNode*> indirections_;
};

////////////////////////////////
////////////////////////////////

class RelationNode : public ASTNode {
public:
    RelationNode(ExpressionNode*, RelationalOperator, ExpressionNode*);

private:
    ExpressionNode* left_expr_;
    RelationalOperator op_;
    ExpressionNode* right_expr_;
};

////////////////////////////////
////////////////////////////////

class StatementNode : public ASTNode {
};

class FunctionCallNode : public ValueNode, public StatementNode {
public:
    FunctionCallNode(IdentifierNode*);
    void AddArgument(ExpressionNode*);

private:
    IdentifierNode* identifier_;
    std::vector<ExpressionNode*> arguments_;
};

class AssignmentNode : public StatementNode {
public:
    AssignmentNode(DesignatorNode*, ExpressionNode*);

private:
    DesignatorNode* designator_;
    ExpressionNode* value_;
};

class StatSequenceNode : public ASTNode {
public:
    void AddStatementToSequence(StatementNode*);

private:
    std::vector<StatementNode*> statements_;
};

class ITENode : public StatementNode {
public:
    ITENode(RelationNode*, StatSequenceNode*);
    void AddElseClause(StatSequenceNode*);

private:
    RelationNode* relation_;
    StatSequenceNode* if_sequence_;
    StatSequenceNode* else_sequence_;
};

class ReturnNode : public StatementNode {
public:
    void AddReturnExpression(ExpressionNode*);

private:
    ExpressionNode* return_expression_;
};

class WhileNode : public StatementNode {
public:
    WhileNode(RelationNode*, StatSequenceNode*);

private:
    RelationNode* loop_condition_;
    StatSequenceNode* statement_sequence_;
};

////////////////////////////////
////////////////////////////////

class TypeDeclNode : public ASTNode {
public:
    void AddArrayDimension(ConstantNode*);
    const bool IsArray() const { return is_array_;};
    void SetIfArray(bool);
private:
    bool is_array_;
    std::vector<ConstantNode*> dimensions_;
};

class VarDeclNode : public ASTNode {
public:
    VarDeclNode(TypeDeclNode*, IdentifierNode*);
    void AddIdentifierDecl(IdentifierNode*);

private:
    TypeDeclNode* type_declaration_;
    std::vector<IdentifierNode*> identifiers_;
};

////////////////////////////////
////////////////////////////////

class FormalParamNode : public ASTNode {
public:
    void AddFormalParam(IdentifierNode*);

private:
    std::vector<IdentifierNode*> parameters_;
};

class FunctionBodyNode : public ASTNode {
public:
    void AddVariableDecl(VarDeclNode*);
    void SetFunctionBodyStatSequence(StatSequenceNode*);

private:
    std::vector<VarDeclNode*> var_declarations_;
    StatSequenceNode* func_statement_sequence_;
};

class FunctionDeclNode : public ASTNode {
public:
    FunctionDeclNode(IdentifierNode*, FormalParamNode*, FunctionBodyNode*);

private:
    IdentifierNode* identifier_;
    FormalParamNode* formal_parameters_;
    FunctionBodyNode* func_body_;
};

////////////////////////////////
////////////////////////////////

class ComputationNode : public ASTNode {
public:
    void AddGlobalVariableDecl(VarDeclNode*);
    void AddFunctionDecl(FunctionDeclNode*);
    void SetComputationBody(StatSequenceNode*);

private:
    std::vector<VarDeclNode*> variable_declarations_;
    std::vector<FunctionDeclNode*> function_declarations_;
    StatSequenceNode* computation_body_;
};

} // namespace papyrus

#endif /* PAPYRUS_ASTNODE_H */
