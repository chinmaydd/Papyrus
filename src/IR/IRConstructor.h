#ifndef PAPYRUS_IRCONSTRUCTOR_H
#define PAPYRUS_IRCONSTRUCTOR_H

#include "Papyrus/Logger/Logger.h"
#include "FrontEnd/ASTConstructor.h"
#include "IR.h"

#include <map>
#include <unordered_map>

namespace papyrus {

class ASTConstructor;

using T          = Instruction::InstructionType;
using V          = Value::ValueType;
using ValueIndex = int;

class IRConstructor {
public:
    IRConstructor(ASTConstructor&);
    void BuildIR();

    ASTConstructor& ASTConst() { return astconst_; }

    void AddFunction(const std::string&, Function*);
    Function* CurrentFunction() const { return current_function_; }
    void SetCurrentFunction(Function* f) { current_function_ = f; }
    void ClearCurrentFunction() { current_function_ = nullptr; }

    const Variable* GetGlobalVariable(const std::string&) const;
    void AddGlobalVariable(const std::string&, Variable*);
    int GlobalOffset(const std::string&) const;

    bool IsVariableGlobal(const std::string&) const;
    bool IsVariableLocal(const std::string&) const;

    ValueIndex CreateValue(V);
    ValueIndex CreateConstant(int);
    void AddUsage(ValueIndex, InstructionIndex);
    
    Instruction* CurrentInstruction() { return instruction_map_[instruction_counter_]; }
    ValueIndex MakeInstruction(T);
    ValueIndex MakeInstruction(T, ValueIndex);
    ValueIndex MakeInstruction(T, ValueIndex, ValueIndex);

    void WriteVariable(const std::string&, ValueIndex);
    void WriteVariable(const std::string&, BBIndex, ValueIndex);

    BBIndex CreateBB(std::string);
    BBIndex CreateBB(std::string, BBIndex);
    void AddBBPredecessor(BBIndex, BBIndex); // current, predecessor
    void AddBBSuccessor(BBIndex, BBIndex);   // current, successor
    BBIndex CurrentBBIdx() const { return bb_counter_; }

    void DeclareGlobalBase();
    ValueIndex GlobalBase() const { return global_base_idx_; }
    ValueIndex LocalBase() const { return current_function_->LocalBase(); }

    std::string GetInstructionString(T);
    T ConvertInstruction(ArithmeticOperator);

private:
    Value* global_base_;
    ValueIndex global_base_idx_;

    ValueIndex value_counter_;
    InstructionIndex instruction_counter_;
    BBIndex bb_counter_;

    std::unordered_map<ValueIndex, Value*> value_map_;

    std::map<std::string, Variable*> global_variable_map_;

    Function* current_function_;
    std::unordered_map<std::string, Function*> functions_;

    std::unordered_map<BBIndex, BasicBlock*> basic_block_map_;

    std::unordered_map<InstructionIndex, Instruction*> instruction_map_;

    ASTConstructor& astconst_;
};

} // namespace papyrus

#endif /* PAPYRUS_IRCONSTRUCTOR_H */
