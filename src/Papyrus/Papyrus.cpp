#include "Papyrus/Papyrus.h"
#include "Papyrus/Logger/Logger.h"

#include "FrontEnd/Lexer.h"
#include "FrontEnd/ASTConstructor.h"

#include "IR/IR.h"
#include "IR/IRConstructor.h"

#include "Analysis/ArrayLSRemover.h"
#include "Analysis/DCE.h"

#include "RegAlloc/IGBuilder.h"
#include "RegAlloc/RegAlloc.h"

#include "Utils.h"

#include "Visualizer/Visualizer.h"

using namespace papyrus;

structlog LOGCFG = {};

int main(int argc, char *argv[]) {
    LOGCFG.headers = true;
    LOGCFG.level = ERROR;
    Utils utils;

    if (argc < 2) {
        LOG(ERROR) << "Please input a file to compile!";
        exit(1);
    }

    // Test file
    std::filebuf fb;
    if (!fb.open(argv[1], std::ios::in))
        LOG(ERROR) << "[MAIN] Could not open file!";

    std::istream is(&fb);
    Lexer lexer(is);

    ASTConstructor astconst(lexer);
    astconst.ConstructAST();

    IRConstructor irconst = IRConstructor(astconst);
    irconst.BuildIR();

    ArrayLSRemover als(irconst);
    als.Run();

    Visualizer viz = Visualizer(irconst);

    std::string ir_fname = utils.ConstructOutFile(argv[1], ".vcg");
    viz.WriteIR(ir_fname);

    IGBuilder igb(irconst);
    RegAllocator ra(irconst, igb);
    ra.Run();

    std::string final_fname = utils.ConstructOutFile(argv[1], ".final.vcg");
    viz.UpdateColoring(ra.Coloring());
    viz.WriteFinalIR(final_fname);

    return 0;
}
