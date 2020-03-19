set(_SOURCE_FILES
    GlobalClobbering.cpp
    InterprocCall.cpp
    CSE.cpp
    DCE.cpp
    )

add_library(Analysis OBJECT
    ${_SOURCE_FILES})
