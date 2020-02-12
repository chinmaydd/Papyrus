#ifndef PAPYRUS_VALUE_H
#define PAPYRUS_VALUE_H

#include "Papyrus/Logger/Logger.h"

#include <string>

namespace papyrus {
class Value {};

class ConstantValue : public Value {

private:
    long int value_;

};

class NamedValue :  public Value {

private:
    std::string identifier_;
};

}

#endif /* PAPYRUS_VALUE_H */
