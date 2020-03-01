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

    void DeclareFunction(const std::string&);
    void AddFunction(const std::string&, Function*);
    void SetCurrentFunction(Function* f) { current_function_ = f; }
    void ClearCurrentFunction() { current_function_ = nullptr; }
    void AddGlobal(const std::string&, Variable*);
    void SetCounter(VI idx) { value_counter_ = idx; }
    void DeclareGlobalBase();

    ASTConstructor& ASTConst() { return astconst_; }

    const std::unordered_map<std::string, Function*> Functions() const {
        return functions_;
    }
    const Variable* GetGlobal(const std::string&) const;

    bool IsExistFunction(const std::string&) const;
    bool IsVariableGlobal(const std::string&) const;
    bool IsIntrinsic(const std::string&) const;

    Function* CurrentFunction() const { return current_function_; }

    int GlobalOffset(const std::string&) const;

    VI ValueCounter() const { return value_counter_; }
    VI CreateConstant(int);
    VI GlobalBase() const { return global_base_idx_; }

    std::unordered_map<VI, Value*>* ValMap() const { return value_map_; }

    T ConvertOperation(ArithmeticOperator);
    T ConvertOperation(RelationalOperator);

private:
    Value* global_base_;

    VI global_base_idx_;
    VI value_counter_;

    BI current_bb_;

    std::map<std::string, Variable*> global_variable_map_;
    std::unordered_map<std::string, Function*> functions_;
    std::unordered_map<VI, Value*>* value_map_;

    Function* current_function_;

    ASTConstructor& astconst_;
    
    void DeclareIntrinsicFunctions();
};

} // namespace papyrus

#endif /* PAPYRUS_IRCONSTRUCTOR_H */
