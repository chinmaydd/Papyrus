#ifndef PAPYRUS_VISUALIZER_H
#define PAPYRUS_VISUALIZER_H

#include "IR/IRConstructor.h"

#include <fstream>

namespace papyrus {

using IRC = IRConstructor;

class Visualizer {
public:
    Visualizer(IRC&);

    void WriteVCG();

    void UpdateVCG();

private:
    IRC& irc_;
    std::ofstream graph_;
};

} // namespace papyrus

#endif /* PAPYRUS_VISUALIZER_H */
