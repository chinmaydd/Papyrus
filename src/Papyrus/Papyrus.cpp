#include "Papyrus/Papyrus.h"
#include "Papyrus/Logger/Logger.h"

#include "FrontEnd/Lexer.h"
#include "FrontEnd/AST.h"
#include "FrontEnd/ASTConstructor.h"

using namespace papyrus;

structlog LOGCFG = {};

int main(int argc, char *argv[]) {
    // Logger configuration (optional)
    LOGCFG.headers = true;
    LOGCFG.level = DEBUG;

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

    ASTConstructor* ASTConst = new ASTConstructor(lexer);
    ComputationNode* root = ASTConst->ComputeAST();

    return 0;
}
