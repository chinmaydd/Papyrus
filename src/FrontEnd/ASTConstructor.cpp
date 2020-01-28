#include "ASTConstructor.h"

#include <cassert>

using namespace papyrus;

/*
relOp = “==“ | “!=“ | “<“ | “<=“ | “>“ | “>=“.
ident = letter {letter | digit}.
number = digit {digit}.
designator = ident{ "[" expression "]" }.
factor = designator | number | “(“ expression “)” | funcCall .
term = factor { (“*” | “/”) factor}.
expression = term {(“+” | “-”) term}.
relation = expression relOp expression .
assignment = “let” designator “<-” expression.
funcCall = “call” ident [ “(“ [expression { “,” expression } ] “)” ].
ifStatement = “if” relation “then” statSequence [ “else” statSequence ] “fi”.
whileStatement = “while” relation “do” StatSequence “od”.
returnStatement = “return” [ expression ] .
statement = assignment | funcCall | ifStatement | whileStatement | returnStatement.
statSequence = statement { “;” statement }.
funcBody = { varDecl } “{” [ statSequence ] “}”.
*/

ASTConstructor::ASTConstructor(Lexer& lexer) :
    lexer_instance_(lexer),
    LOGCFG_() {}

void ASTConstructor::RaiseParseError(Lexer::Token expected_tok) {
    LOG(ERROR) << "Error at Line: " << GetLineNo();
    LOG(INFO) << "Expected: " << GetTokenTranslation(expected_tok) << ". Found: " << GetBuffer();
    exit(1);
}

bool ASTConstructor::MustParseToken(Lexer::Token expected_tok) {
    if (expected_tok != GetCurrentToken()) {
        // XXX: Maybe there is a better way to handle this.
        RaiseParseError(expected_tok);
    }
    return true;
}

// ident = letter {letter | digit}.
IdentifierNode* ASTConstructor::ParseIdentifier() {
    GetNextToken();
    MustParseToken(Lexer::TOK_IDENT);

    std::string identifier_name = GetBuffer();
    IdentifierNode* ident = new IdentifierNode(identifier_name);
    return ident;
}

// typeDecl = “var” | “array” “[“ number “]” { “[“ number “]” }.
TypeDeclNode* ASTConstructor::ParseTypeDecl() {
    GetNextToken();
    TypeDeclNode* type_decl = new TypeDeclNode();

    if (Lexer::TOK_VAR == GetCurrentToken()) {
        type_decl->SetIfArray(false);
    } else if (Lexer::TOK_ARRAY == GetCurrentToken()) {
        type_decl->SetIfArray(true);

        GetNextToken();
        MustParseToken(Lexer::TOK_SQUARE_OPEN);

        GetNextToken();
        MustParseToken(Lexer::TOK_NUM);
        ConstantNode* arr_dimension = new ConstantNode(ParseCurrentTokenAsNumber());
        type_decl->AddArrayDimension(arr_dimension);

        GetNextToken();
        MustParseToken(Lexer::TOK_SQUARE_CLOSED);

        while (Lexer::TOK_SQUARE_OPEN == GetNextToken()) {
            GetNextToken();
            arr_dimension = new ConstantNode(ParseCurrentTokenAsNumber());
            type_decl->AddArrayDimension(arr_dimension);

            GetNextToken();
            MustParseToken(Lexer::TOK_SQUARE_CLOSED);
        }

        GetNextToken();
        MustParseToken(Lexer::TOK_SEMICOLON);
    } else {
        RaiseParseError(Lexer::TOK_VARORARR);
    }
    return type_decl;
}

// varDecl = typeDecl ident { “,” ident } “;”
VarDeclNode* ASTConstructor::ParseVariableDecl() {
    TypeDeclNode* type_decl = ParseTypeDecl();
    IdentifierNode* ident = ParseIdentifier();
    VarDeclNode* var_decl = new VarDeclNode(type_decl, ident);

    while (Lexer::TOK_COMMA == GetNextToken())
        var_decl->AddIdentifierDecl(ParseIdentifier());

    GetNextToken();
    MustParseToken(Lexer::TOK_SEMICOLON);

    return var_decl;
}

// formalParam = “(“ [ident { “,” ident }] “)” .
FormalParamNode* ASTConstructor::ParseFormalParameters() {
    GetNextToken();
    MustParseToken(Lexer::TOK_ROUND_OPEN);

    FormalParamNode* formal_param = new FormalParamNode();

    GetNextToken();

    IdentifierNode* ident = nullptr;
    while (Lexer::TOK_COMMA == GetCurrentToken()) {
        ident = ParseIdentifier();
        formal_param->AddFormalParam(ident);

        GetNextToken();
    }

    MustParseToken(Lexer::TOK_ROUND_CLOSED);

    return formal_param;
}

StatSequenceNode* ASTConstructor::ParseStatementSequence() {
    return nullptr;
}

// funcBody = { varDecl } “{” [ statSequence ] “}”.
FunctionBodyNode* ASTConstructor::ParseFunctionBody() {
    return nullptr;
}

// funcDecl = (“function” | “procedure”) ident [formalParam] “;” funcBody “;” .
FunctionDeclNode* ASTConstructor::ParseFunctionDecl() {
    IdentifierNode* ident = ParseIdentifier();
    FormalParamNode* formal_param = ParseFormalParameters();

    GetNextToken();
    MustParseToken(Lexer::TOK_SEMICOLON);

    FunctionBodyNode* func_body = ParseFunctionBody();

    GetNextToken();
    MustParseToken(Lexer::TOK_SEMICOLON);

    return new FunctionDeclNode(ident, formal_param, func_body);
}

// computation = “main” { varDecl } { funcDecl } “{” statSequence “}” “.” .
ComputationNode* ASTConstructor::ComputeAST() {
    ComputationNode* root = new ComputationNode();

    GetNextToken();
    MustParseToken(Lexer::TOK_MAIN);
    GetNextToken();

    if (Lexer::TOK_VAR == GetCurrentToken() ||
        Lexer::TOK_ARRAY == GetCurrentToken()) {
        root->AddGlobalVariableDeclarations(ParseVariableDecl());
        GetNextToken();
    }

    if (Lexer::TOK_FUNCTION == GetCurrentToken() ||
        Lexer::TOK_PROCEDURE == GetCurrentToken()) {
        root->AddFunctionDeclarations(ParseFunctionDecl());
        GetNextToken();
    }

    /*
    ParseToken(TOK_CURLY_OPEN);
    root.SetComputationBody(ParseComputationBody());
    ParseToken(TOK_CURLY_CLOSE);
    */
    return root;
}
