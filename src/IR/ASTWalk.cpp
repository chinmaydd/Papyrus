#include "ASTWalk.h"

using namespace papyrus;

#define NOTFOUND -1

ValueIndex ConstantNode::GenerateIR(IRC* irc) const {
    ConstantValue* cval = new ConstantValue(value_);

    ValueIndex retval = irc->AddValue(cval);

    return retval;
}

ValueIndex FactorNode::GenerateIR(IRC* irc) const {
    ValueIndex result = -1;
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

    // TODO: Change
    return primary_idx;
}

ValueIndex ExpressionNode::GenerateIR(IRC* irc) const {
    ValueIndex primary_term = primary_term_->GenerateIR(irc);

    // TODO: Change
    return primary_term;
}
    
ValueIndex DesignatorNode::GenerateIR(IRC* irc) const {
    std::string identifier_name = GetIdentifierName();
    ValueIndex result;
    int offset;

    switch(desig_type_) {
        case DESIG_VAR: {
        }
        case DESIG_ARR: {
            ValueIndex base_idx = irc->GetCurrentFunction()
                                     ->GetLocalBase();
           
            // XXX: We could check here if NOTFOUND is returned.
            if (irc->CheckIfGlobal(identifier_name)) {
                offset = irc->GetOffsetForGlobalVariable(identifier_name);
            } else {
                offset = irc->GetCurrentFunction()
                            ->GetOffsetForVariable(identifier_name);
            }

            ValueIndex offset_idx = irc->CreateConstant(offset);
            irc->AddInstruction(InstTy::INS_ADDA, base_idx, offset_idx);
            break;
        }
    }

    return result;
}

void AssignmentNode::GenerateIR(IRC* irc) const {
    ValueIndex expr_idx = value_->GenerateIR(irc);
    DesignatorType desig_type = designator_->GetDesignatorType();

    if (desig_type == DesignatorType::DESIG_VAR) {
        irc->WriteVariable(designator_->GetIdentifierName(), expr_idx);
    } else {
        ValueIndex mem_location = designator_->GenerateIR(irc);
        irc->AddInstruction(Instruction::INS_STORE, expr_idx, mem_location);
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
        // TODO: Formal parameters
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

        var = new Variable(sym, offset);
        var_name = table_entry.first;
        irc->AddGlobalVariable(var_name, var);
    }
    
    for (auto funcn: function_declarations_) {
        funcn->GenerateIR(irc);
    }

    // TODO: Parse main body here.
}
