#ifndef PAPYRUS_ASTCONSTRUCTOR_H
#define PAPYRUS_ASTCONSTRUCTOR_H

#include "Papyrus/Logger/Logger.h"
#include "Operation.h"
#include "AST.h"
#include "Lexer.h"
#include "IR/Variable.h"

#include <memory>
    
namespace papyrus {
////////////////////////////////
class ASTConstructor {
public:
    ASTConstructor(Lexer&);
    void ConstructAST();
    const ComputationNode* GetRoot() const { return root_; }

private:
    ////////////////////////////////
    Lexer::Token GetCurrentToken() const {
        if (is_peek_) {
            return current_token_;
        } else {
            return lexer_instance_.GetToken();
        }
    }
    Lexer::Token FetchToken() { 
        if (is_peek_) {
            is_peek_ = false;
            return lexer_instance_.GetToken();
        } else {
            return lexer_instance_.GetNextToken();
        }
    }
    Lexer::Token PeekNextToken() {
        if (is_peek_) {
            return lexer_instance_.GetToken();
        } else {
            current_token_ = lexer_instance_.GetToken();
            current_buffer_ = lexer_instance_.GetBuffer();
            current_line_no_ = lexer_instance_.GetLineNo();

            is_peek_ = true;
            return lexer_instance_.GetNextToken();
        }
    }
    ////////////////////////////////
    
    ////////////////////////////////
    const std::string& GetBuffer() const { 
        if (is_peek_) {
            return current_buffer_;
        } else {
            return lexer_instance_.GetBuffer();
        }
    }
    int GetLineNo() const { 
        if (is_peek_) {
            return current_line_no_;
        } else {
            return lexer_instance_.GetLineNo();
        }
    }
    ////////////////////////////////
  
    ////////////////////////////////
    const std::string& GetTokenTranslation(const Lexer::Token& tok) const { 
        return lexer_instance_.GetTokenTranslation(tok);
    }
    bool IsRelationalOp(const Lexer::Token& tok) const { 
        return lexer_instance_.IsRelationalOp(tok);
    }
    RelationalOperator GetRelOperatorForToken(const Lexer::Token& tok) const {
        return lexer_instance_.GetRelOperatorForToken(tok);
    }
    ArithmeticOperator GetBinOperatorForToken(const Lexer::Token& tok) const {
        return lexer_instance_.GetBinOperatorForToken(tok);
    }
    long int ParseCurrentTokenAsNumber() const { 
        if (is_peek_) {
            return std::stol(current_buffer_);
        } else {
            return std::stol(lexer_instance_.GetBuffer());
        }
    }
    ////////////////////////////////
   
    ////////////////////////////////
    void RaiseParseError(const Lexer::Token&) const;
    void RaiseParseError(const std::string&) const;
    bool MustParseToken(const Lexer::Token&, const std::string&, int) const;
    ////////////////////////////////

    ////////////////////////////////
    bool IsStatementBegin(const Lexer::Token&) const;
    bool IsExpressionBegin(const Lexer::Token&) const;
    ////////////////////////////////

    ///////////////////////////////
    std::string current_scope_;
    std::unordered_map<std::string, std::unordered_map<std::string, Symbol*> > symbol_table_;
    std::unordered_map<std::string, Symbol*> local_symbol_table_;
    std::unordered_map<std::string, Symbol*> global_symbol_table_;

    void AddSymbol(const IdentifierNode*, const TypeDeclNode*);
    bool IsDefined(const std::string& identifier) {
        return IsGlobal(identifier) || IsLocal(identifier);
    }
    bool IsGlobal(const std::string& identifier) const {
        return global_symbol_table_.find(identifier) != global_symbol_table_.end();
    }
    bool IsLocal(const std::string identifier) const {
        return local_symbol_table_.find(identifier) != 
               local_symbol_table_.end();
    }

    ////////////////////////////////
    IdentifierNode* ParseIdentifier();
    FunctionDeclNode* ParseFunctionDecl();
    FormalParamNode* ParseFormalParameters();
    FunctionBodyNode* ParseFunctionBody();
    StatSequenceNode* ParseStatementSequence();
    StatementNode* ParseStatement();
    AssignmentNode* ParseAssignment();
    FunctionCallNode* ParseFunctionCall();
    ITENode* ParseITE();
    WhileNode* ParseWhile();
    ReturnNode* ParseReturn();
    ExpressionNode* ParseExpression();
    DesignatorNode* ParseDesignator();
    RelationNode* ParseRelation();
    FactorNode* ParseFactor();
    TermNode* ParseTerm();
    TypeDeclNode* ParseTypeDecl();
    void ParseVariableDecl();
    ////////////////////////////////
    Lexer& lexer_instance_;
    ////////////////////////////////
    ComputationNode* root_;
    ////////////////////////////////
    bool is_peek_;
    int current_line_no_;
    Lexer::Token current_token_;
    std::string current_buffer_;
    ////////////////////////////////
};

} // namespace papyrus

#endif /* PAPYRUS_ASTCONSTRUCTOR_H */
