#include "ASTConstructor.h"

#include <cassert>

using namespace papyrus;

ASTConstructor::ASTConstructor(Lexer& lexer) :
    lexer_instance_(lexer),
    LOGCFG_() {}

void ASTConstructor::ParseSingleToken(Lexer::Token expected_tok) {
    if (expected_tok != lexer_instance_.GetNextToken()) {
        LOG(ERROR) << "Error at Line: " << lexer_instance_.GetLineNo();
        LOG(INFO) << "Expected: " << lexer_instance_.GetTokenTranslation(expected_tok) << ". Found: " << lexer_instance_.GetBuffer();
        exit(1);
    }
}

// varDecl = typeDecl ident { “,” ident } “;”
VarDeclNode* ASTConstructor::ParseVariableDecl() {
}

ComputationNode* ASTConstructor::ComputeAST() {
    ComputationNode* root = new ComputationNode();

    ParseSingleToken(Lexer::TOK_MAIN);
    root->AddGlobalVariableDeclarations(ParseVariableDecl());
    /*
    root.AddFunctionDeclarations(ParseFunctionDecl());
    ParseToken(TOK_CURLY_OPEN);
    root.SetComputationBody(ParseComputationBody());
    ParseToken(TOK_CURLY_CLOSE);
    */
    return root;
}
