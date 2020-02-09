#ifndef PAPYRUS_OPERATION_H
#define PAPYRUS_OPERATION_H

namespace papyrus {
enum RelationalOperator {
    RELOP_NONE,
    RELOP_EQ,
    RELOP_NEQ,
    RELOP_LT,
    RELOP_LTE,
    RELOP_GT,
    RELOP_GTE,
};

enum ArithmeticOperator {
    BINOP_NONE,
    BINOP_MUL,
    BINOP_ADD,
    BINOP_SUB,
    BINOP_DIV,
};

} // namespace papyrus

#endif
