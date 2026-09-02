# The pristine SGI sample, compiled UNMODIFIED from vendor/Performer against the
# pfosg shim headers (which shadow <Performer/...>).
add_executable(simple
    ${PF_PERFORMER_SRC_DIR}/pguide/libpf/C/simple.c)
target_link_libraries(simple PRIVATE pfosg)
