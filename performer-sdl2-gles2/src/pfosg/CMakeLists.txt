# SGI's shipped path follower, compiled UNMODIFIED (open-sourced libpfutil).
# fatan2 is an IRIX libm intrinsic; on macOS neither of path.c's guards
# declares it, so an implicit int declaration would garble the float return -
# map calls to atan2f via a function-like macro instead.
set(PFU_PATH_C ${PF_PERFORMER_SRC_DIR}/lib/libpfutil/path.c)
set_source_files_properties(${PFU_PATH_C} PROPERTIES COMPILE_OPTIONS
    "-std=gnu89;-Wno-implicit-function-declaration;-Wno-int-conversion;-Dfatan2(y,x)=atan2f((y),(x))")

# SGI's shipped GUI panel library, also compiled UNMODIFIED; its shim
# support (immediate-mode GL, built-in font, data pools) is pfosg_gui.cpp
set(PFU_GUI_C ${PF_PERFORMER_SRC_DIR}/lib/libpfutil/gui.c)
set_source_files_properties(${PFU_GUI_C} PROPERTIES
    COMPILE_OPTIONS "-std=gnu89;-Wno-int-conversion"
    INCLUDE_DIRECTORIES "${PF_PERFORMER_SRC_DIR}/lib/libpfutil")

# SGI's shipped .pfb/.pfa database loader, also compiled UNMODIFIED; its
# shim support is pfosg_pfb.cpp + generated stubs (see tools/harvest_pfb_api.py).
# fsin/fcos are IRIX libm intrinsics, mapped like path.c's fatan2.
set(PFDB_PFB_C ${PF_PERFORMER_SRC_DIR}/lib/libpfdb/libpfpfb/pfpfb.c)
set_source_files_properties(${PFDB_PFB_C} PROPERTIES
    COMPILE_OPTIONS "-std=gnu89;-Wno-int-conversion;-Dfcos=cosf;-Dfsin=sinf"
    INCLUDE_DIRECTORIES "${PF_PERFORMER_SRC_DIR}/lib/libpfdb/libpfpfb")

add_library(pfosg STATIC pfosg.cpp pfosg_perfly.cpp pfosg_gui.cpp
            pfosg_pfb.cpp pfosg_pfb_stubs.cpp
            ${PFU_PATH_C} ${PFU_GUI_C} ${PFDB_PFB_C})
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
