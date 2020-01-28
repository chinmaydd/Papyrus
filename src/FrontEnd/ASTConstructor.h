#ifndef PAPYRUS_ASTCONSTRUCTOR_H
#define PAPYRUS_ASTCONSTRUCTOR_H

#include "Papyrus/Logger/Logger.h"
#include "Operation.h"
#include "AST.h"
#include "Lexer.h"

#include <memory>
    
// The difference between Parse Trees and Abstract Syntax Trees is very different.
// Here, we will try to build an AST instead of a Parse Tree
namespace papyrus {
class ASTConstructor {
public:
    ASTConstructor(Lexer&);
    ComputationNode* ComputeAST();

private:
    const Lexer::Token GetCurrentToken() const { return lexer_instance_.GetToken();};
    const Lexer::Token GetNextToken() { return lexer_instance_.GetNextToken();};
    const long int ParseCurrentTokenAsNumber() const { return lexer_instance_.ConvertBufferToNumber();};
    std::string GetBuffer() const { return lexer_instance_.GetBuffer();};
    const int GetLineNo() const { return lexer_instance_.GetLineNo();};
    std::string GetTokenTranslation(Lexer::Token tok) const { return lexer_instance_.GetTokenTranslation(tok);};

    void RaiseParseError(Lexer::Token);
    bool MustParseToken(Lexer::Token);
    
    IdentifierNode* ParseIdentifier();
    TypeDeclNode* ParseTypeDecl();
    VarDeclNode* ParseVariableDecl();
    FunctionDeclNode* ParseFunctionDecl();
    FormalParamNode* ParseFormalParameters();
    FunctionBodyNode* ParseFunctionBody();
    StatSequenceNode* ParseStatementSequence();

    structlog LOGCFG_;
    Lexer& lexer_instance_;
};



} // end namespace papyrus

#endif
