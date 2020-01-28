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
    void RaiseParseError(Lexer::Token);
    bool MustParseToken(Lexer::Token);
    Lexer::Token GetNextToken();
    long int ParseCurrentTokenAsNumber();
    
    IdentifierNode* ParseIdentifier();
    TypeDeclNode* ParseTypeDecl();
    VarDeclNode* ParseVariableDecl();

    structlog LOGCFG_;
    Lexer& lexer_instance_;
};













} // end namespace papyrus

#endif
