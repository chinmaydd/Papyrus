include_directories(${CMAKE_SOURCE_DIR}/src)

set(_SOURCE_FILES
    Utils.cpp
    Papyrus.cpp
    )

add_executable(papyrus 
  ${_SOURCE_FILES})

target_link_libraries(papyrus
    $<TARGET_OBJECTS:FrontEnd>
    $<TARGET_OBJECTS:IR>
    $<TARGET_OBJECTS:Analysis>
    $<TARGET_OBJECTS:RegAlloc>
    $<TARGET_OBJECTS:Visualizer>)
