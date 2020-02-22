#ifndef PAPYRUS_IRCONSTRUCTOR_H
#define PAPYRUS_IRCONSTRUCTOR_H

#include "Papyrus/Logger/Logger.h"
#include "FrontEnd/ASTConstructor.h"
#include "IR.h"

#include <map>
#include <unordered_map>

namespace papyrus {

class ASTConstructor;

using InstTy     = Instruction::InstructionType;
using ValueIndex = int;

class IRConstructor {
public:
    IRConstructor(ASTConstructor&);
    void construct();

    ASTConstructor& GetASTConst() { return astconst_; }

    void AddFunction(const std::string&, Function*);
    Function* GetCurrentFunction() { return current_function_; }
    void SetCurrentFunction(Function* f) { current_function_ = f; }
    void ClearCurrentFunction() { current_function_ = nullptr; }

    void AddGlobalVariable(const std::string&, Variable*);
    int GetOffsetForGlobalVariable(const std::string&) const;
    bool CheckIfGlobal(const std::string&) const;

    ValueIndex AddValue(Value*);
    ValueIndex CreateConstant(int);
    
    void AddInstruction(InstTy);
    void AddInstruction(InstTy, ValueIndex);
    void AddInstruction(InstTy, ValueIndex, ValueIndex);

    void WriteVariable(const std::string&, ValueIndex);

    BBIndex CreateBB(std::string);
    BBIndex CreateBB(std::string, BBIndex);
    void AddBBPredecessor(BBIndex, BBIndex); // current, predecessor
    void AddBBSuccessor(BBIndex, BBIndex); // current, successor
    BBIndex GetCurrentBBIdx() const { return bb_counter_; }

private:
    Value* global_base_;

    ValueIndex value_counter_;
    InstructionIndex instruction_counter_;
    BBIndex bb_counter_;

    std::unordered_map<ValueIndex, Value*> value_map_;

    std::map<std::string, Variable*> global_variable_map_;
    std::unordered_map<std::string, ValueIndex> global_defs_;

    Function* current_function_;
    std::unordered_map<std::string, Function*> functions_;

    std::unordered_map<BBIndex, BasicBlock*> basic_block_map_;

    std::unordered_map<InstructionIndex, BBIndex> instruction_map_;

    ASTConstructor& astconst_;
};

} // namespace papyrus

#endif /* PAPYRUS_IRCONSTRUCTOR_H */
