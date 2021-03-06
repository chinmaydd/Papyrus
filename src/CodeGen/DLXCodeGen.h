#ifndef PAPYRUS_DLXCODEGEN_H
#define PAPYRUS_DLXCODEGEN_H

#include "Papyrus/Logger/Logger.h"
#include "IR/IR.h"
#include "IR/IRConstructor.h"

namespace papyrus {

/* 
 * "I had such plans."
 *  - chinmay_dd, March 2020.
 */

class CodeGenerator {
public:
    CodeGenerator(IRConstructor& irc) : irc_(irc) {}

private:
    IRConstructor irc_;
};

} // namespace papyrus

#endif /* PAPYRUS_DLXCODEGEN_H */
