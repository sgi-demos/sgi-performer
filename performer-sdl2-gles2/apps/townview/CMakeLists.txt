add_executable(townview main.cpp)
target_link_libraries(townview PRIVATE pfb2osg SDL2::SDL2)

if(EMSCRIPTEN)
    # bundle the town database into the wasm virtual filesystem; render in
    # WebGL1 (== GLES2) with memory growth (the town scene is large)
    set_target_properties(townview PROPERTIES SUFFIX ".html")
    target_link_options(townview PRIVATE
        "-sUSE_SDL=2"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sMAX_WEBGL_VERSION=1"
        "--preload-file" "${CMAKE_SOURCE_DIR}/data/town@/data/town")
elseif(APPLE)
    target_link_libraries(townview PRIVATE "-framework OpenGL")
endif()
