#include "ConstantFolding.h"

using namespace papyrus;

#define IR irc_

void ConstantFolding::run() {
    for (auto fn: IR.Functions()) {
        if (IR.IsIntrinsic(fn.first)) {
            continue;
        }

        for (auto bb: fn.second->BasicBlocks()) {
        }
    }
}
