#include "ASTWalk.h"

using namespace papyrus;

#define NOTFOUND -1

ValueIndex ConstantNode::GenerateIR(IRC* irc) const {
    LOG(INFO) << "Parsing constant";

    return irc->CreateConstant(value_);
}

ValueIndex FactorNode::GenerateIR(IRC* irc) const {
    LOG(INFO) << "Parsing factor";

    ValueIndex result = NOTFOUND;
    switch(factor_type_) {
        case FACT_DESIGNATOR: {
            // TODO: ReadVariable()
            break;
        }
        case FACT_NUMBER: {
            const ConstantNode* cnstn = static_cast<const ConstantNode*>(factor_node_);
            result = cnstn->GenerateIR(irc);
            break;
        }
        case FACT_EXPR: {
            const ExpressionNode* exprn = static_cast<const ExpressionNode*>(factor_node_);
            result = exprn->GenerateIR(irc);
            break;
        }
        case FACT_FUNCCALL: {
            const FunctionCallNode* fncn = static_cast<const FunctionCallNode*>(factor_node_);
            result = fncn->GenerateIR(irc);
            break;
        }
    }

    return result;
}

ValueIndex TermNode::GenerateIR(IRC* irc) const {
    LOG(INFO) << "Parsing term";

    ValueIndex idx_1, idx_2;
    idx_1 = primary_factor_->GenerateIR(irc);

    ArithmeticOperator op;
    const FactorNode* fact;
    for (auto next_pair: secondary_factors_) {
        op = next_pair.first;
        fact = next_pair.second;

        idx_2 = fact->GenerateIR(irc);
        idx_1 = irc->MakeInstruction(irc->ConvertInstruction(op),
                                     idx_1,
                                     idx_2);
    }

    return idx_1;
}

ValueIndex ExpressionNode::GenerateIR(IRC* irc) const {
    LOG(INFO) << "Parsing expression";

    ValueIndex idx_1, idx_2;
    idx_1 = primary_term_->GenerateIR(irc);

    ArithmeticOperator op;
    const TermNode* term;
    for (auto next_pair: secondary_terms_) {
        op = next_pair.first;
        term = next_pair.second;

        idx_2 = term->GenerateIR(irc);
        idx_1 = irc->MakeInstruction(irc->ConvertInstruction(op),
                                     idx_1,
                                     idx_2);
    }

    return idx_1;
}

ValueIndex ArrIdentifierNode::GenerateIR(IRC* irc) const {
    LOG(INFO) << "Parsing ArrIdentifier";

    std::string var_name = GetIdentifierName();
    ValueIndex base, offset, temp;

    const Variable* var;
    if (irc->IsVariableLocal(var_name)) {
        var = irc->CurrentFunction()->GetVariable(var_name);
        base = irc->LocalBase();
        offset = var->Offset();
    } else {
        var = irc->GetGlobalVariable(var_name);
        base = irc->GlobalBase();
        offset = irc->GlobalOffset(var_name);
    }

    ValueIndex arr_base = irc->MakeInstruction(T::INS_ADDA, 
                                               offset,
                                               base);

    auto it = indirections_.rbegin();
    ExpressionNode* expr = *it;
    ValueIndex offset_idx = expr->GenerateIR(irc);
    it++;

    ValueIndex dim_idx;
    int dim_offset = 1;
    auto dim_it = var->GetDimensions().rbegin();

    while (it != indirections_.rend()) {
        dim_offset *= *dim_it;
        dim_idx = irc->CreateConstant(dim_offset);

        expr = *it;
        temp = irc->MakeInstruction(T::INS_MUL, 
                                    dim_idx, 
                                    expr->GenerateIR(irc));

        offset_idx = irc->MakeInstruction(T::INS_ADDA, 
                                          offset_idx,
                                          temp);
        it++;
        dim_it++;
    }

    return irc->MakeInstruction(T::INS_ADDA,
                                arr_base,
                                offset_idx);
}

ValueIndex AssignmentNode::GenerateIR(IRC* irc) const {
    LOG(INFO) << "[IR] Parsing assignment";
    
    ValueIndex result = NOTFOUND;

    ValueIndex expr_idx = value_->GenerateIR(irc);
    std::string var_name = designator_->GetIdentifierName();

    if (designator_->GetDesignatorType() == DESIG_VAR) {
        if (irc->IsVariableLocal(var_name)) {
            irc->WriteVariable(var_name, expr_idx);
        } else {
            ValueIndex offset_idx = irc->GlobalOffset(var_name);
            ValueIndex mem_location = irc->MakeInstruction(T::INS_ADDA,
                                                           irc->GlobalBase(),
                                                           offset_idx);
            result = irc->MakeInstruction(T::INS_STORE,
                                          expr_idx,
                                          mem_location);
        }
    } else {
       const ArrIdentifierNode* arr_id = static_cast<const ArrIdentifierNode*>(designator_);
       ValueIndex mem_location = arr_id->GenerateIR(irc);
       result = irc->MakeInstruction(T::INS_STORE,
                                     expr_idx,
                                     mem_location);
    }

    return result;
}

// TODO: Implement
ValueIndex FunctionCallNode::GenerateIR(IRC* irc) const {
    LOG(INFO) << "[IR] Parsing function call";
    ValueIndex result = NOTFOUND;

    return result;
}

ValueIndex ITENode::GenerateIR(IRC* irc) const {
    ValueIndex result = NOTFOUND;



    return result;
}

ValueIndex StatementNode::GenerateIR(IRC* irc) const {
    LOG(INFO) << "[IR] Parsing statement";

    ValueIndex result = NOTFOUND;
    switch(statement_type_) {
        case StatementType::STAT_ASSIGN: {
            const AssignmentNode* assgn = static_cast<const AssignmentNode*>(this);
            result = assgn->GenerateIR(irc);
            break;
        }
        case StatementType::STAT_FUNCCALL: {
            const FunctionCallNode* funcn = static_cast<const FunctionCallNode*>(this);
            result = funcn->GenerateIR(irc);
            break;
        }
        case StatementType::STAT_ITE: {
            const ITENode* iten = static_cast<const ITENode*>(this);
            result = iten->GenerateIR(irc);
        }
    }

    return result;
}

void StatSequenceNode::GenerateIR(IRC* irc) const {
    StatementNode* statement;
    for (auto it = GetStatementBegin(); it != GetStatementEnd(); it++) {
        statement = *it;
        statement->GenerateIR(irc);
    }
}

void FunctionBodyNode::GenerateIR(IRC* irc) const {
    LOG(INFO) << "[IR] Parsing function body for: " << irc->CurrentFunction()
                                                          ->FunctionName();

    func_statement_sequence_->GenerateIR(irc);
}

void FunctionDeclNode::GenerateIR(IRC* irc) const {
    LOG(INFO) << "[IR] Parsing function: " << identifier_->GetIdentifierName();

    std::string func_name = identifier_->GetIdentifierName();
    Function* func = new Function(func_name);
    func->SetLocalBase(irc->CreateValue(V::VAL_LOCALBASE));

    irc->AddFunction(func_name, func);
    irc->SetCurrentFunction(func);

    int offset = 0;
    Variable *var;
    std::string var_name;
    Symbol *sym;
    int total_size;

    auto local_sym_table = irc->ASTConst()
                               .GetLocalSymTable(func_name);
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

        ValueIndex offset_idx = irc->CreateConstant(offset);
        var = new Variable(sym, offset_idx);
        var_name = table_entry.first;
        irc->CurrentFunction()->AddVariable(var_name, var);
    }

    BBIndex entry_idx = irc->CreateBB(func_name);
    func->SetEntry(entry_idx);

    func_body_->GenerateIR(irc);

    BBIndex exit_idx = irc->CreateBB(func_name, irc->CurrentBBIdx());
    func->SetExit(exit_idx);

    irc->ClearCurrentFunction();
}

void ComputationNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "[IR] Parsing Computation Root";

    int offset = 0;
    Variable *var;
    std::string var_name;
    Symbol *sym;
    int total_size;

    LOG(INFO) << "[IR] Declaring globals";
    irc->DeclareGlobalBase();
    auto global_sym_table = irc->ASTConst().GetGlobalSymTable();
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

        ValueIndex offset_idx = irc->CreateConstant(offset);
        var = new Variable(sym, offset_idx);
        var_name = table_entry.first;
        irc->AddGlobalVariable(var_name, var);
    }
    
    for (auto funcn: function_declarations_) {
        funcn->GenerateIR(irc);
    }
    
    LOG(INFO) << "[IR] Parsing main";

    std::string func_name = "main";
    Function* func = new Function(func_name);
    func->SetLocalBase(irc->CreateValue(V::VAL_GLOBALBASE));

    irc->AddFunction(func_name, func);
    irc->SetCurrentFunction(func);

    BBIndex entry_idx = irc->CreateBB(func_name);
    func->SetEntry(entry_idx);

    computation_body_->GenerateIR(irc);
    irc->MakeInstruction(T::INS_END);

    BBIndex exit_idx = irc->CreateBB(func_name, irc->CurrentBBIdx());
    func->SetExit(exit_idx);
}
