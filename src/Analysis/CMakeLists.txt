set(_SOURCE_FILES
    GlobalClobbering.cpp
    InterprocCall.cpp
    DCE.cpp
    ArrayLSRemover.cpp
    )

add_library(Analysis OBJECT
    ${_SOURCE_FILES})
