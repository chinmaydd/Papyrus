#include "Utils.h"

using namespace papyrus;

std::vector<std::string> Utils::Split(const std::string& str, const std::string& delim) {
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;

    do {
        pos = str.find(delim, prev);

        if (pos == std::string::npos) {
            pos = str.length();
        }

        std::string token = str.substr(prev, pos-prev);

        if (!token.empty()) {
            tokens.push_back(token);
        }

        prev = pos + delim.length();
    } while (pos < str.length() && prev < str.length());

    return tokens;
}

std::string Utils::ConstructOutFile(const std::string& fpath, const std::string& suffix) {
    std::string root = "/home/chinmay_dd/Projects/Papyrus/tests/public/";
    std::string fname = Split(Split(fpath, "/").back(), ".").front() + suffix;

    return root + fname;
}
