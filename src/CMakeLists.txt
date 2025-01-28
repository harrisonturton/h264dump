# Libraries
add_subdirectory(buf)

# library
add_library(${LIB} INTERFACE)
target_link_libraries(${LIB} INTERFACE buf)

# binary
add_executable(${BIN} main.c)
target_include_directories(${BIN} PUBLIC "${INCLUDE_DIR}")
target_include_directories(${BIN} PRIVATE "${SRC_DIR}")
target_link_libraries(${BIN} PRIVATE ${LIB})
