neg x unary minus
add x y addition
sub x y subtraction
mul x y multiplication
div x y division
cmp x y comparison
adda x y add two addresses x und y (used only with arrays)
load y load from memory address y
store y x store y to memory address x
move y x assign x := y
phi x1 x2 compute Phi(x1, x2)
end end of program
bra y branch to y
bne x y branch to y on x not equal
beq x y branch to y on x equal
ble x y branch to y on x less or equal
blt x y branch to y on x less
bge x y branch to y on x greater or equal
bgt x y branch to y on x greater
call x call function at addr x?

read read
write x write
writeNL writeNewLine


******************************














//#include "ASTWalker.h"
//
//using IRCtx = IRContextInfo;
//
//void ValueNode::GenerateIR(IRCtx& ctx) {
//}
//
//void IdentifierNode::GenerateIR(IRCtx& ctx) {
//}
//
//void ConstantNode::GenerateIR(IRCtx& ctx) {
//}
//
//void FactorNode::GenerateIR(IRCtx& ctx) {
//}
//
//readVariable(identifier) {
//    // magic mystery
//}
//
//// We could return Value or ValueIndex here.
//// ValueIndex would index into a global structure 
//// which stores value -> ValueIndex mapping
//Value* TermNode::GenerateIR(IRCtx& ctx) {
//
//}
//
//evaluate_expr() {
//
//}
//
//something expressionNode::GenerateIR() {
//
//}
//
//something TermNode::GenerateIR() {
//    value1 = parsefactor(primary_factor);
//
//}
//
//something FactorNode::GenerateIR(IRCtx& ctx) {
//    if factor_type_ == FACT_DESIGNATOR {
//        value = readVariable();
//    } else if factor_type == FACT_NUMBER {
//        value = constant();
//    } else if factor_type == FACT_EXPR {
//        value = evaluate_expr()
//    } else if factor_type == FACT_FUNCCALL {
//        value = evaluate_funccall()
//    }
//    return value;
//}
//
//void DesignatorNode::GenerateIR(IRCtx& ctx) {
//    readVariable(identifier);
//}
//
//void VarIdentifierNode::GenerateIR(IRCtx& ctx) {
//}
//
//void ArrIdentifierNode::GenerateIR(IRCtx& ctx) {
//}
//
//void RelationNode::GenerateIR(IRCtx& ctx) {
//}
//
//void StatementNode::GenerateIR(IRCtx& ctx) {
//}
//
//void FunctionCallNode::GenerateIR(IRCtx& ctx) {
//}
//
//void AssignmentNode::GenerateIR(IRCtx& ctx) {
//}
//
//void ITENode::GenerateIR(IRCtx& ctx) {
//    /*
//     * create successor
//     * add dummy phi
//     * check values flowing into it
//     */
//}
//
//void ReturnNode::GenerateIR(IRCtx& ctx) {
//    /*
//     * Create exit node .. I dont think anything else is required here.
//     */
//}
//
//void WhileNode::GenerateIR(IRCtx& ctx) {
//    /*
//     * create relop node
//     * add a dummy phi maybe
//     * add statement sequence as successor
//     * go back to loop header and fix phi
//     */
//}
//
//void StatSequenceNode::GenerateIR(Function f, IRCtx& ctx) {
//    // functioncallnode ...
//    // call function() -> in the codegen pahse, this would be effectively
//    // replaced by the pushing of args and jump to function
//    // It also generates an instruction with that as the operand. Yes
//
//    // assignmentnode
//    // let x <- expression
//    // (4) <- (2) + (3)
//    // (11) <- (2) + (3) * (5)
//
//}
//
//void VarDeclNode::GenerateIR(IRCtx& ir_ctx, ASTWalkerCtx& ast_ctx) {
//
//void FunctionBodyNode::GenerateIR(Function& f, IRCtx& ir_ctx) {
//  /*
//   * Check formal params add them to local value numbering
//   * Add local variables to local value numbering
//   * parse body
//   */
//    for (auto var_decl: var_declarations_)
//        f.addlocalvars(get_var_decl_from_node(var_decl));
//
//
//    f.cfg = parsestatementsequence(func_statement_seq_);
//}
//
//Function* FunctionDeclNode::GenerateFunction(IRCtx& ctx) {
//   f = new Function()
//   f.identifier = identifier_;
//   f.addParams(formal_parameters_); // stores them as local vars but with the 
//   // tag of params...
//
//   // maybe call it setIR()
//   func_body_->generateIR(f, ctx);
//
//   return f;
//}
//
//// class Type {
//// bool is_array_;
//// std::vector<int> dimensions;
//// }
//Type* TypeDeclNode::GenerateIR() const {
//    new Type(is_array_, dimensions);
//}
//
//// AddFunction(Function*) and AddFunction
//void ComputationNode::GenerateIR(IRCtx& ir_ctx, ASTWalkerCtx& ast_ctx) const {
//    for (auto var: &variable_declarations_) {
//        ir.addglobavars(get_variables_from_node(var));
//    }
//
//    for (auto: fdecl: &function_declarations_)  {
//        ir_ctx.AddFunction(fdecl.DefineFunction(ir_ctx));
//    }
//
//    ir_ctx.AddFunction(new Function("main", <params>, computation_body_));
//}
//
///*
// * XXX: add a map in ircontextinfo which stores -> 
// * function identifier -> function graph
// *
// * Take care that all reads and writes for operations are from stack and not reg.
// * Handle this somehow in op.
// *
// * Model Load, LoadStack/Mem
// * Store, StoreStack/Mem
// *
// * Def per basic block?
// */

***************************
