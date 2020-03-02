set(_SOURCE_FILES
    GlobalClobbering.cpp
    LoadStoreRemover.cpp
    CSE.cpp
    )

add_library(Analysis OBJECT
    ${_SOURCE_FILES})
