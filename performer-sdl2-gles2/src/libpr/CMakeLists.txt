# Reconstructed libpr math layer.  The slices currently live in the repo
# root (libpr__pfGeoMath_*.cpp, libpr__pfLinMath_*.cpp); they compile against
# the original <Performer/...> headers (vendor/Performer/Include).
#
# NOTE: some root slices overlap (e.g. pfLinMath_vec3.cpp vs the C-API
# wrappers in pf_linmath_sample.cpp).  Prune duplicates here as the slices
# get merged into proper per-file translation units under src/libpr/.
file(GLOB PF_LIBPR_MATH_SOURCES ${CMAKE_SOURCE_DIR}/libpr__*.cpp)
if(NOT PF_LIBPR_MATH_SOURCES)
    message(STATUS "pr_math skipped: no reconstructed libpr__*.cpp slices "
                   "present (they are local-only, not committed)")
    return()
endif()

add_library(pr_math STATIC
    ${PF_LIBPR_MATH_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/pfCore.cpp)
# compat shims must come before the pristine Performer include tree;
# Include/Performer itself is needed because pr.h does #include <ulocks.h>.
target_include_directories(pr_math PUBLIC
    ${CMAKE_SOURCE_DIR}/src/compat
    ${PF_PERFORMER_INCLUDE_DIR}
    ${PF_PERFORMER_INCLUDE_DIR}/Performer)
target_compile_options(pr_math PUBLIC -include ${CMAKE_SOURCE_DIR}/src/compat/pfcompat.h)
if(APPLE)
    target_link_libraries(pr_math PUBLIC "-framework OpenGL")
endif()
