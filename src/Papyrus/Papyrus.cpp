#include "Papyrus/Papyrus.h"
#include "Papyrus/Logger/Logger.h"

#include "FrontEnd/Lexer.h"

using namespace papyrus;

structlog LOGCFG = {};

int main(int argc, char *argv[]) {
    // Logger configuration (optional)
    LOGCFG.headers = true;
    LOGCFG.level = DEBUG;

    // Log output
    // LOG(INFO) << "Hello, world!";

    // Test file!
    std::filebuf fb;
    if (!fb.open("../tests/test001.txt", std::ios::in))
        LOG(ERROR) << "[MAIN] Could not open file!";

    std::istream is(&fb);
    Lexer lexer(is);

    // Testing!
    while (lexer.GetNextToken() != Lexer::TOK_DOT ||
           lexer.GetNextToken() != Lexer::TOK_EOF) {
        LOG(INFO) << lexer.get_buffer();
    }

    return 0;
}
