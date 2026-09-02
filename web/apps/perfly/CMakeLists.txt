# Pristine SGI perfly, compiled UNMODIFIED from vendor/Performer against the pfosg
# shim.  TUs are added to PERFLY_SRCS as the shim surface grows to cover them.
set(PERFLY ${PF_PERFORMER_SRC_DIR}/sample/C/perfly)
set(COMMON ${PF_PERFORMER_SRC_DIR}/sample/C/common)

set(PERFLY_SRCS
    ${PERFLY}/cmdline.c
    ${PERFLY}/configfile.c
    ${PERFLY}/keybd.c
    ${PERFLY}/gui.c
    ${PERFLY}/perfly.c
    ${COMMON}/env.c
    ${COMMON}/generic.c
    ${COMMON}/main.c
)

add_executable(perfly ${PERFLY_SRCS})
target_include_directories(perfly PRIVATE ${PERFLY} ${COMMON})
target_link_libraries(perfly PRIVATE pfosg)
# 2000-era C plays loose with int/pointer and prototype mismatches; keep the
# pristine sources compiling with warnings, not errors
target_compile_options(perfly PRIVATE -std=gnu89
    -Wno-implicit-function-declaration -Wno-return-type
    -Wno-int-conversion -Wno-incompatible-pointer-types
    -Wno-implicit-int)
