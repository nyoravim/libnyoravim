cmake_minimum_required(VERSION 3.20)
include(CTest)

file(GLOB TEST_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
foreach(SRC_FILE ${TEST_SRC})
    cmake_path(GET SRC_FILE STEM TEST_SRC_NAME)
    set(TEST_NAME nv_test_${TEST_SRC_NAME})

    add_executable(${TEST_NAME} ${SRC_FILE})
    target_link_libraries(${TEST_NAME} nyoravim)

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
