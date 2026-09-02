# webfly - SGI perfly compiled UNMODIFIED for the web (Emscripten/WebGL),
# GUI-less.  Same TUs as apps/perfly; the pfosg shim renders the town scene
# through GLES2 and runs perfly's blocking frame loop under Asyncify.
set(PERFLY ${PF_PERFORMER_SRC_DIR}/sample/C/perfly)
set(COMMON ${PF_PERFORMER_SRC_DIR}/sample/C/common)

set(WEBFLY_SRCS
    ${PERFLY}/cmdline.c
    ${PERFLY}/configfile.c
    ${PERFLY}/keybd.c
    ${PERFLY}/gui.c
    ${PERFLY}/perfly.c
    ${COMMON}/env.c
    ${COMMON}/generic.c
    ${COMMON}/main.c
)

add_executable(webfly ${WEBFLY_SRCS})
target_include_directories(webfly PRIVATE ${PERFLY} ${COMMON})
target_link_libraries(webfly PRIVATE pfosg)
target_compile_options(webfly PRIVATE -std=gnu89
    -Wno-implicit-function-declaration -Wno-return-type
    -Wno-int-conversion -Wno-incompatible-pointer-types
    -Wno-implicit-int)

set_target_properties(webfly PROPERTIES SUFFIX ".html")
target_link_options(webfly PRIVATE
    "-sUSE_SDL=2"
    "-sALLOW_MEMORY_GROWTH=1"
    "-sMAX_WEBGL_VERSION=1"
    # perfly's main() runs a blocking frame loop; Asyncify lets pfFrame yield
    # to the browser (emscripten_sleep) each frame without restructuring it
    "-sASYNCIFY=1"
    "-sASYNCIFY_STACK_SIZE=16384"
    "--shell-file" "${CMAKE_CURRENT_SOURCE_DIR}/shell.html"
    # whole data dir -> /data (town database + vehicles + paths + configs);
    # the tether config's "-F .:data/town" search path resolves against the
    # default MEMFS working dir "/", so /data/town is reachable
    "--preload-file" "${CMAKE_SOURCE_DIR}/data@/data")
set_property(TARGET webfly APPEND PROPERTY LINK_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/shell.html")
