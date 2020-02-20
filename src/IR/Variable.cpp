#include "Variable.h"

using namespace papyrus;

Symbol::Symbol(const std::string& identifier, const std::vector<int>& dimensions, bool if_array, bool is_global) :
    identifier_(identifier),
    dimensions_(dimensions),
    if_array_(if_array),
    is_global_(is_global) {}
