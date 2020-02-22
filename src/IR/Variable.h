#ifndef PAPYRUS_VARIABLE_H
#define PAPYRUS_VARIABLE_H

#include "Papyrus/Logger/Logger.h"

#include <vector>

namespace papyrus {
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
    Variable(Symbol*, int);
    bool IsArray() const { return sym_->IsArray(); }
    bool IsGlobal() const { return sym_->IsGlobal(); }
    int GetOffset() const { return offset_in_words_; }

private:
    Symbol* sym_;
    int offset_in_words_;
};

} // namespace papyrus

#endif /* PAPYRUS_VARIABLE_H */
