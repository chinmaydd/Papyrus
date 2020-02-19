#ifndef PAPYRUS_VARIABLE_H
#define PAPYRUS_VARIABLE_H

#include "Papyrus/Logger/Logger.h"

#include <vector>

namespace papyrus {
class Symbol {
public:
    Symbol(const std::string&, const std::vector<int>&, bool, bool);
    const std::string& GetIdentifierName() const { return identifier_; }
private:
    std::string identifier_;
    std::vector<int> dimensions_;
    bool if_array_;
    bool is_global_;
};

class Variable {
public:
    Variable(const Symbol*, int);
private:
    const Symbol* sym_;
    int offset_in_words_;
};

} // namespace papyrus

#endif /* PAPYRUS_VARIABLE_H */
