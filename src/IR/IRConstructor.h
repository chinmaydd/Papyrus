#ifndef PAPYRUS_IRCONSTRUCTOR_H
#define PAPYRUS_IRCONSTRUCTOR_H

#include "Papyrus/Logger/Logger.h"
#include "FrontEnd/ASTConstructor.h"
#include "IR.h"

#include <map>
#include <unordered_map>

namespace papyrus {

class ASTConstructor;

using T  = Instruction::InstructionType;
using V  = Value::ValueType;
using VI = int;

/*
 * IRConstructor is what is exposed to the USER. The User provides it an 
 * ASTConstructor which contains the AST (the naming could be better). BuildIR()
 * then constructs the IR.
 */
class IRConstructor {
public:
    IRConstructor(ASTConstructor&);

    void BuildIR();

    void DeclareFunction(const std::string&);
    void AddFunction(const std::string&, Function*);
    void SetCurrentFunction(Function* f) { current_function_ = f; }
    void ClearCurrentFunction() { current_function_ = nullptr; }
    void AddGlobal(const std::string&, Variable*);
    void RemoveGlobal(const std::string&);
    void SetCounter(VI idx) { value_counter_ = idx; }
    void DeclareGlobalBase();

    ASTConstructor& ASTConst() { return astconst_; }

    const std::unordered_map<std::string, Function*>& Functions() const;
    const std::map<std::string, Variable*>& Globals() const;
    std::vector<BI> PostOrderCFG(const std::string&) const;

    Variable* GetGlobal(const std::string&) const;
    Value* GetValue(VI) const;

    bool IsExistFunction(const std::string&) const;
    bool IsVariableGlobal(const std::string&) const;
    bool IsIntrinsic(const std::string&) const;

    inline Function* CurrentFunction() const { return current_function_; }

    Function* GetFunction(const std::string&);

    int GlobalOffset(const std::string&) const;

    VI ValueCounter() const { return value_counter_; }
    VI CreateConstant(int);
    VI GlobalBase() const { return global_base_idx_; }
    VI GetLocationValue(const std::string&) const;
    VI CreateValue(V);

    std::unordered_map<VI, Value*>* ValMap() const { return value_map_; }

    T ConvertOperation(ArithmeticOperator);
    T ConvertOperation(RelationalOperator);

private:
    // Value (Index) for the GlobalBase from where globals are referenced.
    Value* global_base_;
    VI global_base_idx_;

    // Value Counter, it is not updated as the counter increases per function.
    VI value_counter_;

    // Index of the current BB. Why is this here?
    BI current_bb_;

    // Global variables map. This is used to store variables which are then 
    // used to determine their offsets from the global base.
    std::map<std::string, Variable*> global_variable_map_;
    // Stores a map from function_name -> pointer to Function object
    std::unordered_map<std::string, Function*> functions_;
    // Global Value map
    std::unordered_map<VI, Value*>* value_map_;

    // Pointer to an object of the current function
    Function* current_function_;

    // Store reference for the ASTConstructor for which we generate the IR.
    ASTConstructor& astconst_;
    
    void DeclareIntrinsicFunctions();
};

} // namespace papyrus

#endif /* PAPYRUS_IRCONSTRUCTOR_H */
