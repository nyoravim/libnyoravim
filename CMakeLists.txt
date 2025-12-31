cmake_minimum_required(VERSION 3.20)
project(libnyoravim LANGUAGES C)

if(BUILD_SHARED_LIBS)
    set(LIB_TYPE SHARED)
else()
    set(LIB_TYPE STATIC)
endif()

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "src/*.c")
add_library(nyoravim ${LIB_TYPE} ${SRC})

target_include_directories(
    nyoravim PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(
    nyoravim PUBLIC
    pthread
)

add_subdirectory("tests")
