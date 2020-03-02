#include "LoadStoreRemover.h"

using namespace papyrus;

void LoadStoreRemover::run() {
    GlobalClobbering gc = GlobalClobbering(irc());
    gc.run();

    global_clobber_ = gc.GetClobberStatus();

    for (auto fn_pair: irc().Functions()) {
        auto fn_name = fn_pair.first;
        if (fn_name == "main") {
            auto fn = fn_pair.second;
            // Assumption made that BB with idx 1 is entry.
            // XXX: This could however, change.
            auto entry_bb = fn->BasicBlocks().at(1);
            std::stack<BasicBlock*> worklist;
            worklist.push(entry_bb);

            while (!worklist.empty()) {
                /*
                 * VisitBB
                 * Assume that the global is a local var
                 * create local defs; make sure that there is no clobbering 
                 * of
                 */
                
                 
            }
        }
    }
}
