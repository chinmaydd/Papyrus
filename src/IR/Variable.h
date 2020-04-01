#ifndef PAPYRUS_VARIABLE_H
#define PAPYRUS_VARIABLE_H

#include "Papyrus/Logger/Logger.h"

#include <vector>

namespace papyrus {

using VI = int;

/* A Symbol is used for a variable but without context. It encapsulates
 * properties of the variable used in the program and stores nothing else. To 
 * understand the distinction better, a Symbol is the entity generated during
 * AST Construction and Variable is generated "using" the associated symbol
 * during IR Construction
 */
class Symbol {
public:
    Symbol(const std::string&, const std::vector<int>&, bool, bool, bool);

    const std::string& IdentifierName() const { return identifier_; }
    const std::vector<int>& GetDimensions() const { return dimensions_; }
    bool IsArray() const { return if_array_; }
    bool IsGlobal() const { return is_global_; }
    bool IsFormal() const { return is_formal_; }
    
private:
    // Identifier for the Symbol
    std::string identifier_;
    // If array, store dimensions
    std::vector<int> dimensions_;

    bool if_array_;
    bool is_global_;
    bool is_formal_;
};

class Variable {
public:
    Variable(Symbol*, int, VI);
    Variable(Symbol*, VI);

    const std::vector<int> GetDimensions() const { return sym_->GetDimensions(); }

    bool IsArray() const { return sym_->IsArray(); }
    bool IsGlobal() const { return sym_->IsGlobal(); }
    bool IsFormal() const { return sym_->IsFormal(); }

    int Offset() const { return offset_; }

    VI GetLocationIdx() const { return vi_; }

    void SetParamNumber(int num) { param_number_ = num; }
    int ParamNumber() const { return param_number_; }

private:
    // Pointer to Symbol Object
    Symbol* sym_;
    // Offset from base (local/global)
    int offset_;
    // Value for the Variable
    VI vi_;
    // If it is formal parameter, store its param_number_ (function call context)
    int param_number_;
};

} // namespace papyrus

#endif /* PAPYRUS_VARIABLE_H */
