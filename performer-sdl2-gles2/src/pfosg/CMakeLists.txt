add_library(pfosg STATIC pfosg.cpp pfosg_perfly.cpp)
# shim include dir first; the pristine Performer include tree follows it so
# the shim's pfutil.h/pfui.h can #include_next the real ones (and so samples
# can include <Performer/prmath.h> directly)
target_include_directories(pfosg PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PF_PERFORMER_INCLUDE_DIR}
    ${OPENSCENEGRAPH_INCLUDE_DIRS})
target_link_libraries(pfosg PUBLIC pfb2osg SDL2::SDL2)
if(APPLE)
    target_link_libraries(pfosg PUBLIC "-framework OpenGL")
endif()
