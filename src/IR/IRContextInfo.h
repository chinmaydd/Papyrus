#ifndef PAPYRUS_IRCONTEXTINFO_H
#define PAPYRUS_IRCONTEXTINFO_H

#include <unordered_map>

namespace papyrus {
class IRContextInfo {
public:
    IRContextInfo();

    void AddGlobalVariable(Variable*);

private:
    std::unordered_map<std::string, Function*> function_map_;
    std::unordered_map<std::string, Variable*> global_variables_;
}; 

} // namespace papyrus

#endif /* PAPYRUS_IRCONTEXTINFO_H */
