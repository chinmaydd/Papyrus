set(_SOURCE_FILES
    GlobalClobbering.cpp
    InterprocCall.cpp
    LoadStoreRemover.cpp
    IGBuilder.cpp
    )

add_library(Analysis OBJECT
    ${_SOURCE_FILES})
