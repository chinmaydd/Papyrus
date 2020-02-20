set(_SOURCE_FILES
    Variable.cpp
    IR.cpp
    IRConstructor.cpp
    )

add_library(IR OBJECT
    ${_SOURCE_FILES})
