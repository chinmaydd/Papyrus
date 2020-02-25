#ifndef PAPYRUS_VISUALIZER_H
#define PAPYRUS_VISUALIZER_H

#include "IR/IRConstructor.h"

#include <fstream>
#include <sstream>

namespace papyrus {

using IRC = IRConstructor;

class Visualizer {
public:
    Visualizer(IRC&);

    void WriteVCG();

    void UpdateVCG();

    void DrawFunc(const Function*);

    std::string GetBaseNodeString(BBIndex, const std::string&) const;

    std::string CloseNode() const;

    std::string GetEdgeString(BBIndex, BBIndex) const;

private:
    IRC& irc_;
    std::ofstream graph_;
};

} // namespace papyrus

#endif /* PAPYRUS_VISUALIZER_H */
