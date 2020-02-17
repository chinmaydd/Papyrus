set(_SOURCE_FILES
    Graph.cpp
    ASTWalker.cpp
    IRConstructor.cpp
    )

add_library(IR OBJECT
    ${_SOURCE_FILES})
