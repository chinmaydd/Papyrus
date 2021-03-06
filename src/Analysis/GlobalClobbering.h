#ifndef PAPYRUS_GLOBAL_CLOBBERING_H
#define PAPYRUS_GLOBAL_CLOBBERING_H

#include "AnalysisPass.h"
#include "InterprocCall.h"

#include <stack>

namespace papyrus {

using T = Instruction::InstructionType;
using VarMap = std::unordered_map<std::string, std::unordered_set<std::string> >;

/*
 * GlobalClobbering Analysis tells us which functions "clobber" and "read" which 
 * variables. "main" is not analysed initially. GlobalClobbering is used
 * during IR Construction before analysis of main. This helps us identify
 * which variavbles are actually global and which can be treated as locals
 * by the IR Construction algorithm
 */

class GlobalClobbering : public AnalysisPass {
public:
    GlobalClobbering(IRConstructor&);
    void Run();

    const VarMap& GetClobberStatus() const;
    const VarMap& GetReadDefStatus() const;

private:
    VarMap clobbered_vars_;
    VarMap read_vars_;

    std::unordered_set<std::string> visited_;

    VarMap callee_info_;

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
