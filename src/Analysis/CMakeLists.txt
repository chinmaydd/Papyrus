set(_SOURCE_FILES
    AnalysisPass.cpp
    GlobalClobbering.cpp
    InterprocCall.cpp
    LoadStoreRemover.cpp
    CSE.cpp
    )

add_library(Analysis OBJECT
    ${_SOURCE_FILES})
