
function(create_test NAME SRCS)
    add_executable(${NAME} "${SRCS}")
    target_include_directories(${NAME}
        PUBLIC "${INCLUDE_DIR}"
        PRIVATE "${SRC_DIR}")
    target_link_libraries(${NAME}
        PRIVATE ${LIB})

    add_test(NAME ${NAME} COMMAND ${NAME})
endfunction()

create_test(buf_file_test buf/file_test.c)
