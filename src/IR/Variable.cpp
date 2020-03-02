#include "Variable.h"

using namespace papyrus;

Symbol::Symbol(const std::string& identifier, const std::vector<int>& dimensions, bool if_array, bool is_global, bool is_formal) :
    identifier_(identifier),
    dimensions_(dimensions),
    if_array_(if_array),
    is_global_(is_global),
    is_formal_(is_formal) {}

Variable::Variable(Symbol* sym, int offset, VI vi) :
    sym_(sym),
    offset_(offset),
    vi_(vi) {}

Variable::Variable(Symbol* sym, VI vi) :
    sym_(sym),
    vi_(vi) {}
