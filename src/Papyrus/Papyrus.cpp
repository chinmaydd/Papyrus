#include "Papyrus/Papyrus.h"
#include "Papyrus/Logger/Logger.h"

#include "FrontEnd/Lexer.h"
#include "FrontEnd/ASTConstructor.h"

#include "IR/IR.h"
#include "IR/IRConstructor.h"

#include "Visualizer/Visualizer.h"

using namespace papyrus;

structlog LOGCFG = {};

int main(int argc, char *argv[]) {
    LOGCFG.headers = true;
    LOGCFG.level = ERROR;

    if (argc < 2) {
        LOG(ERROR) << "Please input a file to compile!";
        exit(1);
    }

    // Test file!
    std::filebuf fb;
    if (!fb.open(argv[1], std::ios::in))
        LOG(ERROR) << "[MAIN] Could not open file!";

    std::istream is(&fb);
    Lexer lexer(is);
    
    ASTConstructor astconst(lexer);
    astconst.ConstructAST();

    IRConstructor irconst = IRConstructor(astconst);

    irconst.BuildIR();

    Visualizer viz = Visualizer(irconst);
    viz.WriteVCG();

    return 0;
}
