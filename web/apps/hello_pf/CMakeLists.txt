add_executable(hello_pf main.cpp)
target_link_libraries(hello_pf PRIVATE SDL2::SDL2)

if(EMSCRIPTEN)
    # Plain WebGL1 (= GLES2). VBO-only drawing, so no -sFULL_ES2 needed.
    set_target_properties(hello_pf PROPERTIES SUFFIX ".html")
    target_link_options(hello_pf PRIVATE "-sMIN_WEBGL_VERSION=1" "-sMAX_WEBGL_VERSION=1")
elseif(APPLE)
    # SDL_GL_GetProcAddress resolves against the OpenGL framework.
    target_link_libraries(hello_pf PRIVATE "-framework OpenGL")
endif()
