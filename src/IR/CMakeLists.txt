set(_SOURCE_FILES
    ASTWalker.cpp
    IRConstructor.cpp
    )

add_library(IR OBJECT
    ${_SOURCE_FILES})
