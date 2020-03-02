#ifndef PAPYRUS_VISUALIZER_H
#define PAPYRUS_VISUALIZER_H

#include "IR/IRConstructor.h"

#include <fstream>
#include <sstream>

namespace papyrus {

using IRC = IRConstructor;
using T   = Instruction::InstructionType;

class Visualizer {
public:
    Visualizer(IRC&);

    void WriteVCG(const std::string&);

    void UpdateVCG();

    void DrawFunc(const Function*);

    std::string GetBaseNodeString(BI, const std::string&) const;

    std::string CloseNode() const;

    std::string GetEdgeString(const std::string&, BI, BI) const;

private:
    IRC& irc_;
    std::ofstream graph_;
};

} // namespace papyrus

#endif /* PAPYRUS_VISUALIZER_H */
