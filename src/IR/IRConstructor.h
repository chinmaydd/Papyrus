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

class IRConstructor {
public:
    IRConstructor(ASTConstructor&);
    void BuildIR();

    ASTConstructor& ASTConst() { return astconst_; }

    void AddFunction(const std::string&, Function*);
    Function* CurrentFunction() const { return current_function_; }
    void SetCurrentFunction(Function* f) { current_function_ = f; }
    void ClearCurrentFunction() { current_function_ = nullptr; }
    const std::unordered_map<std::string, Function*> Functions() const {
        return functions_;
    }

    bool IsVariableGlobal(const std::string&) const;
    const Variable* GetGlobal(const std::string&) const;
    void AddGlobal(const std::string&, Variable*);
    int GlobalOffset(const std::string&) const;

    VI ValueCounter() const { return value_counter_; }
    void SetCounter(VI idx) { value_counter_ = idx; }
    VI CreateConstant(int);
    std::unordered_map<VI, Value*>* ValMap() const { return value_map_; }

    void DeclareGlobalBase();
    VI GlobalBase() const { return global_base_idx_; }

    std::string GetInstructionString(T);
    T ConvertOperation(ArithmeticOperator);
    T ConvertOperation(RelationalOperator);

private:
    Value* global_base_;
    VI global_base_idx_;

    BI current_bb_;

    std::map<std::string, Variable*> global_variable_map_;

    Function* current_function_;
    std::unordered_map<std::string, Function*> functions_;

    std::unordered_map<VI, Value*>* value_map_;

    ASTConstructor& astconst_;
    
    VI value_counter_;
};

} // namespace papyrus

#endif /* PAPYRUS_IRCONSTRUCTOR_H */
