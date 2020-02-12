#ifndef PAPYRUS_VALUEMAP_H
#define PAPYRUS_VALUEMAP_H

#include "Papyrus/Logger/Logger.h"

#include <unordered_map>

namespace papyrus {
class ValueMap {
private:
    std::unordered_map<ValueIndex, Value*> value_map_;
};
} // namespace papyrus

#endif /* PAPYRUS_VALUEMAP_H */
