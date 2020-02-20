#include "ASTConstructor.h"

using namespace papyrus;

#define MUSTPARSE(x) MustParseToken(x, __func__, __LINE__)
#define STACKTRACE LOG(DEBUG) << __func__ << ":" << std::to_string(__LINE__)

////////////////////////////////
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
////////////////////////////////

void ASTConstructor::AddSymbol(const IdentifierNode* ident, const TypeDeclNode* type_decl) {
    Symbol *s = new Symbol(ident->GetIdentifierName(),
                           type_decl->GetDimensions(),
                           type_decl->IsArray(),
                           current_scope_ == "main");

    if (current_scope_ == "main") {
        global_symbol_table_[ident->GetIdentifierName()] = s;
    } else {
        local_symbol_table_[ident->GetIdentifierName()] = s;
    }
}

void ASTConstructor::AddSymbol(const IdentifierNode* ident) {
    local_symbol_table_[ident->GetIdentifierName()] = nullptr;
}

////////////////////////////////
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
////////////////////////////////

////////////////////////////////
// Rule: ident = letter {letter | digit}.
////////////////////////////////
IdentifierNode* ASTConstructor::ParseIdentifier() {
    FetchToken();
    MUSTPARSE(Lexer::TOK_IDENT);

    std::string identifier_name = GetBuffer();
    IdentifierNode* ident = new IdentifierNode(identifier_name);
    
    return ident;
}

////////////////////////////////
// Rule: typeDecl = “var” | “array” “[“ number “]” { “[“ number “]” }.
////////////////////////////////
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

            FetchToken();
            MUSTPARSE(Lexer::TOK_NUM);

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

////////////////////////////////
// Rule: varDecl = typeDecl ident { “,” ident } “;”
////////////////////////////////
void ASTConstructor::ParseVariableDecl() {
    TypeDeclNode* type_decl = ParseTypeDecl();
    IdentifierNode* ident = ParseIdentifier();

    AddSymbol(ident, type_decl);

    while (Lexer::TOK_COMMA == PeekNextToken()) {
        FetchToken();
        ident = ParseIdentifier();
        AddSymbol(ident, type_decl);
    }

    FetchToken();
    MUSTPARSE(Lexer::TOK_SEMICOLON);
}

////////////////////////////////
// Rule: formalParam = “(“ [ident { “,” ident }] “)” .
////////////////////////////////
void ASTConstructor::ParseFormalParameters() {
    FetchToken();
    MUSTPARSE(Lexer::TOK_ROUND_OPEN);

    if (Lexer::TOK_IDENT == PeekNextToken()) {
        IdentifierNode* ident = ParseIdentifier();
        AddSymbol(ident);

        while (Lexer::TOK_COMMA == PeekNextToken()) {
            FetchToken();
            ident = ParseIdentifier();
            AddSymbol(ident);
        }
    }

    FetchToken();
    MUSTPARSE(Lexer::TOK_ROUND_CLOSED);
}

////////////////////////////////
// Rule: factor = designator | number | “(“ expression “)” | funcCall .
////////////////////////////////
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

////////////////////////////////
// Rule: term = factor { (“*” | “/”) factor}.
////////////////////////////////
TermNode* ASTConstructor::ParseTerm() {
    TermNode* term = new TermNode(ParseFactor());
    
    while (Lexer::TOK_BINOP_MUL == PeekNextToken() ||
           Lexer::TOK_BINOP_DIV == PeekNextToken()) {
        FetchToken();

        term->AddSecondaryFactor(GetBinOperatorForToken(GetCurrentToken()), ParseFactor());
    }

    return term;
}

////////////////////////////////
// Rule: expression = term {(“+” | “-”) term}.
////////////////////////////////
ExpressionNode* ASTConstructor::ParseExpression() {
    ExpressionNode* expr = new ExpressionNode(ParseTerm());

    while (Lexer::TOK_BINOP_ADD == PeekNextToken() ||
           Lexer::TOK_BINOP_SUB == PeekNextToken()) {
        FetchToken();

        expr->AddSecondaryTerm(GetBinOperatorForToken(GetCurrentToken()), ParseTerm());
    }

    return expr;
}

////////////////////////////////
// Rule: designator = ident{ "[" expression "]" }.
////////////////////////////////
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

////////////////////////////////
// Rule: assignment = “let” designator “<-” expression.
////////////////////////////////
AssignmentNode* ASTConstructor::ParseAssignment() {
    DesignatorNode* desig = ParseDesignator();

    FetchToken();
    MUSTPARSE(Lexer::TOK_LEFTARROW);

    ExpressionNode* value = ParseExpression();

    return new AssignmentNode(desig, value);
}

////////////////////////////////
// Rule: funcCall = “call” ident [ “(“ [expression { “,” expression } ] “)” ].
////////////////////////////////
FunctionCallNode* ASTConstructor::ParseFunctionCall() {
    FunctionCallNode* func_call = new FunctionCallNode(ParseIdentifier());

    if (Lexer::TOK_ROUND_OPEN == PeekNextToken()) {
        FetchToken();

        if (IsExpressionBegin(PeekNextToken())) {
            func_call->AddArgument(ParseExpression());

            while (Lexer::TOK_COMMA == PeekNextToken()) {
                FetchToken();
                func_call->AddArgument(ParseExpression());
            }
        }

        FetchToken();
        MUSTPARSE(Lexer::TOK_ROUND_CLOSED);
    }

    return func_call;
}

////////////////////////////////
// Rule: relation = expression relOp expression .
////////////////////////////////
RelationNode* ASTConstructor::ParseRelation() {
    ExpressionNode* left_expr = ParseExpression();

    if (!IsRelationalOp(PeekNextToken())) {
        RaiseParseError("Expected relational operator in relation");
    }

    FetchToken();
    RelationalOperator rel_op = GetRelOperatorForToken(GetCurrentToken());

    ExpressionNode* right_expr = ParseExpression();

    return new RelationNode(left_expr, rel_op, right_expr);
}

////////////////////////////////
// Rule: ifStatement = “if” relation “then” statSequence [ “else” statSequence ] “fi”.
////////////////////////////////
ITENode* ASTConstructor::ParseITE() {
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

////////////////////////////////
// Rule: whileStatement = “while” relation “do” StatSequence “od”.
////////////////////////////////
WhileNode* ASTConstructor::ParseWhile() {
    RelationNode* loop_condition = ParseRelation();

    FetchToken();
    MUSTPARSE(Lexer::TOK_DO);

    StatSequenceNode* loop_body = ParseStatementSequence();
    
    FetchToken();
    MUSTPARSE(Lexer::TOK_OD);

    return new WhileNode(loop_condition, loop_body);
}

////////////////////////////////
// Rule: returnStatement = “return” [ expression ] .
////////////////////////////////
ReturnNode* ASTConstructor::ParseReturn() {
    ReturnNode* ret = new ReturnNode();

    if (IsExpressionBegin(PeekNextToken())) {
        ret->AddReturnExpression(ParseExpression());
    }

    return ret;
}

////////////////////////////////
// Rule: statement = assignment | funcCall | ifStatement | whileStatement | returnStatement.
////////////////////////////////
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

////////////////////////////////
// Rule: statSequence = statement { “;” statement }.
////////////////////////////////
StatSequenceNode* ASTConstructor::ParseStatementSequence() {
    StatSequenceNode* stat_seq = new StatSequenceNode();

    stat_seq->AddStatementToSequence(ParseStatement());

    while (Lexer::TOK_SEMICOLON == PeekNextToken()) {
        FetchToken();
        stat_seq->AddStatementToSequence(ParseStatement());
    }

    return stat_seq;
}

////////////////////////////////
// Rule: funcBody = { varDecl } “{” [ statSequence ] “}”.
////////////////////////////////
FunctionBodyNode* ASTConstructor::ParseFunctionBody() {
    FunctionBodyNode* func_body = new FunctionBodyNode();

    while (Lexer::TOK_VAR == PeekNextToken() ||
        Lexer::TOK_ARRAY == PeekNextToken()) {
        ParseVariableDecl();
    }

    FetchToken();
    MUSTPARSE(Lexer::TOK_CURLY_OPEN);


    StatSequenceNode* stat_sequence = ParseStatementSequence();
    func_body->SetFunctionBodyStatSequence(stat_sequence);

    FetchToken();
    MUSTPARSE(Lexer::TOK_CURLY_CLOSED);

    return func_body;
}

////////////////////////////////
// Rule: funcDecl = (“function” | “procedure”) ident [formalParam] “;” funcBody “;” .
////////////////////////////////
FunctionDeclNode* ASTConstructor::ParseFunctionDecl() {
    IdentifierNode* ident = ParseIdentifier();

    ////////////////////////////////////////////
    current_scope_ = ident->GetIdentifierName();
    symbol_table_[current_scope_] = {};
    ////////////////////////////////////////////

    if (Lexer::TOK_ROUND_OPEN == PeekNextToken()) {
        ParseFormalParameters();
    }

    FetchToken();
    MUSTPARSE(Lexer::TOK_SEMICOLON);

    FunctionBodyNode* func_body = ParseFunctionBody();

    FunctionDeclNode* func_decl = new FunctionDeclNode(ident, func_body);

    FetchToken();
    MUSTPARSE(Lexer::TOK_SEMICOLON);

    return func_decl;
}

ASTConstructor::ASTConstructor(Lexer& lexer) :
    lexer_instance_(lexer) {}

////////////////////////////////
// Rule: computation = “main” { varDecl } { funcDecl } “{” statSequence “}” “.” .
////////////////////////////////
void ASTConstructor::ConstructAST() {

    FetchToken();
    MUSTPARSE(Lexer::TOK_MAIN);

    ComputationNode* root = new ComputationNode();

    ////////////////////////////////////////////
    current_scope_ = "global";
    ////////////////////////////////////////////
    while (Lexer::TOK_VAR == PeekNextToken() ||
           Lexer::TOK_ARRAY == PeekNextToken()) {
        ParseVariableDecl();
    }

    while (Lexer::TOK_FUNCTION == PeekNextToken() ||
        Lexer::TOK_PROCEDURE == PeekNextToken()) {
        FetchToken();

        root->AddFunctionDecl(ParseFunctionDecl());

        symbol_table_[current_scope_] = local_symbol_table_;
        local_symbol_table_.clear();
    }

    FetchToken();
    MUSTPARSE(Lexer::TOK_CURLY_OPEN);
    
    ////////////////////////////////////////////
    current_scope_ = "global";
    ////////////////////////////////////////////
    if (Lexer::TOK_CURLY_CLOSED != PeekNextToken()) {
        root->SetComputationBody(ParseStatementSequence());
    }

    FetchToken();
    MUSTPARSE(Lexer::TOK_CURLY_CLOSED);

    FetchToken();
    MUSTPARSE(Lexer::TOK_DOT);

    root_ = root;

    return;
}
