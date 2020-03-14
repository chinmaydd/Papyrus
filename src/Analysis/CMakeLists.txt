set(_SOURCE_FILES
    LiveVar.cpp
    GlobalClobbering.cpp
    InterprocCall.cpp
    LoadStoreRemover.cpp
    )

add_library(Analysis OBJECT
    ${_SOURCE_FILES})
