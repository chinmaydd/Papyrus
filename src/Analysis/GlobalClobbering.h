#ifndef PAPYRUS_GLOBAL_CLOBBERING_H
#define PAPYRUS_GLOBAL_CLOBBERING_H

#include "AnalysisPass.h"
#include "InterprocCall.h"

#include <stack>

namespace papyrus {

using T = Instruction::InstructionType;

class GlobalClobbering : public AnalysisPass {
public:
    GlobalClobbering(IRConstructor&);
    void run();

    const std::unordered_map<std::string, std::unordered_map<std::string, bool> >& GetClobberStatus() const;
    const std::unordered_map<std::string, std::unordered_map<std::string, bool> >& GetReadDefStatus() const;

private:
    std::unordered_map<std::string, std::unordered_map<std::string, bool> > clobbered_vars_;
    std::unordered_map<std::string, std::unordered_map<std::string, bool> > read_vars_;

    std::unordered_map<std::string, bool> visited_;

    std::unordered_map<std::string, std::unordered_map<std::string, bool> > callee_info_;

    
    bool IsMemoryStore(T) const;
    bool IsMemoryLoad(T) const;
    bool IsFunctionCall(T) const;

    void Visit(const std::string&);

    /*
     * Check here if we are clobbering variables which are shadowed by a 
     * local definition of a global. This is ideally taken care of in the IR
     * but just to be safe.
     * UPDATE: Checked. Works.
     */
    void Clobber(const std::string&, const std::string&);
    void ReadDef(const std::string&, const std::string&);
};

} // namespace papyrus

#endif /* PAPYRUS_GLOBAL_CLOBBERING_H */
