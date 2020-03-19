#include "Papyrus/Papyrus.h"
#include "Papyrus/Logger/Logger.h"

#include "FrontEnd/Lexer.h"
#include "FrontEnd/ASTConstructor.h"

#include "IR/IR.h"
#include "IR/IRConstructor.h"

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

    // DCE dce(irconst);
    // dce.Run();

    std::string vcg_fname = utils.ConstructOutFile(argv[1], ".vcg");
    Visualizer viz = Visualizer(irconst);
    viz.WriteVCG(vcg_fname);

    IGBuilder igb(irconst);
    igb.Run();

    RegAllocator ra(irconst, igb);
    ra.Run();

    // vcg_fname = utils.ConstructOutFile(argv[1], ".final.vcg");
    // Visualizer viz = Visualizer(irconst);
    // viz.WriteVCG(vcg_fname);

    return 0;
}
