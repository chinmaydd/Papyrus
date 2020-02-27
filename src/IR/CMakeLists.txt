set(_SOURCE_FILES
    Variable.cpp
    SSA.cpp
    IR.cpp
    ASTWalk.cpp
    IRConstructor.cpp
    )

add_library(IR OBJECT
    ${_SOURCE_FILES})
