#ifndef PAPYRUS_IRCONSTRUCTOR_H
#define PAPYRUS_IRCONSTRUCTOR_H

#include "Papyrus/Logger/Logger.h"
#include "FrontEnd/ASTConstructor.h"
#include "IR.h"

namespace papyrus {
class ASTConstructor;

using InstTy = Instruction::InstructionType;

class IRConstructor {
public:
   IRConstructor(ASTConstructor&);
    void construct();

    ASTConstructor& GetASTConst() { return astconst_; }

    void AddFunction(const std::string&, Function*);
    Function* GetCurrentFunction() { return current_function_; }
    void SetCurrentFunction(Function* f) { current_function_ = f; }
    void ClearCurrentFunction() { current_function_ = nullptr; }
    
    void AddInstruction(InstTy);
    void AddInstruction(InstTy, ValueIndex);
    void AddInstruction(InstTy, ValueIndex, ValueIndex);

private:
    Value* global_base_;

    int global_value_counter_;
    std::unordered_map<std::string, ValueIndex> global_defs_;
    std::unordered_map<ValueIndex, Value*> global_value_map_;

    Function* current_function_;
    std::unordered_map<std::string, Function*> functions_;
    ASTConstructor& astconst_;
};

} // namespace papyrus

#endif /* PAPYRUS_IRCONSTRUCTOR_H */
