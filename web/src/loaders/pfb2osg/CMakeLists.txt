add_library(pfb2osg STATIC pfb2osg.cpp)
target_include_directories(pfb2osg PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OPENSCENEGRAPH_INCLUDE_DIRS})
target_link_libraries(pfb2osg PUBLIC ${OPENSCENEGRAPH_LIBRARIES})
