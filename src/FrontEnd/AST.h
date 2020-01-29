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
    long int GetConstantValue() const { return value_; }

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
    void SetPrimaryTerm(ExpressionNode*);
    void SetSecondaryTerm(ExpressionNode*);
    void SetOperation(ArithmeticOperator);
    void SetExpressionType(ExpressionType);

private:
    ExpressionNode* primary_term_;
    ExpressionNode* secondary_term_;
    ArithmeticOperator op_;
    bool is_binary_;
    ExpressionType expression_type_;
};

////////////////////////////////
////////////////////////////////

class DesignatorNode : public ASTNode {
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

class FunctionCallNode : public ExpressionNode, public StatementNode {
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
    ReturnNode(ExpressionNode*);

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
