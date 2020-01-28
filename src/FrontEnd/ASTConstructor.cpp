#include "ASTConstructor.h"

#include <cassert>

using namespace papyrus;

/*
 * letter = “a” | “b” | … | “z”.
digit = “0” | “1” | … | “9”.
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
typeDecl = “var” | “array” “[“ number “]” { “[“ number “]” }.
varDecl = typeDecl indent { “,” ident } “;” .
funcDecl = (“function” | “procedure”) ident [formalParam] “;” funcBody “;” .
formalParam = “(“ [ident { “,” ident }] “)” .
funcBody = { varDecl } “{” [ statSequence ] “}”.
computation = “main” { varDecl } { funcDecl } “{” statSequence “}” “.” .
*/

ASTConstructor::ASTConstructor(Lexer& lexer) :
    lexer_instance_(lexer),
    LOGCFG_() {}

void ASTConstructor::RaiseParseError(Lexer::Token expected_tok) {
    LOG(ERROR) << "Error at Line: " << lexer_instance_.GetLineNo();
    LOG(INFO) << "Expected: " << lexer_instance_.GetTokenTranslation(expected_tok) << ". Found: " << lexer_instance_.GetBuffer();
    exit(1);
}

Lexer::Token ASTConstructor::GetNextToken() {
    return lexer_instance_.GetNextToken();
}

long int ASTConstructor::ParseCurrentTokenAsNumber() {
    // XXX: Take care here?
    return lexer_instance_.ConvertBufferToNumber();
}

bool ASTConstructor::MustParseToken(Lexer::Token expected_tok) {
    if (expected_tok != lexer_instance_.GetNextToken()) {
        // XXX: Maybe there is a better way to handle this.
        RaiseParseError(expected_tok);
    }

    return true;
}

// ident = letter {letter | digit}.
IdentifierNode* ASTConstructor::ParseIdentifier() {
    MustParseToken(Lexer::TOK_IDENT);

    std::string identifier_name = lexer_instance_.GetBuffer();
    IdentifierNode* ident = new IdentifierNode(identifier_name);

    return ident;
}

// typeDecl = “var” | “array” “[“ number “]” { “[“ number “]” }.
// varDecl = typeDecl indent { “,” ident } “;” .
TypeDeclNode* ASTConstructor::ParseTypeDecl() {
    GetNextToken();
    TypeDeclNode* type_decl = new TypeDeclNode();

    if (Lexer::TOK_VAR == lexer_instance_.GetToken()) {
        type_decl->SetIfArray(false);
    } else if (Lexer::TOK_ARRAY == lexer_instance_.GetToken()) {
        type_decl->SetIfArray(true);

        MustParseToken(Lexer::TOK_SQUARE_OPEN);
        MustParseToken(Lexer::TOK_NUM);
        ConstantNode* arr_dimension = new ConstantNode(ParseCurrentTokenAsNumber());
        type_decl->AddArrayDimension(arr_dimension);
        MustParseToken(Lexer::TOK_SQUARE_CLOSED);

        while (Lexer::TOK_SQUARE_OPEN == GetNextToken()) {
            GetNextToken();
            arr_dimension = new ConstantNode(ParseCurrentTokenAsNumber());
            type_decl->AddArrayDimension(arr_dimension);
            MustParseToken(Lexer::TOK_SQUARE_CLOSED);
        }

        MustParseToken(Lexer::TOK_SEMICOLON);
    } else {
        RaiseParseError(Lexer::TOK_VARORARR);
    }

    return type_decl;
}

// varDecl = typeDecl ident { “,” ident } “;”
VarDeclNode* ASTConstructor::ParseVariableDecl() {
    TypeDeclNode* type_decl = ParseTypeDecl();
    IdentifierNode* default_ident = ParseIdentifier();

    VarDeclNode* var_decl = new VarDeclNode(type_decl, default_ident);

    while (Lexer::TOK_COMMA == GetNextToken())
        var_decl->AddIdentifierDecl(ParseIdentifier());

    MustParseToken(Lexer::TOK_SEMICOLON);

    return var_decl;
}

ComputationNode* ASTConstructor::ComputeAST() {
    ComputationNode* root = new ComputationNode();

    MustParseToken(Lexer::TOK_MAIN);
    root->AddGlobalVariableDeclarations(ParseVariableDecl());
    /*
    root.AddFunctionDeclarations(ParseFunctionDecl());
    ParseToken(TOK_CURLY_OPEN);
    root.SetComputationBody(ParseComputationBody());
    ParseToken(TOK_CURLY_CLOSE);
    */
    return root;
}
