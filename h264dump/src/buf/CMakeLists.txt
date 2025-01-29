
add_library(buf
    buf.c
    file.c
    mem.c
    nal2.c)

target_include_directories(buf
    PUBLIC "${SRC_DIR}"
    PRIVATE "${INCLUDE_DIR}")
