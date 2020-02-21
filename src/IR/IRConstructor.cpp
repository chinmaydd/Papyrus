#include "IRConstructor.h"

using namespace papyrus;

using ASTC   = ASTConstructor;
using IRC    = IRConstructor;
using InstTy = Instruction::InstructionType;

using ValueIndex = int;

IRConstructor::IRConstructor(ASTC& astconst) :
    astconst_(astconst) {}

void IRConstructor::AddInstruction(InstTy insty) {}
void IRConstructor::AddInstruction(InstTy insty, ValueIndex arg_1) {}

void IRConstructor::AddInstruction(InstTy insty, ValueIndex arg_1, ValueIndex arg_2) {
    switch(insty) {
        case InstTy::INS_STORE: {
            // TODO
        }
    }
}

void IRConstructor::AddFunction(const std::string& func_name, Function *func) {
    functions_[func_name] = func;
}

ValueIndex ConstantNode::GenerateIR(IRC* irc) const {
    ConstantValue* cval = new ConstantValue(value_);

    ValueIndex retval = irc->GetCurrentFunction()
                           ->AddValue(cval);

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
    ValueIndex result;
    switch(desig_type_) {
        case DESIG_VAR: {
            ValueIndex base_idx = irc->GetCurrentFunction()
                                     ->GetLocalBase();
           
            int offset = irc->GetCurrentFunction()
                            ->GetOffsetForVariable(identifier_->GetIdentifierName());
            ValueIndex offset_idx = irc->GetCurrentFunction()
                                       ->CreateConstant(offset);

            // TODO: Need to think about this.
            irc->AddInstruction(InstTy::INS_ADDA, base_idx, offset_idx);
        }
        case DESIG_ARR: {
        }
    }

    return result;
}

void AssignmentNode::GenerateIR(IRC* irc) const {
    ValueIndex expr_idx = value_->GenerateIR(irc);
    ValueIndex mem_location = designator_->GenerateIR(irc);

    irc->AddInstruction(Instruction::INS_STORE, expr_idx, mem_location);
}

void FunctionCallNode::GenerateIR(IRC* irc) const {
    // Push onto stack and then call?
}

void StatementNode::GenerateIR(IRC* irc) const {
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
    irc->SetCurrentFunction(func);

    int offset = 0;
    Variable *var;
    std::string var_name;
    Symbol *sym;
    int total_size;

    auto local_sym_table = irc->GetASTConst()
                               .GetLocalSymTable(func_name);
        for (auto table_entry: local_sym_table) {

        total_size = 1;
        if (sym->IsArray()) {
            for (auto dim: sym->GetDimensions()) {
                total_size *= dim;
            }
            offset += total_size;
        } else {
            offset += 1;
        }

        sym = table_entry.second;
        var = new Variable(sym, offset);
        var_name = table_entry.first;
        irc->GetCurrentFunction()
           ->AddVariable(var_name, var);
    }

    func_body_->GenerateIR(irc);

    irc->ClearCurrentFunction();
}

void ComputationNode::GenerateIR(IRC* irc) const {
    LOG(INFO) << "[IR] Parsing functions..";

    // TODO: Parse globals here
    
    for (auto funcn: function_declarations_) {
        funcn->GenerateIR(irc);
    }
}

void IRConstructor::construct() {
    const ComputationNode* root = astconst_.GetRoot();
    root->GenerateIR(this);
}
