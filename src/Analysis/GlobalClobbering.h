#ifndef PAPYRUS_GLOBAL_CLOBBERING_H
#define PAPYRUS_GLOBAL_CLOBBERING_H

#include "AnalysisPass.h"

namespace papyrus {

using T = Instruction::InstructionType;

class GlobalClobbering : public AnalysisPass {
public:
    GlobalClobbering(IRConstructor& irc) : AnalysisPass(irc) {}
    void run();

    const std::unordered_map<std::string, std::unordered_map<std::string, bool> >& GetClobberStatus() const;

private:
    std::unordered_map<std::string, std::unordered_map<std::string, bool> > clobbered_vars_;
    
    bool IsMemoryStore(T) const;

    /*
     * TODO: Check here if we are clobbering variables which are shadowed by a 
     * local definition of a global. This is ideally taken care of in the IR
     * but just to be safe
     */
    void Clobber(const std::string&, const std::string&);
};

} // namespace papyrus

#endif /* PAPYRUS_GLOBAL_CLOBBERING_H */
