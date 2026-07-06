# Pristine SGI programming-guide samples, compiled UNMODIFIED from vendor/Performer
# against the pfosg shim headers.
set(PGUIDE ${PF_PERFORMER_SRC_DIR}/pguide/libpf/C)

foreach(sample texture lod_func earthsky intersect)
    add_executable(${sample} ${PGUIDE}/${sample}.c)
    target_link_libraries(${sample} PRIVATE pfosg)
    # 2000-era C: implicit declarations (exit without <stdlib.h>) must stay
    # warnings, not errors, since the samples are compiled untouched
    target_compile_options(${sample} PRIVATE -std=gnu89
        -Wno-implicit-function-declaration)
endforeach()
