
function(create_test NAME SRCS)
    add_executable(${NAME} "${SRCS}")
    target_include_directories(${NAME} PRIVATE "${SRC_DIR}" "${TEST_DIR}")
    target_link_libraries(${NAME} PRIVATE ${LIB})
    add_test(NAME ${NAME} COMMAND ${NAME})
endfunction()

create_test(buf_test buf/buf_test.c)
