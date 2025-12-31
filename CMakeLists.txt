cmake_minimum_required(VERSION 3.20)
project(libnyoravim LANGUAGES C)

if(BUILD_SHARED_LIBS)
    set(LIB_TYPE SHARED)
else()
    set(LIB_TYPE STATIC)
endif()

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "src/*.c")
add_library(nyoravim ${LIB_TYPE} ${SRC})

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
file(GLOB_RECURSE PUB_HEADERS CONFIGURE_DEPENDS "${INCLUDE_DIR}/*.h")
target_sources(
    nyoravim PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${INCLUDE_DIR}
    FILES ${PUB_HEADERS}
)

target_include_directories(
    nyoravim PUBLIC
    $<BUILD_INTERFACE:${INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(
    nyoravim PUBLIC
    pthread
)

set_target_properties(
    nyoravim PROPERTIES
    VERSION 1
)

install(
    TARGETS nyoravim
    EXPORT libnyoravim
    FILE_SET HEADERS
)

install(
    EXPORT libnyoravim
    DESTINATION "lib/cmake/libnyoravim/"
    NAMESPACE libnyoravim
    FILE "libnyoravim-config.cmake"
)

install(
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
    DESTINATION "share/licenses/libnyoravim/"
)

add_subdirectory("tests")
