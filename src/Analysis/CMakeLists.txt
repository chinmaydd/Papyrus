set(_SOURCE_FILES
    ConstantFolding.cpp
    CSE.cpp
    )

add_library(Analysis OBJECT
    ${_SOURCE_FILES})
