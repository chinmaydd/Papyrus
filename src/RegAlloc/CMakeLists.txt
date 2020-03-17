set(_SOURCE_FILES
    IGBuilder.cpp
    RegAlloc.cpp
    )

add_library(RegAlloc OBJECT
    ${_SOURCE_FILES})
