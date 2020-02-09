set(_SOURCE_FILES
    BasicBlock.cpp
    Instruction.cpp
    IRConstructor.cpp
    )

add_library(IR OBJECT
    ${_SOURCE_FILES})
