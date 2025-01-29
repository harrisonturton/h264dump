
configure_file(
    version.h.in
    version.h @ONLY
)

# Libraries
add_subdirectory(buf)

# library
add_library(${LIB} INTERFACE)
target_link_libraries(${LIB} INTERFACE buf)

# binary
add_executable(${BIN} main.c)
target_include_directories(${BIN} PRIVATE "${SRC_DIR}")
target_link_libraries(${BIN} PRIVATE ${LIB})
set_target_properties(${BIN} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
