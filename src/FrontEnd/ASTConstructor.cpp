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

*/

ASTConstructor::ASTConstructor(Lexer& lexer) :
    lexer_instance_(lexer),
    LOGCFG_() {}

void ASTConstructor::RaiseParseError(const Lexer::Token& expected_tok) const {
    LOG(ERROR) << "[PARSER] Error at Line: " << GetLineNo();
    LOG(ERROR) << "[PARSER] Expected: " << GetTokenTranslation(expected_tok) << ". Found: " << GetBuffer();
    exit(1);
}

void ASTConstructor::RaiseParseError(const std::string& error_msg) const {
    LOG(ERROR) << "[PARSER] Error at Line: " << GetLineNo();
    LOG(ERROR) << "[PARSER] " << error_msg;
    exit(1);
}

bool ASTConstructor::MustParseToken(const Lexer::Token& expected_tok) const {
    if (expected_tok != GetCurrentToken()) {
        // XXX: Maybe there is a better way to handle this.
        RaiseParseError(expected_tok);
    }
    return true;
}

bool ASTConstructor::IsStatementBegin(const Lexer::Token& token) const {
    return (Lexer::TOK_LET    == token || 
            Lexer::TOK_CALL   == token ||  
            Lexer::TOK_IF     == token || 
            Lexer::TOK_WHILE  == token || 
            Lexer::TOK_RETURN == token);
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

// expression = term {(“+” | “-”) term}.
ExpressionNode* ASTConstructor::ParseExpression() {
    return nullptr;
}

// designator = ident{ "[" expression "]" }.
DesignatorNode* ASTConstructor::ParseDesignator() {
    return nullptr;
}

// assignment = “let” designator “<-” expression.
AssignmentNode* ASTConstructor::ParseAssignment() {
    MustParseToken(Lexer::TOK_LET);

    DesignatorNode* desig = ParseDesignator();

    GetNextToken();
    MustParseToken(Lexer::TOK_LEFTARROW);

    ExpressionNode* value = ParseExpression();

    return new AssignmentNode(desig, value);
}

// funcCall = “call” ident [ “(“ [expression { “,” expression } ] “)” ].
FunctionCallNode* ASTConstructor::ParseFunctionCall() {
    MustParseToken(Lexer::TOK_CALL);

    FunctionCallNode* func_call = new FunctionCallNode(ParseIdentifier());

    GetNextToken();
    if (Lexer::TOK_ROUND_OPEN == GetCurrentToken()) {
        func_call->AddArgument(ParseExpression());

        GetNextToken();
        while (Lexer::TOK_COMMA == GetCurrentToken()) {
            func_call->AddArgument(ParseExpression());

            GetNextToken();
        }

        MustParseToken(Lexer::TOK_ROUND_CLOSED);
    }
    return func_call;
}

// relation = expression relOp expression .
RelationNode* ASTConstructor::ParseRelation() {
    ExpressionNode* left_expr = ParseExpression();

    GetNextToken();
    if (!IsRelationalOp(GetCurrentToken())) {
        RaiseParseError("Expected relational operator in relation");
    }

    RelationalOperator rel_op = GetOperatorForToken(GetCurrentToken());

    if (RELOP_NONE == rel_op) {
        RaiseParseError("Failed to identify relational operator");
    }

    ExpressionNode* right_expr = ParseExpression();

    return new RelationNode(left_expr, rel_op, right_expr);
}

// ifStatement = “if” relation “then” statSequence [ “else” statSequence ] “fi”.
ITENode* ASTConstructor::ParseITE() {
    MustParseToken(Lexer::TOK_IF);

    RelationNode* condition = ParseRelation();
    
    GetNextToken();
    MustParseToken(Lexer::TOK_THEN);

    StatSequenceNode* if_clause = ParseStatementSequence();

    ITENode* ite = new ITENode(condition, if_clause);

    GetNextToken();
    if (Lexer::TOK_ELSE == GetCurrentToken()) {
        ite->AddElseClause(ParseStatementSequence());
    }

    GetNextToken();
    MustParseToken(Lexer::TOK_FI);

    return ite;
}

// whileStatement = “while” relation “do” StatSequence “od”.
WhileNode* ASTConstructor::ParseWhile() {
    MustParseToken(Lexer::TOK_WHILE);

    RelationNode* loop_condition = ParseRelation();

    GetNextToken();
    MustParseToken(Lexer::TOK_DO);

    StatSequenceNode* loop_body = ParseStatementSequence();
    
    GetNextToken();
    MustParseToken(Lexer::TOK_OD);

    return new WhileNode(loop_condition, loop_body);
}

// returnStatement = “return” [ expression ] .
ReturnNode* ASTConstructor::ParseReturn() {
    MustParseToken(Lexer::TOK_RETURN);


    return nullptr;
}

/*
   statement = assignment | funcCall | ifStatement | whileStatement | returnStatement.
*/
StatementNode* ASTConstructor::ParseStatement() {
    GetNextToken();
    StatementNode* statement = nullptr;

    if (Lexer::TOK_LET == GetCurrentToken()) {
        statement = ParseAssignment();
    } else if (Lexer::TOK_CALL == GetCurrentToken()) {
        statement = ParseFunctionCall();
    } else if (Lexer::TOK_IF == GetCurrentToken()) {
        statement = ParseITE();
    } else if (Lexer::TOK_WHILE == GetCurrentToken()) {
        statement = ParseWhile();
    } else if (Lexer::TOK_RETURN == GetCurrentToken()) {
        statement = ParseReturn();
    } else {
        RaiseParseError("Statement not valid");
    }

    return statement;
}

// statSequence = statement { “;” statement }.
StatSequenceNode* ASTConstructor::ParseStatementSequence() {
    StatSequenceNode* stat_seq = new StatSequenceNode();

    stat_seq->AddStatementToSequence(ParseStatement());

    GetNextToken();
    if (Lexer::TOK_SEMICOLON == GetCurrentToken()) {
        stat_seq->AddStatementToSequence(ParseStatement());
    }

    return stat_seq;
}

// funcBody = { varDecl } “{” [ statSequence ] “}”.
FunctionBodyNode* ASTConstructor::ParseFunctionBody() {
    GetNextToken();
    FunctionBodyNode* func_body = new FunctionBodyNode();

    if (Lexer::TOK_VAR == GetCurrentToken() ||
        Lexer::TOK_ARRAY == GetCurrentToken()) {
        func_body->AddVariableDecl(ParseVariableDecl());
        GetNextToken();
    }

    MustParseToken(Lexer::TOK_CURLY_OPEN);

    StatSequenceNode* stat_sequence = ParseStatementSequence();
    func_body->SetFunctionBodyStatSequence(stat_sequence);

    GetNextToken();
    MustParseToken(Lexer::TOK_CURLY_CLOSED);

    return func_body;
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
        root->AddGlobalVariableDecl(ParseVariableDecl());
        GetNextToken();
    }

    if (Lexer::TOK_FUNCTION == GetCurrentToken() ||
        Lexer::TOK_PROCEDURE == GetCurrentToken()) {
        root->AddFunctionDecl(ParseFunctionDecl());
        GetNextToken();
    }

    /*
    ParseToken(TOK_CURLY_OPEN);
    root.SetComputationBody(ParseComputationBody());
    ParseToken(TOK_CURLY_CLOSE);
    */
    return root;
}
