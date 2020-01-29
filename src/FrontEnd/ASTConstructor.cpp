#include "ASTConstructor.h"

using namespace papyrus;

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
    FetchToken();
    MustParseToken(Lexer::TOK_IDENT);

    std::string identifier_name = GetBuffer();
    IdentifierNode* ident = new IdentifierNode(identifier_name);
    return ident;
}

// typeDecl = “var” | “array” “[“ number “]” { “[“ number “]” }.
TypeDeclNode* ASTConstructor::ParseTypeDecl() {
    FetchToken();

    TypeDeclNode* type_decl = new TypeDeclNode();

    if (Lexer::TOK_VAR == GetCurrentToken()) {
        type_decl->SetIfArray(false);
    } else if (Lexer::TOK_ARRAY == GetCurrentToken()) {
        type_decl->SetIfArray(true);

        FetchToken();
        MustParseToken(Lexer::TOK_SQUARE_OPEN);
        FetchToken();
        MustParseToken(Lexer::TOK_NUM);
        ConstantNode* arr_dimension = new ConstantNode(ParseCurrentTokenAsNumber());
        FetchToken();
        MustParseToken(Lexer::TOK_SQUARE_CLOSED);

        type_decl->AddArrayDimension(arr_dimension);

        while (Lexer::TOK_SQUARE_OPEN == PeekNextToken()) {
            FetchToken();

            arr_dimension = new ConstantNode(ParseCurrentTokenAsNumber());
            type_decl->AddArrayDimension(arr_dimension);

            FetchToken();
            MustParseToken(Lexer::TOK_SQUARE_CLOSED);
        }

        FetchToken();
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

    while (Lexer::TOK_COMMA == PeekNextToken()) {
        FetchToken();
        var_decl->AddIdentifierDecl(ParseIdentifier());
    }

    FetchToken();
    MustParseToken(Lexer::TOK_SEMICOLON);

    return var_decl;
}

// formalParam = “(“ [ident { “,” ident }] “)” .
FormalParamNode* ASTConstructor::ParseFormalParameters() {
    FetchToken();
    MustParseToken(Lexer::TOK_ROUND_OPEN);

    FormalParamNode* formal_param = new FormalParamNode();

    IdentifierNode* ident;
    while (Lexer::TOK_COMMA == PeekNextToken()) {
        FetchToken();
        ident = ParseIdentifier();
        formal_param->AddFormalParam(ident);
    }

    FetchToken();
    MustParseToken(Lexer::TOK_ROUND_CLOSED);

    return formal_param;
}

// expression = term {(“+” | “-”) term}.
// term = factor { (“*” | “/”) factor}.
// factor = designator | number | “(“ expression “)” | funcCall .
ExpressionNode* ASTConstructor::ParseExpression() {
    ExpressionNode* expr = new ExpressionNode();
    ExpressionNode* left_term = new ExpressionNode();

    if (Lexer::TOK_IDENT == PeekNextToken()) {
    } else if (Lexer::TOK_NUM == PeekNextToken()) {
    } else if (Lexer::TOK_ROUND_OPEN == PeekNextToken()) {
        FetchToken();
        expr->SetPrimaryTerm(ParseExpression());

        FetchToken();
        MustParseToken(Lexer::TOK_ROUND_CLOSED);
    } else {
        RaiseParseError("Failed to parse expression");
    }

    return expr;
}

// designator = ident{ "[" expression "]" }.
DesignatorNode* ASTConstructor::ParseDesignator() {
    IdentifierNode* ident = ParseIdentifier();

    if (Lexer::TOK_SQUARE_OPEN == PeekNextToken()) {
        FetchToken();
        ArrIdentifierNode* designator = new ArrIdentifierNode(ident);

        while (Lexer::TOK_SQUARE_OPEN == PeekNextToken()) {
            FetchToken();
            designator->AddIndirectionToArray(ParseExpression());

            FetchToken();
            MustParseToken(Lexer::TOK_SQUARE_CLOSED);
        }

        return designator;
    } else {
        VarIdentifierNode* designator = new VarIdentifierNode(ident);

        return designator;
    }
}

// assignment = “let” designator “<-” expression.
AssignmentNode* ASTConstructor::ParseAssignment() {
    MustParseToken(Lexer::TOK_LET);

    DesignatorNode* desig = ParseDesignator();

    FetchToken();
    MustParseToken(Lexer::TOK_LEFTARROW);

    ExpressionNode* value = ParseExpression();

    return new AssignmentNode(desig, value);
}

// funcCall = “call” ident [ “(“ [expression { “,” expression } ] “)” ].
FunctionCallNode* ASTConstructor::ParseFunctionCall() {
    MustParseToken(Lexer::TOK_CALL);

    FunctionCallNode* func_call = new FunctionCallNode(ParseIdentifier());

    if (Lexer::TOK_ROUND_OPEN == PeekNextToken()) {
        FetchToken();
        func_call->AddArgument(ParseExpression());

        while (Lexer::TOK_COMMA == PeekNextToken()) {
            FetchToken();
            func_call->AddArgument(ParseExpression());
        }

        MustParseToken(Lexer::TOK_ROUND_CLOSED);
    }

    return func_call;
}

// relation = expression relOp expression .
RelationNode* ASTConstructor::ParseRelation() {
    ExpressionNode* left_expr = ParseExpression();

    FetchToken();
    if (!IsRelationalOp(GetCurrentToken())) {
        RaiseParseError("Expected relational operator in relation");
    }

    RelationalOperator rel_op = GetOperatorForToken(GetCurrentToken());
    if (RELOP_NONE == rel_op) {
        RaiseParseError("Failed to identify relational operator");
    }
    FetchToken();

    ExpressionNode* right_expr = ParseExpression();

    return new RelationNode(left_expr, rel_op, right_expr);
}

// ifStatement = “if” relation “then” statSequence [ “else” statSequence ] “fi”.
ITENode* ASTConstructor::ParseITE() {
    MustParseToken(Lexer::TOK_IF);

    RelationNode* condition = ParseRelation();
    
    FetchToken();
    MustParseToken(Lexer::TOK_THEN);

    StatSequenceNode* if_clause = ParseStatementSequence();

    ITENode* ite = new ITENode(condition, if_clause);

    FetchToken();
    if (Lexer::TOK_ELSE == GetCurrentToken()) {
        ite->AddElseClause(ParseStatementSequence());
    }

    FetchToken();
    MustParseToken(Lexer::TOK_FI);

    return ite;
}

// whileStatement = “while” relation “do” StatSequence “od”.
WhileNode* ASTConstructor::ParseWhile() {
    MustParseToken(Lexer::TOK_WHILE);

    RelationNode* loop_condition = ParseRelation();

    FetchToken();
    MustParseToken(Lexer::TOK_DO);

    StatSequenceNode* loop_body = ParseStatementSequence();
    
    FetchToken();
    MustParseToken(Lexer::TOK_OD);

    return new WhileNode(loop_condition, loop_body);
}

// returnStatement = “return” [ expression ] .
ReturnNode* ASTConstructor::ParseReturn() {
    MustParseToken(Lexer::TOK_RETURN);
    return nullptr;
}

// statement = assignment | funcCall | ifStatement | whileStatement | returnStatement.
StatementNode* ASTConstructor::ParseStatement() {
    StatementNode* statement;

    FetchToken();
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

    while (Lexer::TOK_SEMICOLON == PeekNextToken()) {
        FetchToken();
        stat_seq->AddStatementToSequence(ParseStatement());
    }

    return stat_seq;
}

// funcBody = { varDecl } “{” [ statSequence ] “}”.
FunctionBodyNode* ASTConstructor::ParseFunctionBody() {
    FunctionBodyNode* func_body = new FunctionBodyNode();

    if (Lexer::TOK_VAR == PeekNextToken() ||
        Lexer::TOK_ARRAY == PeekNextToken()) {
        func_body->AddVariableDecl(ParseVariableDecl());
    }

    FetchToken();
    MustParseToken(Lexer::TOK_CURLY_OPEN);

    StatSequenceNode* stat_sequence = ParseStatementSequence();
    func_body->SetFunctionBodyStatSequence(stat_sequence);

    FetchToken();
    MustParseToken(Lexer::TOK_CURLY_CLOSED);

    return func_body;
}

// funcDecl = (“function” | “procedure”) ident [formalParam] “;” funcBody “;” .
FunctionDeclNode* ASTConstructor::ParseFunctionDecl() {
    FetchToken();

    IdentifierNode* ident = ParseIdentifier();
    FormalParamNode* formal_param = ParseFormalParameters();

    FetchToken();
    MustParseToken(Lexer::TOK_SEMICOLON);

    FunctionBodyNode* func_body = ParseFunctionBody();

    FetchToken();
    MustParseToken(Lexer::TOK_SEMICOLON);

    return new FunctionDeclNode(ident, formal_param, func_body);
}

// computation = “main” { varDecl } { funcDecl } “{” statSequence “}” “.” .
ComputationNode* ASTConstructor::ComputeAST() {
    ComputationNode* root = new ComputationNode();

    FetchToken();
    MustParseToken(Lexer::TOK_MAIN);

    while (Lexer::TOK_VAR == PeekNextToken() ||
           Lexer::TOK_ARRAY == PeekNextToken()) {
        root->AddGlobalVariableDecl(ParseVariableDecl());
    }

    if (Lexer::TOK_FUNCTION == PeekNextToken() ||
        Lexer::TOK_PROCEDURE == PeekNextToken()) {
        root->AddFunctionDecl(ParseFunctionDecl());
    }

    /*
    ParseToken(TOK_CURLY_OPEN);
    root.SetComputationBody(ParseComputationBody());
    ParseToken(TOK_CURLY_CLOSE);
    */
    return root;
}
