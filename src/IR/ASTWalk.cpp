#include "ASTWalk.h"

using namespace papyrus;

#define CF irc.CurrentFunction()
#define NOTFOUND -1

ValueIndex ConstantNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "Parsing constant";

    return CF->CreateConstant(value_);
}

ValueIndex FactorNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "Parsing factor";

    ValueIndex result = NOTFOUND;
    switch(factor_type_) {
        case FACT_DESIGNATOR: {
            // TODO: ReadVariable()
            break;
        }
        case FACT_NUMBER: {
            auto cnstn = static_cast<const ConstantNode*>(factor_node_);
            result = cnstn->GenerateIR(irc);
            break;
        }
        case FACT_EXPR: {
            auto exprn = static_cast<const ExpressionNode*>(factor_node_);
            result = exprn->GenerateIR(irc);
            break;
        }
        case FACT_FUNCCALL: {
            auto fncn = static_cast<const FunctionCallNode*>(factor_node_);
            result = fncn->GenerateIR(irc);
            break;
        }
    }

    return result;
}

ValueIndex TermNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "Parsing term";

    ValueIndex idx_1, idx_2;
    idx_1 = primary_factor_->GenerateIR(irc);

    ArithmeticOperator op;
    const FactorNode* fact;
    for (auto next_pair: secondary_factors_) {
        op    = next_pair.first;
        fact  = next_pair.second;

        idx_2 = fact->GenerateIR(irc);
        idx_1 = CF->MakeInstruction(irc.ConvertOperation(op),
                                    idx_1,
                                    idx_2);
    }

    return idx_1;
}

ValueIndex ExpressionNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "Parsing expression";

    ValueIndex idx_1, idx_2;
    idx_1 = primary_term_->GenerateIR(irc);

    ArithmeticOperator op;
    const TermNode* term;
    for (auto next_pair: secondary_terms_) {
        op    = next_pair.first;
        term  = next_pair.second;

        idx_2 = term->GenerateIR(irc);
        idx_1 = CF->MakeInstruction(irc.ConvertOperation(op),
                                    idx_1,
                                    idx_2);
    }

    return idx_1;
}

ValueIndex ArrIdentifierNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "Parsing ArrIdentifier";

    std::string var_name = GetIdentifierName();
    ValueIndex base, offset, temp;

    const Variable* var;
    if (CF->IsVariableLocal(var_name)) {
        var    = CF->GetVariable(var_name);
        base   = CF->LocalBase();
        offset = CF->CreateConstant(var->Offset());
    } else {
        var    = irc.GetGlobal(var_name);
        base   = irc.GlobalBase();
        offset = CF->CreateConstant(irc.GlobalOffset(var_name));
    }

    ValueIndex arr_base = CF->MakeInstruction(T::INS_ADDA, 
                                              offset,
                                              base);

    auto it = indirections_.rbegin();
    ExpressionNode* expr  = *it;
    ValueIndex offset_idx = expr->GenerateIR(irc);
    it++;

    ValueIndex dim_idx;
    int dim_offset = 1;
    auto dim_it    = var->GetDimensions().rbegin();

    while (it != indirections_.rend()) {
        dim_offset *= *dim_it;
        dim_idx     = CF->CreateConstant(dim_offset);

        expr = *it;
        temp = CF->MakeInstruction(T::INS_MUL, 
                                   dim_idx, 
                                   expr->GenerateIR(irc));

        offset_idx = CF->MakeInstruction(T::INS_ADDA, 
                                         offset_idx,
                                         temp);
        it++;
        dim_it++;
    }

    return CF->MakeInstruction(T::INS_ADDA,
                               arr_base,
                               offset_idx);
}

ValueIndex AssignmentNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "[IR] Parsing assignment";
    
    ValueIndex result = NOTFOUND;

    ValueIndex expr_idx  = value_->GenerateIR(irc);
    std::string var_name = designator_->GetIdentifierName();

    if (designator_->GetDesignatorType() == DESIG_VAR) {
        if (CF->IsVariableLocal(var_name)) {
            CF->WriteVariable(var_name, expr_idx);
        } else {
            int offset = irc.GlobalOffset(var_name);
            ValueIndex offset_idx   = CF->CreateConstant(offset);
            ValueIndex mem_location = CF->MakeInstruction(T::INS_ADDA,
                                                          irc.GlobalBase(),
                                                          offset_idx);
            // TODO: Think about this.
            // Do we need to use STORE or MOVE?
            result = CF->MakeInstruction(T::INS_STORE,
                                         expr_idx,
                                         mem_location);
        }
    } else {
       auto arr_id = static_cast<const ArrIdentifierNode*>(designator_);
       ValueIndex mem_location = arr_id->GenerateIR(irc);
       result = CF->MakeInstruction(T::INS_STORE,
                                    expr_idx,
                                    mem_location);
    }

    return result;
}

// TODO: Implement
ValueIndex FunctionCallNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "[IR] Parsing function call";

    ValueIndex result = NOTFOUND;

    return result;
}

ValueIndex RelationNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "[IR] Parsing relation";

    ValueIndex expr_1 = left_expr_->GenerateIR(irc);
    ValueIndex expr_2 = right_expr_->GenerateIR(irc);

    return CF->MakeInstruction(T::INS_CMP, expr_1, expr_2);
}

ValueIndex ITENode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "[IR] Parsing ITE";

    ValueIndex result = NOTFOUND;

    ValueIndex reln       = relation_->GenerateIR(irc);
    RelationalOperator op = relation_->GetOp();

    BBIndex previous    = CF->CurrentBBIdx();

    BBIndex then_start  = CF->CreateBB();
    CF->AddBBEdge(previous, then_start);

    CF->SetCurrentBB(then_start);
    then_sequence_->GenerateIR(irc);
    BBIndex then_end = CF->CurrentBBIdx();

    if (else_sequence_ == nullptr) {
        ////////////////////////////////////
        /*
         *           //////////////////
         *           //  conditional //
         *           //////////////////
         *                |  |
         *                |  ---------|
         *                |           v
         *                |       //////////////////
         *                |       // then_branch //
         *                |       /////////////////
         *                |           |
         *                | -----------
         *                | |
         *                v v
         *          ////////////////
         *          //  f_through //
         *          ////////////////
         *
         */
        ////////////////////////////////////
        BBIndex f_through = CF->CreateBB();

        CF->MakeInstruction(irc.ConvertOperation(op),
                            f_through,
                            reln);

        CF->AddBBEdge(previous, f_through);
        CF->AddBBEdge(then_end, f_through);

        CF->SetCurrentBB(f_through);
    } else {
        ////////////////////////////////////
        /*
         *           //////////////////
         *           //  conditional //
         *           //////////////////
         *                |  |
         *         |-------  ---------|
         *         v                  v
         *    /////////////////   /////////////////
         *    // else_branch //   // then_branch //
         *    /////////////////   /////////////////   
         *         |                   |
         *         |                   |
         *         |                   |
         *         |                   |
         *         ---------|  |--------
         *                  |  |
         *                  v  v
         *            ////////////////
         *            //  f_through //
         *            ////////////////
         */
         /////////////////////////////////////
        BBIndex else_start = CF->CreateBB();

        CF->MakeInstruction(irc.ConvertOperation(op),
                            else_start,
                            reln);
        
        CF->AddBBEdge(previous, else_start);

        CF->SetCurrentBB(else_start);
        else_sequence_->GenerateIR(irc);
        BBIndex else_end = CF->CurrentBBIdx();

        BBIndex f_through = CF->CreateBB();

        CF->SetCurrentBB(else_end);
        CF->MakeInstruction(T::INS_BRA,
                            f_through);

        CF->SetCurrentBB(then_end);
        CF->MakeInstruction(T::INS_BRA,
                            f_through);
                            
        CF->AddBBEdge(else_end, f_through);
        CF->AddBBEdge(then_end, f_through);

        CF->SetCurrentBB(f_through);
    }

    return result;
}

ValueIndex StatementNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "[IR] Parsing statement";

    ValueIndex result = NOTFOUND;
    switch(statement_type_) {
        case StatementType::STAT_ASSIGN: {
            auto assgn = static_cast<const AssignmentNode*>(this);
            result = assgn->GenerateIR(irc);
            break;
        }
        case StatementType::STAT_FUNCCALL: {
            auto funcn = static_cast<const FunctionCallNode*>(this);
            result = funcn->GenerateIR(irc);
            break;
        }
        case StatementType::STAT_ITE: {
            auto iten = static_cast<const ITENode*>(this);
            result = iten->GenerateIR(irc);
        }
    }

    return result;
}

void StatSequenceNode::GenerateIR(IRC& irc) const {
    StatementNode* statement;
    ValueIndex result;
    for (auto it = GetStatementBegin(); it != GetStatementEnd(); it++) {
        statement = *it;
        result = statement->GenerateIR(irc);
    }
    
    // TODO: Return value of last statement parsed.
}

void FunctionBodyNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "[IR] Parsing function body for: " << CF->FunctionName();

    func_statement_sequence_->GenerateIR(irc);
}

void FunctionDeclNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "[IR] Parsing function: " << identifier_->GetIdentifierName();

    std::string func_name = identifier_->GetIdentifierName();

    Function* func = new Function(func_name, irc.ValueCounter(), irc.ValMap());

    irc.AddFunction(func_name, func);
    irc.SetCurrentFunction(func);

    int offset = 0;
    Variable *var;
    std::string var_name;
    Symbol *sym;
    int total_size;

    auto local_sym_table = irc.ASTConst().GetLocalSymTable(func_name);
    for (auto table_entry: local_sym_table) {
        // TODO: Implement formal parameter handling
        if (table_entry.second == nullptr) {
            continue;
        }

        sym = table_entry.second;
        if (sym->IsArray()) {
            for (auto dim: sym->GetDimensions()) {
                total_size *= dim;
            }
            offset += total_size;
        } else {
            offset += 1;
        }

        var = new Variable(sym, offset);
        var_name = table_entry.first;
        CF->AddVariable(var_name, var);
    }

    func_body_->GenerateIR(irc);

    irc.SetCounter(CF->GetCounter());
    irc.ClearCurrentFunction();
}

void ComputationNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "[IR] Parsing Computation Root";

    int offset = 0;
    Variable *var;
    std::string var_name;
    Symbol *sym;
    int total_size;

    LOG(INFO) << "[IR] Declaring globals";
    irc.DeclareGlobalBase();
    auto global_sym_table = irc.ASTConst().GetGlobalSymTable();
    for (auto table_entry: global_sym_table) {
        sym = table_entry.second;
        total_size = 1;
        if (sym->IsArray()) {
            for (auto dim: sym->GetDimensions()) {
                total_size *= dim;
            }
            offset += total_size;
        } else {
            offset += 1;
        }

        var = new Variable(sym, offset);
        var_name = table_entry.first;
        irc.AddGlobal(var_name, var);
    }
    
    for (auto funcn: function_declarations_) {
        funcn->GenerateIR(irc);
    }
    
    LOG(INFO) << "[IR] Parsing main";

    std::string func_name = "main";
    Function* func = new Function(func_name, irc.ValueCounter(), irc.ValMap());

    irc.AddFunction(func_name, func);
    irc.SetCurrentFunction(func);

    computation_body_->GenerateIR(irc);
    CF->MakeInstruction(T::INS_END);
}
