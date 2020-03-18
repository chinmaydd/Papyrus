#ifndef PAPYRUS_UTILS_H
#define PAPYRUS_UTILS_H

#include <vector>
#include <string>

namespace papyrus {

class Utils {
public:
    std::vector<std::string> Split(const std::string&, const std::string&);
    std::string ConstructOutFile(const std::string&, const std::string&);
};

} // namespace papyrus

#endif /* PAPYRUS_UTILS_H */
