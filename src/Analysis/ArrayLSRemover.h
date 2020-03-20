#ifndef PAPYRUS_ARRAYLSREMOVER_H
#define PAPYRUS_ARRAYLSREMOVER_H

#include "AnalysisPass.h"

namespace papyrus {
class ArrayLSRemover : public AnalysisPass {
public:
    ArrayLSRemover(IRConstructor& irc) : AnalysisPass(irc) {}
    void Run();

private:
    std::unordered_map<std::string, bool> kill_status_;
    std::unordered_map<std::string, std::string> hash_val_;
    std::unordered_map<std::string, VI> live_;
    
    void Recurse(Function*, Instruction*);
};
} // namespace papyrus

#endif /* PAPYRUS_ARRAYLSREMOVER_H */
