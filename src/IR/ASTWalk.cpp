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
        auto temp = CF->TryReduce(op, idx_1, idx_2);
        if (temp == NOTFOUND) {
            //////////////////////////////////////////////////
            idx_1 = MI(irc.ConvertOperation(op), idx_1, idx_2);
            //////////////////////////////////////////////////
        } else {
            idx_1 = temp;
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

        // Check if factor is reducible
        // If yes, fold it.
        auto temp = CF->TryReduce(op, idx_1, idx_2);
        if (temp == NOTFOUND) {
            //////////////////////////////////////////////////
            idx_1 = MI(irc.ConvertOperation(op), idx_1, idx_2);
            //////////////////////////////////////////////////
        } else {
            idx_1 = temp;
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

    // Basic error checking - Check if variables are being used correctly
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
    VI temp_idx;

    // access_str stores the access path to the array variable. For each array
    // load and store we can save this in a cache and later optimize away 
    // unnecessary loads from the IR. The reason we can do this is that
    // since the IR is SSA, we are guaranteed that there will be only one 
    // definition for each value being used in the access path. This implies that
    // if the access_path value is the exact same and there has not been any
    // kill involved in the interim, we will retrieve the same value from the array
    //
    // access_str looks like : 
    //
    // variable_VAL1_#CONSTVAL2_VAL3
    std::string access_str = "";

    if (CF->GetValue(offset_idx)->Type() == V::VAL_CONST) {
        access_str = access_str + "_#" + std::to_string(CF->GetValue(offset_idx)->GetConstant());
    } else {
        access_str = access_str + "_" + std::to_string(offset_idx);
    }

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

        auto expr_idx = expr->GenerateIR(irc);

        // Add to access_str
        if (CF->GetValue(expr_idx)->Type() == V::VAL_CONST) {
            access_str = access_str + "_#" + std::to_string(CF->GetValue(expr_idx)->GetConstant());
        } else {
            access_str = access_str + "_" + std::to_string(expr_idx);
        }

        // Try and Reduce generated value
        temp_idx = CF->TryReduce(ArithmeticOperator::BINOP_MUL, dim_idx, expr_idx);
        if (temp_idx == NOTFOUND) {
            //////////////////////////////////////////////////
            temp = MI(T::INS_MUL, dim_idx, expr_idx);
            //////////////////////////////////////////////////
            CF->load_contributors.push_back(CF->CurrentInstructionIdx());
        } else {
            temp = temp_idx;
        }

        // Try and reduce generated value
        temp_idx = CF->TryReduce(ArithmeticOperator::BINOP_ADD, offset_idx, temp);
        if (temp_idx == NOTFOUND) {
            //////////////////////////////////////////////////
            offset_idx = MI(T::INS_ADD, offset_idx, temp);
            //////////////////////////////////////////////////
            CF->load_contributors.push_back(CF->CurrentInstructionIdx());
        } else {
            offset_idx = temp_idx;
        }

        it++;
        dim_it++;
    }

    // Try and reduce generated value
    temp_idx = CF->TryReduce(ArithmeticOperator::BINOP_MUL, offset_idx, CC(4));
    if (temp_idx == NOTFOUND) {
        //////////////////////////////////////////////////
        temp = MI(T::INS_MUL, offset_idx, CC(4));
        //////////////////////////////////////////////////
        CF->load_contributors.push_back(CF->CurrentInstructionIdx());
    } else {
        temp = temp_idx;
    }

    access_str = var_name + access_str;
    CF->access_str_ = access_str;
    //////////////////////////////////////////////////
    auto result = MI(T::INS_ADDA, arr_base, temp);
    //////////////////////////////////////////////////

    // Add it to the load contributor. The idea here is that for each load/store
    // there will be some value which contribute to the access. If we optimize
    // a load away, we will also remove all the instructions which lead to 
    // generation of these contributors (in case they arent being used elsewhere)
    CF->load_contributors.push_back(CF->CurrentInstructionIdx());

    return result;
}

/*
 * Depending on the type of the variable local v/s global generate a Value
 */
VI DesignatorNode::GenerateIR(IRC& irc) const {
    LOG(INFO) << "[IR] Parsing designator";

    VI result = NOTFOUND;
    auto var_name = identifier_->IdentifierName();

    const Variable* var;
    
    // Check local v/s global
    if (CF->IsVariableLocal(var_name)) {
        var = CF->GetVariable(var_name);
    } else if (irc.IsVariableGlobal(var_name)) {
        var = irc.GetGlobal(var_name);
    } else {
        LOG(ERROR) << "[IR] Usage of variable " + var_name + " which is not defined.";
        exit(1);
    }
    
    // Check if being used correctly
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
                //
                // This is the fundamental "read" of the SSA generation algorithm.
                // For each variable, we go searching for the current definition
                // of the variable. We start with the current block and then
                // the predecessors and so on (lazily adding Phis wherever) we go
                result = CF->ReadVariable(var_name, CF->CurrentBBIdx());
            }
        } else if (irc.IsVariableGlobal(var_name)) {
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
            ////////////////////////////////////////
            result = MI(T::INS_LOAD, mem_location);
            ////////////////////////////////////////
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
        
        // All of the below instructions are used for collecting some extra
        // information for optimizing away the loads and stores in the IR.
        //
        // Collecting some metadata for ArrayLSRemover
        //
        for (auto ins_idx: CF->load_contributors) {
            CF->AddArrContributor(ins_idx, CF->CurrentInstructionIdx());
        }

        CF->load_contributors = {};

        // Reset the temporary variables which actually track information
        if (CF->access_str_ != "") {
            CF->load_hash_[result] = CF->access_str_;
            CF->access_str_ = "";
        }
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
            //
            // This is again a fundamental operation in the SSA generation 
            // algorithm. WriteVariable will overwrite the current definition of
            // the variable in the current BB. A subsequent ReadVariable will
            // read expr_idx as the value.
            CF->WriteVariable(var_name, expr_idx);

            result = expr_idx;
        } else if (irc.IsVariableGlobal(var_name)) {
            // Removed in lieu using LocationIdx()
            //
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
       // Reset load_contributors since they will be computed again.
       CF->load_contributors = {};
       auto arr_id = static_cast<const ArrIdentifierNode*>(designator_);
       VI mem_location = arr_id->GenerateIR(irc);

       // Insert a kill instruction for a store.
       VI location_value;
       if (irc.IsVariableGlobal(var_name)) {
           location_value = irc.GetLocationValue(var_name);
       } else {
           location_value = CF->GetLocationValue(var_name);
       }

       // Kill the current BB. This ensures that any join node with the BB as a 
       // predecessor will have a KILL at the very beginning. This ensures that
       // we dont introduce new errors in the KILLing of vars.
       MI(T::INS_KILL, location_value);
       CF->KillBB(CF->CurrentBBIdx(), location_value);

       CF->GetValue(mem_location)->SetIdentifier(var_name);
       //////////////////////////////////////////////////
       result = MI(T::INS_STORE, expr_idx, mem_location);
       //////////////////////////////////////////////////
     
       for (auto ins_idx: CF->load_contributors) {
           CF->AddArrContributor(ins_idx, CF->CurrentInstructionIdx());
       }

       // Reset temporaries.
       CF->load_contributors = {};

       if (CF->access_str_ != "")  {
            CF->store_hash_[result] = CF->access_str_;
            CF->access_str_ = "";
       }
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

// This function is used to fold branches which can be computed at compile-time
VI ITENode::TryReducingCmp(IRConstructor& irc, VI left, VI right) const {
    // Check if we want to explore THEN or ELSE branch of the condition.
    // We will discard the other.
    int THEN = 1;
    int ELSE = 2;
    auto op = relation_->GetOp();
    auto result = CF->ReduceCondition(op, left, right);

    if (result != NOTFOUND) {
        // TODO: Remove usages of exprs
        // Make current instruction inactive and remove uses of expressions
        CF->CurrentInstruction()->MakeInactive();
        if (result == THEN) {
            then_sequence_->GenerateIR(irc);
        } else if (result == ELSE) {
            if (else_sequence_ != nullptr) {
                else_sequence_->GenerateIR(irc);
            }
        }
    }

    return result;
}

// Note:
//
// Please read the reference paper to understand the ordering of the SealBB()
// operations. 
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

    VI temp = TryReducingCmp(irc, left_expr, right_expr);
    if (temp != NOTFOUND) {
        return temp;
    }

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

        //////////////////////////////////
        // KILL HANDLING
        //
        // Here, we insert a kill if either previous is killed or
        // then is killed. But if then has ended, that means its modifications
        // wont flow into this block
        //////////////////////////////////
        if (CF->IsKilled(then_end) && !CF->HasEndedBB(then_end)) {
            for (auto location_val: CF->GetKilledValues(then_end)) {
                MI(T::INS_KILL, location_val);
                CF->KillBB(CF->CurrentBBIdx(), location_val);
            }
        }
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
 
        /////////////////////////////////////////////////
        // KILL HANDLING
        //
        // Here, if either then or else has ended
        // their kills wont flow into the newly created block
        // and hence we need either of them to have the kill AND
        // none of them to have ended.
        /////////////////////////////////////////////////
        if (CF->IsKilled(then_end) && !CF->HasEndedBB(then_end)) {
            for (auto location_val: CF->GetKilledValues(then_end)) {
                MI(T::INS_KILL, location_val);
                CF->KillBB(CF->CurrentBBIdx(), location_val);
            }
        }

        if (CF->IsKilled(else_end) && !CF->HasEndedBB(else_end)) {
            for (auto location_val: CF->GetKilledValues(else_end)) {
                MI(T::INS_KILL, location_val);
                CF->KillBB(CF->CurrentBBIdx(), location_val);
            }
        }
    }

    return result;
}

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

    ////////////////////////////
    // KILL HANDLING
    ////////////////////////////
    if (CF->IsKilled(loop_end) && !CF->HasEndedBB(loop_end)) {
       for (auto location_val: CF->GetKilledValues(loop_end)) {
           CF->MakeInstructionFront(T::INS_KILL, location_val);
           CF->KillBB(CF->CurrentBBIdx(), location_val);
       }
    }

    // Commenting this out.
    // The idea here is that if the loop is NOT killed in the loop body,
    // it will not be killed in the loop head and the previous definition
    // prevails.
    //
    // if (CF->IsKilled(previous)) {
    //    for (auto location_val: CF->GetKilledValues(previous)) {
    //        CF->MakeInstructionFront(T::INS_KILL, location_val);
    //        CF->KillBB(CF->CurrentBBIdx(), location_val);
    //    }
    // }

    CF->SealBB(next_bb);
    CF->SetCurrentBB(next_bb);

    if (CF->IsKilled(loop_header)) {
       for (auto location_val: CF->GetKilledValues(loop_header)) {
           CF->MakeInstructionFront(T::INS_KILL, location_val);
           CF->KillBB(CF->CurrentBBIdx(), location_val);
       }
    }

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
            location = CV(V::VAL_STACK);
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
            location = CV(V::VAL_STACK);
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

    /////////////////////////////////////////////////////////
    // EXPERIMENTAL!
    // This is a conservative analysis... More on this later.
    /////////////////////////////////////////////////////////
    GlobalClobbering gc = GlobalClobbering(irc);
    gc.Run();

    auto clobber_status = gc.GetClobberStatus();
    auto readdef_status = gc.GetReadDefStatus();

    std::unordered_set<std::string> tainted_globals = {};
    for (auto clob_pair: clobber_status) {
        for (auto var_name: clob_pair.second) {
            tainted_globals.insert(var_name);
        }
    }

    for (auto readdef_pair: readdef_status) {
        for (auto var_name: readdef_pair.second) {
            tainted_globals.insert(var_name);
        }
    }

    auto variables = irc.Globals();
    for (auto var_pair: variables) {
        auto var_name = var_pair.first;
        auto var      = var_pair.second;

        if (var->IsArray()) {
            tainted_globals.insert(var_name);
        }
    }

    // We could perform some analysis since we have already looked at functions
    // and their definitions.
    auto func_name = "main";
    auto func = new Function(func_name, irc.ValueCounter(), irc.ValMap());

    // Now add non-tainted variables as locals.
    irc.AddFunction(func_name, func);
    irc.SetCurrentFunction(func);

    std::unordered_set<std::string> mark;
    for (auto globvar_pair: irc.Globals()) {
        auto var_name = globvar_pair.first;
        auto var = globvar_pair.second;
        if (tainted_globals.find(var_name) == tainted_globals.end()) {
            CF->AddVariable(var_name, var);
            mark.insert(var_name);
        }
    }

    for (auto glob: mark) {
        irc.RemoveGlobal(glob);
    }

    // IR generation of "main" begins here.
    if (computation_body_ != nullptr) {
        computation_body_->GenerateIR(irc);
    }

    if (CF->CurrentBB()->IsSealed()) {
        CF->CurrentBB()->Seal();
    }

    //////////////////////////////////////////////////
    MI(T::INS_END);
    //////////////////////////////////////////////////
}
