cmake_minimum_required(VERSION 3.28.3)
project(ch32v003_nixie C)

set(CMAKE_C_STANDARD 99)

find_package(Python3 COMPONENTS Interpreter)
if (NOT Python3_EXECUTABLE)
    message(FATAL_ERROR "Python3 interpreter not found. Please install Python3 to generate version header.")
endif ()

find_program(GIT_EXECUTABLE git)
if (NOT GIT_EXECUTABLE)
    message(WARNING "git not found; version header will be generated without git metadata unless git is available at build time")
endif ()

set(VERSION_HEADER_DIR ${CMAKE_BINARY_DIR}/generated/include)
set(VERSION_HEADER ${VERSION_HEADER_DIR}/version.h)

add_custom_target(generate_version_header
    COMMAND ${CMAKE_COMMAND} -E make_directory ${VERSION_HEADER_DIR}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/generate_version_header.py
        -o ${VERSION_HEADER}
        --git-exec ${GIT_EXECUTABLE}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating version header ${VERSION_HEADER}"
    VERBATIM
)

add_compile_options(-Os -g -march=rv32ec_zicsr -mabi=ilp32e)

add_executable(ch32v003_nixie
        main.c
        src/i2c.c
        src/pid.c
        src/display.c
        src/flyback_driver.c
)

add_subdirectory(lib/ch32v003fun)

target_include_directories(ch32v003_nixie PRIVATE ${VERSION_HEADER_DIR})
target_include_directories(ch32v003_nixie PRIVATE include)
add_dependencies(ch32v003_nixie generate_version_header)

target_link_libraries(
        ${CMAKE_PROJECT_NAME}
        ch32v003fun
        gcc
)
target_link_options(
        ${CMAKE_PROJECT_NAME}
        PRIVATE
        -specs=nosys.specs
        -nostartfiles
        -nostdlib
        -march=rv32ec_zicsr -mabi=ilp32e
)


add_custom_command(
        TARGET ${CMAKE_PROJECT_NAME}
        POST_BUILD
        COMMAND size -G ${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}
)