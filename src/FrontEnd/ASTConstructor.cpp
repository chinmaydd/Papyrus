#include "ASTConstructor.h"

using namespace papyrus;

#define MUSTPARSE(x) MustParseToken(x, __func__, __LINE__)
// #define STACKTRACE LOG(DEBUG) << __func__ << ":" << std::to_string(__LINE__)
// #define STACKTRACE LOG(DEBUG) << ""

ASTConstructor::ASTConstructor(Lexer& lexer) :
    lexer_instance_(lexer),
    LOGCFG_() {}

void ASTConstructor::RaiseParseError(const Lexer::Token& expected_tok) const {
    LOG(ERROR) << "[PARSER] Error at Line: " << GetLineNo();
    LOG(ERROR) << "[PARSER] Expected: " << GetTokenTranslation(expected_tok) << ", found: " << GetBuffer();
    exit(1);
}

void ASTConstructor::RaiseParseError(const std::string& error_msg) const {
    LOG(ERROR) << "[PARSER] Error at Line: " << GetLineNo();
    LOG(ERROR) << "[PARSER] " << error_msg;
    exit(1);
}

bool ASTConstructor::MustParseToken(const Lexer::Token& expected_tok, const std::string& func, int line) const {
    if (expected_tok != GetCurrentToken()) {
        // XXX: Maybe there is a better way to handle this.
        LOG(ERROR) << "[DEBUG] " << func << ", Line: " << std::to_string(line);
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

bool ASTConstructor::IsExpressionBegin(const Lexer::Token& token) const {
    return (Lexer::TOK_IDENT      == token ||
            Lexer::TOK_NUM        == token ||
            Lexer::TOK_ROUND_OPEN == token ||
            Lexer::TOK_CALL       == token);

}

// ident = letter {letter | digit}.
IdentifierNode* ASTConstructor::ParseIdentifier() {
    
    FetchToken();
    MUSTPARSE(Lexer::TOK_IDENT);

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
        MUSTPARSE(Lexer::TOK_SQUARE_OPEN);
        FetchToken();
        MUSTPARSE(Lexer::TOK_NUM);
        ConstantNode* arr_dimension = new ConstantNode(ParseCurrentTokenAsNumber());
        FetchToken();
        MUSTPARSE(Lexer::TOK_SQUARE_CLOSED);

        type_decl->AddArrayDimension(arr_dimension);

        while (Lexer::TOK_SQUARE_OPEN == PeekNextToken()) {
            FetchToken();

            arr_dimension = new ConstantNode(ParseCurrentTokenAsNumber());
            type_decl->AddArrayDimension(arr_dimension);

            FetchToken();
            MUSTPARSE(Lexer::TOK_SQUARE_CLOSED);
        }
    } else {
        MUSTPARSE(Lexer::TOK_VARORARR);
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
    MUSTPARSE(Lexer::TOK_SEMICOLON);

    return var_decl;
}

// formalParam = “(“ [ident { “,” ident }] “)” .
FormalParamNode* ASTConstructor::ParseFormalParameters() {
    
    FetchToken();
    MUSTPARSE(Lexer::TOK_ROUND_OPEN);

    FormalParamNode* formal_param = new FormalParamNode();

    if (Lexer::TOK_IDENT == PeekNextToken()) {
        IdentifierNode* ident = ParseIdentifier();
        formal_param->AddFormalParam(ident);

        while (Lexer::TOK_COMMA == PeekNextToken()) {
            FetchToken();
            ident = ParseIdentifier();
            formal_param->AddFormalParam(ident);
        }
    }

    FetchToken();
    MUSTPARSE(Lexer::TOK_ROUND_CLOSED);

    return formal_param;
}

// factor = designator | number | “(“ expression “)” | funcCall .
FactorNode* ASTConstructor::ParseFactor() {
    
    FactorNode* fact;
    if (Lexer::TOK_IDENT == PeekNextToken()) {

        fact = new FactorNode(ParseDesignator());
    } else if (Lexer::TOK_NUM == PeekNextToken()) {
        FetchToken();

        fact = new FactorNode(new ConstantNode(ParseCurrentTokenAsNumber()));
    } else if (Lexer::TOK_ROUND_OPEN == PeekNextToken()) {
        FetchToken();

        fact = new FactorNode(ParseExpression());

        FetchToken();
        MUSTPARSE(Lexer::TOK_ROUND_CLOSED);
    } else if (Lexer::TOK_CALL == PeekNextToken()) {
        FetchToken();

        fact = new FactorNode(ParseFunctionCall());
    } else {
        RaiseParseError("Could not parse factor.");
    }

    return fact;
}

// term = factor { (“*” | “/”) factor}.
TermNode* ASTConstructor::ParseTerm() {
    
    TermNode* term = new TermNode(ParseFactor());
    
    while (Lexer::TOK_BINOP_MUL == PeekNextToken() ||
           Lexer::TOK_BINOP_DIV == PeekNextToken()) {
        FetchToken();

        term->AddSecondaryFactor(GetBinOperatorForToken(GetCurrentToken()), ParseFactor());
    }

    return term;
}

// expression = term {(“+” | “-”) term}.
ExpressionNode* ASTConstructor::ParseExpression() {
    
    ExpressionNode* expr = new ExpressionNode(ParseTerm());

    while (Lexer::TOK_BINOP_ADD == PeekNextToken() ||
           Lexer::TOK_BINOP_SUB == PeekNextToken()) {
        FetchToken();

        expr->AddSecondaryTerm(GetBinOperatorForToken(GetCurrentToken()), ParseTerm());
    }

    return expr;
}

// designator = ident{ "[" expression "]" }.
DesignatorNode* ASTConstructor::ParseDesignator() {
    
    IdentifierNode* ident = ParseIdentifier();

    if (Lexer::TOK_SQUARE_OPEN == PeekNextToken()) {
        ArrIdentifierNode* designator = new ArrIdentifierNode(ident);

        while (Lexer::TOK_SQUARE_OPEN == PeekNextToken()) {
            FetchToken();
            designator->AddIndirectionToArray(ParseExpression());

            FetchToken();
            MUSTPARSE(Lexer::TOK_SQUARE_CLOSED);
        }

        return designator;
    } else {
        VarIdentifierNode* designator = new VarIdentifierNode(ident);

        return designator;
    }
}

// assignment = “let” designator “<-” expression.
AssignmentNode* ASTConstructor::ParseAssignment() {
    
    MUSTPARSE(Lexer::TOK_LET);

    DesignatorNode* desig = ParseDesignator();

    FetchToken();
    MUSTPARSE(Lexer::TOK_LEFTARROW);

    ExpressionNode* value = ParseExpression();

    return new AssignmentNode(desig, value);
}

// funcCall = “call” ident [ “(“ [expression { “,” expression } ] “)” ].
FunctionCallNode* ASTConstructor::ParseFunctionCall() {
    
    MUSTPARSE(Lexer::TOK_CALL);

    FunctionCallNode* func_call = new FunctionCallNode(ParseIdentifier());

    if (Lexer::TOK_ROUND_OPEN == PeekNextToken()) {
        FetchToken();
        func_call->AddArgument(ParseExpression());

        while (Lexer::TOK_COMMA == PeekNextToken()) {
            FetchToken();
            func_call->AddArgument(ParseExpression());
        }

        FetchToken();
        MUSTPARSE(Lexer::TOK_ROUND_CLOSED);
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

    RelationalOperator rel_op = GetRelOperatorForToken(GetCurrentToken());

    ExpressionNode* right_expr = ParseExpression();

    return new RelationNode(left_expr, rel_op, right_expr);
}

// ifStatement = “if” relation “then” statSequence [ “else” statSequence ] “fi”.
ITENode* ASTConstructor::ParseITE() {
    
    MUSTPARSE(Lexer::TOK_IF);

    RelationNode* condition = ParseRelation();
    
    FetchToken();
    MUSTPARSE(Lexer::TOK_THEN);

    StatSequenceNode* if_clause = ParseStatementSequence();

    ITENode* ite = new ITENode(condition, if_clause);

    if (Lexer::TOK_ELSE == PeekNextToken()) {
        FetchToken();
        ite->AddElseClause(ParseStatementSequence());
    }

    FetchToken();
    MUSTPARSE(Lexer::TOK_FI);

    return ite;
}

// whileStatement = “while” relation “do” StatSequence “od”.
WhileNode* ASTConstructor::ParseWhile() {
    
    MUSTPARSE(Lexer::TOK_WHILE);

    RelationNode* loop_condition = ParseRelation();

    FetchToken();
    MUSTPARSE(Lexer::TOK_DO);

    StatSequenceNode* loop_body = ParseStatementSequence();
    
    FetchToken();
    MUSTPARSE(Lexer::TOK_OD);

    return new WhileNode(loop_condition, loop_body);
}

// returnStatement = “return” [ expression ] .
ReturnNode* ASTConstructor::ParseReturn() {

    MUSTPARSE(Lexer::TOK_RETURN);

    ReturnNode* ret = new ReturnNode();

    if (IsExpressionBegin(PeekNextToken())) {
        ret->AddReturnExpression(ParseExpression());
    }

    return ret;
}

// statement = assignment | funcCall | ifStatement | whileStatement | returnStatement.
StatementNode* ASTConstructor::ParseStatement() {
    
    StatementNode* statement;

    FetchToken();
    if (Lexer::TOK_LET == GetCurrentToken()) {
        statement = static_cast<StatementNode*>(ParseAssignment());
    } else if (Lexer::TOK_CALL == GetCurrentToken()) {
        statement = static_cast<StatementNode*>(ParseFunctionCall());
    } else if (Lexer::TOK_IF == GetCurrentToken()) {
        statement = static_cast<StatementNode*>(ParseITE());
    } else if (Lexer::TOK_WHILE == GetCurrentToken()) {
        statement = static_cast<StatementNode*>(ParseWhile());
    } else if (Lexer::TOK_RETURN == GetCurrentToken()) {
        statement = static_cast<StatementNode*>(ParseReturn());
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

    while (Lexer::TOK_VAR == PeekNextToken() ||
        Lexer::TOK_ARRAY == PeekNextToken()) {
        func_body->AddVariableDecl(ParseVariableDecl());
    }

    FetchToken();
    MUSTPARSE(Lexer::TOK_CURLY_OPEN);

    StatSequenceNode* stat_sequence = ParseStatementSequence();
    func_body->SetFunctionBodyStatSequence(stat_sequence);

    FetchToken();
    MUSTPARSE(Lexer::TOK_CURLY_CLOSED);

    return func_body;
}

// funcDecl = (“function” | “procedure”) ident [formalParam] “;” funcBody “;” .
FunctionDeclNode* ASTConstructor::ParseFunctionDecl() {
    
    FetchToken();

    IdentifierNode* ident = ParseIdentifier();
    FormalParamNode* formal_param = ParseFormalParameters();

    FetchToken();
    MUSTPARSE(Lexer::TOK_SEMICOLON);

    FunctionBodyNode* func_body = ParseFunctionBody();

    FetchToken();
    MUSTPARSE(Lexer::TOK_SEMICOLON);

    return new FunctionDeclNode(ident, formal_param, func_body);
}

// computation = “main” { varDecl } { funcDecl } “{” statSequence “}” “.” .
ComputationNode* ASTConstructor::ComputeAST() {
    ComputationNode* root = new ComputationNode();

    FetchToken();
    MUSTPARSE(Lexer::TOK_MAIN);

    while (Lexer::TOK_VAR == PeekNextToken() ||
           Lexer::TOK_ARRAY == PeekNextToken()) {
        root->AddGlobalVariableDecl(ParseVariableDecl());
    }

    while (Lexer::TOK_FUNCTION == PeekNextToken() ||
        Lexer::TOK_PROCEDURE == PeekNextToken()) {
        root->AddFunctionDecl(ParseFunctionDecl());
    }

    FetchToken();
    MUSTPARSE(Lexer::TOK_CURLY_OPEN);
    
    root->SetComputationBody(ParseStatementSequence());

    FetchToken();
    MUSTPARSE(Lexer::TOK_CURLY_CLOSED);

    return root;
}
