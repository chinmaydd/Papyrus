#ifndef PAPYRUS_VARIABLE_H
#define PAPYRUS_VARIABLE_H

#include "Papyrus/Logger/Logger.h"

#include <vector>

namespace papyrus {

using ValueIndex = int;

class Symbol {
public:
    Symbol(const std::string&, const std::vector<int>&, bool, bool);

    const std::string& GetIdentifierName() const { return identifier_; }
    const std::vector<int>& GetDimensions() const { return dimensions_; }
    bool IsArray() const { return if_array_; }
    bool IsGlobal() const { return is_global_; }
    
private:
    std::string identifier_;
    std::vector<int> dimensions_;
    bool if_array_;
    bool is_global_;
};

class Variable {
public:
    Variable(Symbol*, ValueIndex);

    const std::vector<int> GetDimensions() const { return sym_->GetDimensions(); }
    bool IsArray() const { return sym_->IsArray(); }
    bool IsGlobal() const { return sym_->IsGlobal(); }
    ValueIndex Offset() const { return offset_idx_; }

private:
    Symbol* sym_;
    ValueIndex offset_idx_;
};

} // namespace papyrus

#endif /* PAPYRUS_VARIABLE_H */
