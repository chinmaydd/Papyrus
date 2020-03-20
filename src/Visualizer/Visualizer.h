#ifndef PAPYRUS_VISUALIZER_H
#define PAPYRUS_VISUALIZER_H

#include "IR/IRConstructor.h"
#include "RegAlloc/RegAlloc.h"

#include <fstream>
#include <sstream>

namespace papyrus {

using IRC   = IRConstructor;
using T     = Instruction::InstructionType;

class Visualizer {
public:
    Visualizer(IRC&);

    void WriteIR(const std::string&);
    void WriteFinalIR(const std::string&);

    void UpdateVCG();

    void UpdateColoring(const std::unordered_map<VI, Color>&);

    void DrawFunc(const Function*);

    std::string GetBaseNodeString(BI, const std::string&) const;

    std::string CloseNode() const;

    std::string GetEdgeString(const std::string&, BI, BI) const;

private:
    IRC& irc_;
    std::ofstream graph_;

    void WriteVCG(const std::string& fname);

    Color GetColor(VI) const;

    bool writing_ir_before_ra_;
    bool RADone() const { return !writing_ir_before_ra_; }

    std::string ConvertInstructionToString(const Function*, II) const;
    std::string RegisterString(VI) const;

    std::unordered_map<VI, Color> coloring_;
};

} // namespace papyrus

#endif /* PAPYRUS_VISUALIZER_H */
