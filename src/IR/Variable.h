#ifndef PAPYRUS_VARIABLE_H
#define PAPYRUS_VARIABLE_H

#include "Papyrus/Logger/Logger.h"

#include <vector>

namespace papyrus {

using ValueIndex = int;

class Symbol {
public:
    Symbol(const std::string&, const std::vector<int>&, bool, bool, bool);

    const std::string& IdentifierName() const { return identifier_; }
    const std::vector<int>& GetDimensions() const { return dimensions_; }
    bool IsArray() const { return if_array_; }
    bool IsGlobal() const { return is_global_; }
    bool IsFormal() const { return is_formal_; }
    
private:
    std::string identifier_;
    std::vector<int> dimensions_;
    bool if_array_;
    bool is_global_;
    bool is_formal_;
};

class Variable {
public:
    Variable(Symbol*, ValueIndex);
    Variable(Symbol*);

    const std::vector<int> GetDimensions() const { return sym_->GetDimensions(); }
    bool IsArray() const { return sym_->IsArray(); }
    bool IsGlobal() const { return sym_->IsGlobal(); }
    int Offset() const { return offset_; }

private:
    Symbol* sym_;
    int offset_;
};

} // namespace papyrus

#endif /* PAPYRUS_VARIABLE_H */
