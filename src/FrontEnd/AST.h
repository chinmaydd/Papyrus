#ifndef PAPYRUS_AST_H
#define PAPYRUS_AST_H

#include "Papyrus/Logger/Logger.h"
#include "Operation.h"

#include <string>
#include <memory>
#include <vector>

#define NOTFOUND -1

using ValueIndex = int;

namespace papyrus {

class IRConstructor;
using IRC = IRConstructor;

////////////////////////////////
class ASTNode {};
////////////////////////////////

////////////////////////////////
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
////////////////////////////////

////////////////////////////////
class ConstantNode : public ValueNode {
public:
    ConstantNode(int);
    const int GetConstantValue() const { return value_; }

    ValueIndex GenerateIR(IRC& irc) const;

protected:
    int value_;
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

    ValueIndex GenerateIR(IRC&) const;

private:
    ValueNode* factor_node_;
    FactorType factor_type_;
};
////////////////////////////////

////////////////////////////////
class TermNode : public ASTNode {
public:
    TermNode(FactorNode*);
    void AddSecondaryFactor(ArithmeticOperator, FactorNode*);

    ValueIndex GenerateIR(IRC&) const;

private:
    FactorNode* primary_factor_;
    std::vector<std::pair<ArithmeticOperator, FactorNode*> > secondary_factors_;
};
////////////////////////////////

////////////////////////////////
class ExpressionNode : public ValueNode {
public:
    ExpressionNode(TermNode*);
    void AddSecondaryTerm(ArithmeticOperator, TermNode*);

    ValueIndex GenerateIR(IRC&) const;

private:
    TermNode* primary_term_;
    std::vector<std::pair<ArithmeticOperator, TermNode*> > secondary_terms_;
};
////////////////////////////////

////////////////////////////////
enum DesignatorType {
    DESIG_NONE,
    DESIG_VAR,
    DESIG_ARR,
    DESIG_ANY
};

class DesignatorNode : public ValueNode {
public:
    const std::string& GetIdentifierName() const { return identifier_->GetIdentifierName(); }
    DesignatorType GetDesignatorType() const { return desig_type_; }

    virtual ValueIndex GenerateIR(IRC&) const = 0;

protected:

    IdentifierNode* identifier_;
    DesignatorNode(IdentifierNode*);
    DesignatorType desig_type_;
};
////////////////////////////////

////////////////////////////////
class VarIdentifierNode : public DesignatorNode {
public:
    VarIdentifierNode(IdentifierNode*);

    ValueIndex GenerateIR(IRC&) const override { return NOTFOUND; }
};
////////////////////////////////

////////////////////////////////
class ArrIdentifierNode : public DesignatorNode {
public:
    ArrIdentifierNode(IdentifierNode*);
    void AddIndirectionToArray(ExpressionNode*);

    ValueIndex GenerateIR(IRC&) const override;

private:
    std::vector<ExpressionNode*> indirections_;
};
////////////////////////////////

////////////////////////////////
class RelationNode : public ASTNode {
public:
    RelationNode(ExpressionNode*, RelationalOperator, ExpressionNode*);

    RelationalOperator GetOp() const { return op_; }

    ValueIndex GenerateIR(IRC&) const;

private:
    ExpressionNode* left_expr_;
    RelationalOperator op_;
    ExpressionNode* right_expr_;
};
////////////////////////////////

////////////////////////////////
enum StatementType {
    STAT_NONE,

    STAT_FUNCCALL,
    STAT_ASSIGN,
    STAT_ITE,
    STAT_RETURN,
    STAT_WHILE,

    STAT_ANY,
};

class StatementNode : public ASTNode {
public:
    const StatementType GetStatementType() const { return statement_type_; }

    ValueIndex GenerateIR(IRC&) const;

protected:
    StatementType statement_type_;
};
////////////////////////////////

////////////////////////////////
class FunctionCallNode : public ValueNode, public StatementNode {
public:
    FunctionCallNode(IdentifierNode*);
    void AddArgument(ExpressionNode*);

    ValueIndex GenerateIR(IRC&) const;

private:
    IdentifierNode* identifier_;
    std::vector<ExpressionNode*> arguments_;
};
////////////////////////////////

////////////////////////////////
class AssignmentNode : public StatementNode {
public:
    AssignmentNode(DesignatorNode*, ExpressionNode*);

    const ExpressionNode* GetAssignedExpression() const { return value_; }
    const DesignatorNode* GetDesignator() const { return designator_; }

    ValueIndex GenerateIR(IRC&) const;

private:
    DesignatorNode* designator_;
    ExpressionNode* value_;
};
////////////////////////////////

////////////////////////////////
class StatSequenceNode : public ASTNode {
public:
    void AddStatementToSequence(StatementNode*);
    std::vector<StatementNode*>::const_iterator GetStatementBegin() const { return statements_.cbegin(); }
    std::vector<StatementNode*>::const_iterator GetStatementEnd() const { return statements_.cend(); }

    void GenerateIR(IRC&) const;

private:
    std::vector<StatementNode*> statements_;
};
////////////////////////////////

////////////////////////////////
class ITENode : public StatementNode {
public:
    ITENode(RelationNode*, StatSequenceNode*);
    void AddElseClause(StatSequenceNode*);

    ValueIndex GenerateIR(IRC&) const;

private:
    RelationNode* relation_;
    StatSequenceNode* then_sequence_;
    StatSequenceNode* else_sequence_;
};
////////////////////////////////

////////////////////////////////
class ReturnNode : public StatementNode {
public:
    ReturnNode();
    void AddReturnExpression(ExpressionNode*);

private:
    ExpressionNode* return_expression_;
};
////////////////////////////////

////////////////////////////////
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
    std::vector<int> GetDimensions() const {
        std::vector<int> dim;
        for (auto constant: dimensions_) {
            dim.push_back(constant->GetConstantValue());
        }
        return dim;
    }

    const bool IsArray() const { return is_array_; }
    void SetIfArray(bool);

private:
    bool is_array_;
    std::vector<ConstantNode*> dimensions_;
};
////////////////////////////////

////////////////////////////////
class FunctionBodyNode : public ASTNode {
public:
    void SetFunctionBodyStatSequence(StatSequenceNode*);

    void GenerateIR(IRC&) const;

private:
    StatSequenceNode* func_statement_sequence_;
};
////////////////////////////////

////////////////////////////////
class FunctionDeclNode : public ASTNode {
public:
    FunctionDeclNode(IdentifierNode*, FunctionBodyNode*);

    const std::string& GetFunctionName() const { return identifier_->GetIdentifierName(); }

    void GenerateIR(IRC&) const;

private:
    IdentifierNode* identifier_;
    FunctionBodyNode* func_body_;
};
////////////////////////////////

////////////////////////////////
class ComputationNode : public ASTNode {
public:
    void AddFunctionDecl(FunctionDeclNode*);
    void SetComputationBody(StatSequenceNode*);

    void GenerateIR(IRC&) const;
    
private:
    std::vector<FunctionDeclNode*> function_declarations_;
    StatSequenceNode* computation_body_;
};
////////////////////////////////

} // namespace papyrus

#endif /* PAPYRUS_AST_H */
