#include "ASTWalk.h"

using namespace papyrus;

#define NOTFOUND -1

ValueIndex ConstantNode::GenerateIR(IRC* irc) const {
    return irc->CreateConstant(value_);
}

ValueIndex FactorNode::GenerateIR(IRC* irc) const {
    ValueIndex result = NOTFOUND;

    switch(factor_type_) {
        case FACT_DESIGNATOR: {
            break;
        }
        case FACT_NUMBER: {
            const ConstantNode* cnstn = static_cast<const ConstantNode*>(factor_node_);
            result = cnstn->GenerateIR(irc);
            break;
        }
        case FACT_EXPR: {
            break;
        }
        case FACT_FUNCCALL: {
            break;
        }
    }

    return result;
}

ValueIndex TermNode::GenerateIR(IRC* irc) const {
    ValueIndex primary_idx = primary_factor_->GenerateIR(irc);

    // TODO: Not implemented
    return primary_idx;
}

ValueIndex ExpressionNode::GenerateIR(IRC* irc) const {
    ValueIndex primary_term = primary_term_->GenerateIR(irc);

    // TODO: Not implemented
    return primary_term;
}

ValueIndex ArrIdentifierNode::GenerateIR(IRC* irc) const {
    std::string var_name = GetIdentifierName();
    ValueIndex base, offset, temp;

    const Variable* var;
    if (irc->IsVariableLocal(var_name)) {
        var = irc->GetCurrentFunction()
                 ->GetVariable(var_name);
        base = irc->GetLocalBase();
        offset = var->GetOffset();
    } else {
        var = irc->GetGlobalVariable(var_name);
        base = irc->GetGlobalBase();
        offset = irc->GetOffsetForGlobalVariable(var_name);
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

void AssignmentNode::GenerateIR(IRC* irc) const {
    LOG(INFO) << "[IR] Parsing assignment";

    ValueIndex expr_idx = value_->GenerateIR(irc);
    std::string var_name = designator_->GetIdentifierName();

    switch(designator_->GetDesignatorType()) {
        case DESIG_VAR: {
            irc->WriteVariable(var_name, expr_idx);
            break;
        }
        case DESIG_ARR: {
            const ArrIdentifierNode* arr_id = static_cast<const ArrIdentifierNode*>(designator_);
            ValueIndex mem_location = arr_id->GenerateIR(irc);
            irc->MakeInstruction(T::INS_STORE,
                                 expr_idx,
                                 mem_location);
        }
    }
}

void FunctionCallNode::GenerateIR(IRC* irc) const {
}

void StatementNode::GenerateIR(IRC* irc) const {
    LOG(INFO) << "[IR] Parsing statement";

    switch(statement_type_) {
        case StatementType::STAT_ASSIGN: {
            const AssignmentNode* assgn = static_cast<const AssignmentNode*>(this);
            assgn->GenerateIR(irc);
            break;
        }
        case StatementType::STAT_FUNCCALL: {
            const FunctionCallNode* funcn = static_cast<const FunctionCallNode*>(this);
            funcn->GenerateIR(irc);
            break;
        }
        default: {
        }
    }
}

void FunctionBodyNode::GenerateIR(IRC* irc) const {
    LOG(INFO) << "[IR] Parsing function body for: " << irc->GetCurrentFunction()
                                                          ->GetFunctionName();

    StatementNode* statement;
    for (auto it = GetStatementBegin(); it != GetStatementEnd(); it++) {
        statement = *it;
        statement->GenerateIR(irc);
    }
}

void FunctionDeclNode::GenerateIR(IRC* irc) const {
    LOG(INFO) << "[IR] Parsing function: " << identifier_->GetIdentifierName();

    std::string func_name = identifier_->GetIdentifierName();
    Function* func = new Function(func_name);
    func->SetLocalBase(irc->CreateValue(V::VAL_BASE));

    irc->AddFunction(func_name, func);
    irc->SetCurrentFunction(func);

    int offset = 0;
    Variable *var;
    std::string var_name;
    Symbol *sym;
    int total_size;

    auto local_sym_table = irc->GetASTConst()
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
        irc->GetCurrentFunction()
           ->AddVariable(var_name, var);
    }

    BBIndex entry_idx = irc->CreateBB(func_name);
    func->SetEntry(entry_idx);

    func_body_->GenerateIR(irc);

    BBIndex exit_idx = irc->CreateBB(func_name, irc->GetCurrentBBIdx());
    func->SetExit(exit_idx);

    irc->ClearCurrentFunction();
}

void ComputationNode::GenerateIR(IRC* irc) const {
    LOG(INFO) << "[IR] Parsing Computation Root";

    int offset = 0;
    Variable *var;
    std::string var_name;
    Symbol *sym;
    int total_size;

    LOG(INFO) << "[IR] Declaring globals";
    irc->DeclareGlobalBase();
    auto global_sym_table = irc->GetASTConst()
                                .GetGlobalSymTable();
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
    func->SetLocalBase(irc->CreateValue(V::VAL_BASE));

    irc->AddFunction(func_name, func);
    irc->SetCurrentFunction(func);

    BBIndex entry_idx = irc->CreateBB(func_name);
    func->SetEntry(entry_idx);

    StatementNode* statement;
    auto stat_begin = computation_body_->GetStatementBegin();
    auto stat_end = computation_body_->GetStatementEnd();
    for (auto it  = stat_begin; it != stat_end; it++) {
        statement = *it;
        statement->GenerateIR(irc);
    }

    BBIndex exit_idx = irc->CreateBB(func_name, irc->GetCurrentBBIdx());
    func->SetExit(exit_idx);
}
