#include "ASTWalk.h"

using namespace papyrus;

#define CF irc.CurrentFunction()
#define MI irc.CurrentFunction()->MakeInstruction
#define CC irc.CurrentFunction()->CreateConstant
#define CV irc.CurrentFunction()->CreateValue
#define NOTFOUND -1
#define REDUCIBLE -2

/*
 * ConstantNode is converted into Value with VAL_CONST and the value of the
 * number embedded in it
 */
VI ConstantNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "Parsing constant";

    return CC(value_);
}

/*
 * FactorNode is converted into a Value as well
 */
VI FactorNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "Parsing factor";

    VI result = NOTFOUND;
    switch(factor_type_) {
        case FACT_DESIGNATOR: {
            auto design = static_cast<const DesignatorNode*>(factor_node_);
            result = design->GenerateIR(irc);
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

/* 
 * Convert TermNode into a Value
 */
VI TermNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "Parsing term";

    // Convert the primary factor into a Value
    // For each of the (other) secondary factors, convert them to a value
    // and then FACTOR_1 (op) FACTOR_2
    // and so on..
    VI idx_1, idx_2;
    idx_1 = primary_factor_->GenerateIR(irc);

    ArithmeticOperator op;
    const FactorNode* fact;
    for (auto next_pair: secondary_factors_) {
        op    = next_pair.first;
        fact  = next_pair.second;

        idx_2 = fact->GenerateIR(irc);

        // Check if factor is reducible
        // If yes, fold it.
        if (!CF->IsReducible(idx_1, idx_2)) {
            //////////////////////////////////////////////////
            idx_1 = MI(irc.ConvertOperation(op), idx_1, idx_2);
            //////////////////////////////////////////////////
        } else {
            idx_1 = CF->Reduce(idx_1, idx_2, op);
        }
    }

    return idx_1;
}

/*
 * Convert Expression into a Value
 */
VI ExpressionNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "Parsing expression";

    // Convert the primary term into a value
    // For each of the (other) secondary terms, convert them to a value
    // and then TERM_1 (op) TERM_2
    // and so on..
    VI idx_1, idx_2;
    idx_1 = primary_term_->GenerateIR(irc);

    ArithmeticOperator op;
    const TermNode* term;
    for (auto next_pair: secondary_terms_) {
        op    = next_pair.first;
        term  = next_pair.second;

        idx_2 = term->GenerateIR(irc);

        // Check if term is reducible
        // If yes, fold it.
        if (!CF->IsReducible(idx_1, idx_2)) {
            //////////////////////////////////////////////////
            idx_1 = MI(irc.ConvertOperation(op), idx_1, idx_2);
            //////////////////////////////////////////////////
        } else {
            idx_1 = CF->Reduce(idx_1, idx_2, op);
        }
    }

    return idx_1;
}

/*
 * For arrays, we do not perform any kind of optimizations. Ideally, we should
 * take care optimizing base value calculations and only change the index
 */
VI ArrIdentifierNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "Parsing ArrIdentifier";

    auto var_name = IdentifierName();
    VI base, offset, temp;

    const Variable* var;

    // Locals v/s Global
    if (CF->IsVariableLocal(var_name)) {
        var    = CF->GetVariable(var_name);
        base   = CF->LocalBase();
        offset = var->GetLocationIdx();

        // Commented out to include LocationIdx() as the location value instead.
        //
        // offset = CC(var->Offset());
    } else if (irc.IsVariableGlobal(var_name)) {
        var    = irc.GetGlobal(var_name);
        base   = irc.GlobalBase();
        offset = var->GetLocationIdx();

        // Commented out to include LocationIdx() as the location value instead.
        //
        // offset = CC(irc.GlobalOffset(var_name)*4);
    } else {
        LOG(ERROR) << "[IR] Usage of variable " + var_name + " which is not defined.";
        exit(1);
    }

    if (!var->IsArray()) {
        LOG(ERROR) << "[IR] Usage of variable " + var_name + " as an array.";
        exit(1);
    } else if (indirections_.size() != var->GetDimensions().size()) {
        LOG(ERROR) << "[IR] Incorrect indirections given to " + var_name + " in usage";
        exit(1);
    }

    //////////////////////////////////////////////////
    // To compute base to initial offset we should use
    // ADD instead of ADDA.
    //
    // Changed: 03/16/2020
    VI arr_base = MI(T::INS_ADD, offset, base);
    //////////////////////////////////////////////////

    auto ind_temp(indirections_);
    std::reverse(ind_temp.begin(), ind_temp.end());
    auto it = ind_temp.begin();

    auto expr  = *it;
    VI offset_idx = expr->GenerateIR(irc);
    it++;

    // Calculate the offset
    VI dim_idx;
    int dim_offset = 1;

    std::vector<int> var_temp(var->GetDimensions());
    std::reverse(var_temp.begin(), var_temp.end());
    auto dim_it = var_temp.begin();

    while (it != ind_temp.end()) {
        dim_offset *= *dim_it;
        // Changed it back again
        dim_idx     = CC(dim_offset);

        expr = *it;

        //////////////////////////////////////////////////
        auto expr_idx = expr->GenerateIR(irc);
        if (!CF->IsReducible(dim_idx, expr_idx)) {
            temp = MI(T::INS_MUL, dim_idx, expr_idx);
        } else {
            temp = CF->Reduce(dim_idx, expr_idx, T::INS_MUL);
        }
        //////////////////////////////////////////////////

        //////////////////////////////////////////////////
        if (!CF->IsReducible(offset_idx, temp)) {
            offset_idx = MI(T::INS_ADD, offset_idx, temp);
        } else {
            offset_idx = CF->Reduce(offset_idx, temp, T::INS_ADD);
        }
        //////////////////////////////////////////////////
 
        it++;
        dim_it++;
    }

    //////////////////////////////////////////////////
    if (!CF->IsReducible(offset_idx, CC(4))) {
        temp = MI(T::INS_MUL, offset_idx, CC(4));
    } else {
        temp = CF->Reduce(offset_idx, CC(4), T::INS_MUL);
    }

    //////////////////////////////////////////////////
    return MI(T::INS_ADDA, arr_base, temp);
    //////////////////////////////////////////////////
}

/*
 * Depending on the type of the variable local v/s global generate a Value
 */
VI DesignatorNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "[IR] Parsing designator";

    VI result = NOTFOUND;
    auto var_name = identifier_->IdentifierName();

    const Variable* var;
    if (CF->IsVariableLocal(var_name)) {
        var = CF->GetVariable(var_name);
    } else if (irc.IsVariableGlobal(var_name)) {
        var = irc.GetGlobal(var_name);
    } else {
        LOG(ERROR) << "[IR] Usage of variable " + var_name + " which is not defined.";
        exit(1);
    }
    
    if (var->IsArray() && desig_type_ == DESIG_VAR) {
        LOG(ERROR) << "[IR] Usage of variable " + var_name + " as a variable which is an array";
        exit(1);
    }

    if (desig_type_ == DESIG_VAR) {
        if (CF->IsVariableLocal(var_name)) {
            // Handling formals:
            //
            // The way we handle formals right now is that we assume them to be
            // local variables. But their first load is from the stack since the
            // assumption is that they are passed to functions from the stack.
            // Once they are loaded, they are marked loaded. In the register 
            // allocation phase, we can then choose to store/reload them as
            // necessary. We create an instruction which will create a memory
            // location value on the stack; localbase - (4 * ParamNumber). This
            // is an approximation as there will be the return address on the
            // stack as well. But that is part of the ABI which can be modified
            // according to the architecture itself.
            if (CF->IsVariableFormal(var_name) &&
                !CF->IsFormalLoaded(var_name)) {

                auto mem_location = var->GetLocationIdx();

                //////////////////////////////////////////
                auto temp = MI(T::INS_ADD, CF->LocalBase(), mem_location);
                result    = MI(T::INS_LOAD, temp);
                //////////////////////////////////////////

                CF->LoadFormal(var_name);
                CF->WriteVariable(var_name, result);
            } else {
                // Load variable. Can be thought of as a "SSA Read"
                result = CF->ReadVariable(var_name, CF->CurrentBBIdx());
            }
        } else if (irc.IsVariableGlobal(var_name)) {
            //////////////////////////////////////////////////
            // This still uses *4 as we know that the offset is a constant
            // calculated at compile time
            // auto offset = irc.GlobalOffset(var_name);
            // auto offset_idx = CC(offset*4);
            auto offset_idx = var->GetLocationIdx();

            /////////////////////////////////////
            // Changed to use ADD instead of ADDA
            auto mem_location = MI(T::INS_ADD, irc.GlobalBase(), offset_idx);
            /////////////////////////////////////

            CF->GetValue(mem_location)->SetIdentifier(var_name);
            /////////////////////////////////////
            result = MI(T::INS_LOAD, mem_location);
            /////////////////////////////////////
        } else {
            LOG(ERROR) << "[IR] Usage of variable " + var_name + " which is not defined";
            exit(1);
        }
    } else {
        auto arr_id = static_cast<const ArrIdentifierNode*>(this);
        auto mem_location = arr_id->GenerateIR(irc);

        CF->GetValue(mem_location)->SetIdentifier(var_name);
        /////////////////////////////////////
        result = MI(T::INS_LOAD, mem_location);
        /////////////////////////////////////
    }

    return result;
}

VI AssignmentNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "[IR] Parsing assignment";
    
    auto result = NOTFOUND;

    auto expr_idx  = value_->GenerateIR(irc);
    auto var_name  = designator_->IdentifierName();

    if (designator_->GetDesignatorType() == DESIG_VAR) {
        if (CF->IsVariableLocal(var_name)) {
            // If this is a formal param, we want to mark it loaded
            // since the next use should use this value and not load
            // it from the stack again.
            if (CF->IsVariableFormal(var_name) &&
                !CF->IsFormalLoaded(var_name)) {
                CF->LoadFormal(var_name);
            }

            /////////////////////
            // XXX: Experimental
            /////////////////////
            auto val = CF->GetValue(expr_idx);
            if (val->Type() != V::VAL_CONST) {
                // Here, setting the identifier does not make sense
                // since with copy-propagation we will keep on overwriting the 
                // same value.
                //
                // val->SetIdentifier(var_name);
                val->SetType(V::VAL_VAR);
            }

            // Overwrite variable definition in the SSA Context
            CF->WriteVariable(var_name, expr_idx);

            result = expr_idx;
        } else if (irc.IsVariableGlobal(var_name)) {
            // int offset      = irc.GlobalOffset(var_name);
            // This still uses *4 as we know that the offset is a constant
            // calculated at compile time
            // VI offset_idx   = CC(offset*4);
            // CF->GetValue(mem_location)->SetIdentifier(var_name);

            auto temp = irc.GetGlobal(var_name)->GetLocationIdx();

            //////////////////////////////////////////////////
            // Changed to use addition from base to var offset
            // to use ADD instead of ADDA
            auto mem_location = MI(T::INS_ADD, irc.GlobalBase(), temp);
            //////////////////////////////////////////////////

            CF->GetValue(mem_location)->SetIdentifier(var_name);
            //////////////////////////////////////////////////
            result = MI(T::INS_STORE, expr_idx, mem_location);
            //////////////////////////////////////////////////
        } else {
            LOG(ERROR) << "Usage of variable " + var_name + " which is not defined.";
            exit(1);
        }
    } else {
       auto arr_id = static_cast<const ArrIdentifierNode*>(designator_);
       VI mem_location = arr_id->GenerateIR(irc);

       CF->GetValue(mem_location)->SetIdentifier(var_name);
       //////////////////////////////////////////////////
       result = MI(T::INS_STORE, expr_idx, mem_location);
       //////////////////////////////////////////////////
    }

    return result;
}

VI FunctionCallNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "[IR] Parsing function call";

    VI func_call = CV(V::VAL_FUNC);
    auto func_name = identifier_->IdentifierName();

    if (!irc.IsExistFunction(func_name)) {
        LOG(ERROR) << "[IR] Usage of function " + func_name + " which is not defined";
        exit(1);
    }

    CF->GetValue(func_call)->SetIdentifier(identifier_->IdentifierName());


    VI result;
    if (func_name == "InputNum") {
        if (arguments_.size() != 0) {
            LOG(ERROR) << "[IR] Incorrect usage of InputNum() function";
            exit(1);
        }
        
        //////////////////////////////////////////////////
        result = MI(T::INS_READ);
        //////////////////////////////////////////////////
    } else if (func_name == "OutputNum") {
        if (arguments_.size() != 1) {
            LOG(ERROR) << "[IR] Incorrect usage of OutputNum() function";
            exit(1);
        }

        //////////////////////////////////////////////////
        result = MI(T::INS_WRITEX, arguments_.at(0)->GenerateIR(irc));
        //////////////////////////////////////////////////
    } else if (func_name == "OutputNewLine") {
        if (arguments_.size() != 0) {
            LOG(ERROR) << "[IR] Incorrct usage of OutputNewLine() function";
            exit(1);
        }

        //////////////////////////////////////////////////
        result = MI(T::INS_WRITENL);
        //////////////////////////////////////////////////
    } else {
        // Let us assume here, that the arguments are pushed from L-R
        VI interm;
        for (auto argument: arguments_) {
            interm = argument->GenerateIR(irc); 
            //////////////////////////////////////////////////
            MI(T::INS_ARG, interm);
            //////////////////////////////////////////////////
        }

        //////////////////////////////////////////////////
        result = MI(T::INS_CALL, func_call);
        //////////////////////////////////////////////////
    }
    
    return result;
}

VI RelationNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "[IR] Parsing relation";

    VI expr_1 = left_expr_->GenerateIR(irc);
    VI expr_2 = right_expr_->GenerateIR(irc);

    //////////////////////////////////////////////////
    return MI(T::INS_CMP, expr_1, expr_2);
    //////////////////////////////////////////////////
}

VI ITENode::HandleReducibleCmp(IRConstructor& irc, VI left, VI right) const {
    LOG(INFO) << "[IR] Reducing CFG";

    VI result = NOTFOUND;

    // Make current instruction inactive and remove uses of expressions
    CF->CurrentInstruction()->MakeInactive();
    // TODO: Remove usages of exprs

    auto op = relation_->GetOp();

    // Check if we want to explore THEN or ELSE branch of the condition.
    // We will discard the other.
    int THEN = 1;
    int ELSE = 2;
    auto retval = CF->ReduceCondition(op, left, right);

    if (retval == THEN) {
        then_sequence_->GenerateIR(irc);
    } else {
        if (else_sequence_ != nullptr) {
            else_sequence_->GenerateIR(irc);
        }
    }

    return result;
}

/*
 * TODO: Draw fancy ascii picture.
 */
VI ITENode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "[IR] Parsing ITE";

    VI result = NOTFOUND;

    VI reln = relation_->GenerateIR(irc);
    auto op = relation_->GetOp();

    // Here, we handle the case where the control flow is reducible. 
    // The reason for doing this during AST construction is that we already
    // know a lot of information about the "constantness" of the conditionals
    // It might be a waste of time to perform this as an analysis since, 
    // reducing the CFG at this stage also allows us to get rid of 
    // unnecessary PHI functions.
    auto curr_ins = CF->CurrentInstruction();
    VI left_expr  = curr_ins->Operands().at(0);
    VI right_expr = curr_ins->Operands().at(1);

    if (CF->IsReducible(left_expr, right_expr)) {
        // Early return
        return HandleReducibleCmp(irc, left_expr, right_expr);
    } // TODO: Handle values being same but not constants
    ///////////////////////////////////////////////

    BI previous = CF->CurrentBBIdx();
    CF->SealBB(previous);

    BI then_start  = CF->CreateBB(B::BB_THEN);
    CF->AddBBEdge(previous, then_start);

    CF->SealBB(then_start);
    CF->SetCurrentBB(then_start);

    then_sequence_->GenerateIR(irc);
    BI then_end = CF->CurrentBBIdx();

    if (else_sequence_ == nullptr) {
        BI f_through = CF->CreateBB(B::BB_THROUGH);

        CF->SetCurrentBB(previous);
        VI bb_val = CF->GetBB(f_through)->GetSelfValue();

        //////////////////////////////////////////////////
        MI(irc.ConvertOperation(op), reln, bb_val);
        //////////////////////////////////////////////////
                            
        if (!CF->HasEndedBB(then_end)) {
            CF->AddBBEdge(then_end, f_through);
        }

        CF->AddBBEdge(previous, f_through);

        CF->SealBB(then_end);
        CF->SealBB(f_through);

        CF->SetCurrentBB(f_through);
    } else {
        BI else_start = CF->CreateBB(B::BB_ELSE);

        CF->SetCurrentBB(previous);
        VI bb_val = CF->GetBB(else_start)->GetSelfValue();

        //////////////////////////////////////////////////
        MI(irc.ConvertOperation(op), reln, bb_val);
        //////////////////////////////////////////////////
        
        CF->AddBBEdge(previous, else_start);
        CF->SealBB(else_start);
        CF->SetCurrentBB(else_start);

        else_sequence_->GenerateIR(irc);
        BI else_end = CF->CurrentBBIdx();

        CF->SealBB(then_end);
        CF->SealBB(else_end);

        bool then_ended = CF->HasEndedBB(then_end);
        bool else_ended = CF->HasEndedBB(else_end);
        if (then_ended && else_ended) {
            return result;
        }

        BI f_through = CF->CreateBB(B::BB_THROUGH);

        if (!then_ended) {
            CF->SetCurrentBB(then_end);
            VI bb_val = CF->GetBB(f_through)->GetSelfValue();
            //////////////////////////////////////////////////
            MI(T::INS_BRA, bb_val);
            //////////////////////////////////////////////////
            CF->AddBBEdge(then_end, f_through);
        }

        if (!else_ended) {
            CF->AddBBEdge(else_end, f_through);
        }

        CF->SetCurrentBB(f_through);
        CF->SealBB(f_through);
    }

    return result;
}

/*
 * TODO: Draw fancy ascii picture.
 */
VI WhileNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "[IR] Parsing While";

    VI result = NOTFOUND;

    BI previous = CF->CurrentBBIdx();

    BI loop_header = CF->CreateBB(B::BB_LOOPHEAD);

    VI bb_val = CF->GetBB(loop_header)->GetSelfValue();
    
    //////////////////////////////////////////////////
    // TODO: Implement loop condition checking for 
    // reducibility
    //////////////////////////////////////////////////

    CF->AddBBEdge(previous, loop_header);
    CF->SealBB(previous);

    BI loop_body = CF->CreateBB(B::BB_LOOPBODY);

    VI next_bb = CF->CreateBB(B::BB_THROUGH);

    CF->AddBBEdge(loop_header, next_bb);
    CF->AddBBEdge(loop_header, loop_body);

    CF->SetCurrentBB(loop_header);
    VI reln = loop_condition_->GenerateIR(irc);

    CF->SealBB(loop_body);

    CF->SetCurrentBB(loop_body);
    statement_sequence_->GenerateIR(irc);
    BI loop_end = CF->CurrentBBIdx();

    if (!CF->HasEndedBB(loop_end)) {
        CF->AddBBEdge(loop_end, loop_header);
        CF->AddBackEdge(loop_end, loop_header);
        CF->SetCurrentBB(loop_end);
        VI bb_val = CF->GetBB(loop_header)->GetSelfValue();
        //////////////////////////////////////////////////
        MI(T::INS_BRA, bb_val);
        //////////////////////////////////////////////////
    }

    CF->SealBB(loop_end);
    CF->SealBB(loop_header);

    CF->SetCurrentBB(loop_header);
    auto op = loop_condition_->GetOp();

    bb_val = CF->GetBB(next_bb)->GetSelfValue();
    //////////////////////////////////////////////////
    MI(irc.ConvertOperation(op), reln, bb_val);
    //////////////////////////////////////////////////

    CF->SealBB(next_bb);
    CF->SetCurrentBB(next_bb);

    return result;
}

// Generate a Return Node
VI ReturnNode::GenerateIR(IRC& irc) const {
    VI result = NOTFOUND;

    VI interm;
    if (return_expression_ != nullptr) {
        interm = return_expression_->GenerateIR(irc);
        //////////////////////////////////////////////////
        result = MI(T::INS_RET, interm);
        //////////////////////////////////////////////////
    } else {
        //////////////////////////////////////////////////
        result = MI(T::INS_RET);
        //////////////////////////////////////////////////
    }

    CF->GetBB(CF->CurrentBBIdx())->EndBB();
    CF->AddExitBlock(CF->CurrentBBIdx());

    return result;
}

// Handle each type of statement possible
VI StatementNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "[IR] Parsing statement";

    VI result = NOTFOUND;
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
            break;
        }
        case StatementType::STAT_WHILE: {
            auto whilen = static_cast<const WhileNode*>(this);
            result = whilen->GenerateIR(irc);
            break;
        }
        case StatementType::STAT_RETURN: {
            auto retn = static_cast<const ReturnNode*>(this);
            result = retn->GenerateIR(irc);
        }
    }

    return result;
}

// For each statement, generate a result
void StatSequenceNode::GenerateIR(IRC& irc) const {
    StatementNode* statement;
    VI result;
    for (auto it = GetStatementBegin(); it != GetStatementEnd(); it++) {
        statement = *it;
        result = statement->GenerateIR(irc);
    }

    // TODO: Generate an implicit return instruction if not already
    // generated.
}

// Simple handling of statement sequences
void FunctionBodyNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "[IR] Parsing function body for: " << CF->FunctionName();

    func_statement_sequence_->GenerateIR(irc);
}

// Generic functions to declaring and defining functions
void FunctionDeclNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "[IR] Parsing function: " << identifier_->IdentifierName();

    auto func_name = identifier_->IdentifierName();
    auto func = new Function(func_name, irc.ValueCounter(), irc.ValMap());

    irc.AddFunction(func_name, func);
    irc.SetCurrentFunction(func);

    int offset = 0, old_offset;
    Variable *var;
    std::string var_name;
    Symbol *sym;
    int total_size;
    VI expr, location;
    int formal_count = 1;

    auto local_sym_table = irc.ASTConst().GetLocalSymTable(func_name);
    for (auto table_entry: local_sym_table) {
        var_name = table_entry.first;
        sym = table_entry.second;

        // Local variables shadow global definitions. Due to this, if a variable
        // is redefined i.e if it is already global we dont really take any action
        // Ideally, if it was not possible to have two symbols of the same name, 
        // this would lead to a parse error.
        //
        // if (irc.IsVariableGlobal(var_name)) {
        //     exit(1);
        // }

        if (sym->IsFormal()) {
            // Creating a location value here to be used for later
            location = CV(V::VAL_LOCATION);
            CF->GetValue(location)->SetConstant(-4 * formal_count);
            CF->GetValue(location)->SetIdentifier(var_name);

            var = new Variable(sym, location);

            // We would like to store ordering of the formal parameters
            // Ideally, their storage locations, but atleast their order
            // in the argument sequence
            var->SetParamNumber(formal_count);
            expr = CV(V::VAL_FORMAL);
            CF->WriteVariable(var_name, expr);

            formal_count++;
        } else {
            old_offset = offset;
            if (sym->IsArray()) {
                total_size = 1;
                for (auto dim: sym->GetDimensions()) {
                    total_size *= dim;
                }
                offset += total_size;
            } else {
                offset += 1;
            }

            // Again, here we would like to store localbase+offset in the 
            // register if possible, but for now we can store this value in
            // the variable; can be accessed later
            location = CV(V::VAL_LOCATION);
            CF->GetValue(location)->SetConstant(-4 * old_offset);
            CF->GetValue(location)->SetIdentifier(var_name);

            // The offset in words in stored in the "Variable" whereas the 
            // offset in bytes is stored the in the value that is created.
            var = new Variable(sym, old_offset, location);
        }

        CF->AddVariable(var_name, var);
    }

    func_body_->GenerateIR(irc);

    irc.SetCounter(CF->GetCounter());
    irc.ClearCurrentFunction();
}

// Handle Root Computation!
void ComputationNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "[IR] Parsing Computation Root";

    int offset = 0, old_offset;
    Variable *var;
    std::string var_name;
    Symbol *sym;
    int total_size;
    VI location;

    LOG(INFO) << "[IR] Declaring globals";

    irc.DeclareGlobalBase();
    auto global_sym_table = irc.ASTConst().GetGlobalSymTable();
    for (auto table_entry: global_sym_table) {
        sym = table_entry.second;
        total_size = 1;
        old_offset = offset;
        if (sym->IsArray()) {
            for (auto dim: sym->GetDimensions()) {
                total_size *= dim;
            }
            offset += total_size;
        } else {
            offset += 1;
        }

        // We are creating a location value here to simplify loads and
        // store at a later stage in the IR lowering. This would allow us to
        // enable better register allocation since the offset is a value 
        // which needs to be stored in reg. Actually, it is the base+offset
        // which is more relevant to be stored. But that is a worry for a
        // later day.
        var_name = table_entry.first;

        location = irc.CreateValue(V::VAL_LOCATION);
        irc.GetValue(location)->SetConstant(4 * old_offset);
        irc.GetValue(location)->SetIdentifier(var_name);

        var = new Variable(sym, old_offset, location);
        irc.AddGlobal(var_name, var);
    }

    // Forward declaration of functions
    for (auto funcn: function_declarations_) {
        irc.DeclareFunction(funcn->GetFunctionName());
    }
    
    for (auto funcn: function_declarations_) {
        funcn->GenerateIR(irc);
    }

    LOG(INFO) << "[IR] Parsing main";

    // We could perform some analysis since we have already looked at functions
    // and their definitions.
    auto func_name = "main";
    auto func = new Function(func_name, irc.ValueCounter(), irc.ValMap());

    irc.AddFunction(func_name, func);
    irc.SetCurrentFunction(func);

    if (computation_body_ != nullptr) {
        computation_body_->GenerateIR(irc);
    }

    //////////////////////////////////////////////////
    MI(T::INS_END);
    //////////////////////////////////////////////////
}
