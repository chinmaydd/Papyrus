set(_SOURCE_FILES
    Instruction.cpp
    IRConstructor.cpp
    )

add_library(IR OBJECT
    ${_SOURCE_FILES})
