cmake_minimum_required(VERSION 3.19)
project(chopper C)

set(CMAKE_C_STANDARD 99)

add_compile_options(-Wall -Wextra -pedantic -Werror)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBNL REQUIRED IMPORTED_TARGET libnl-genl-3.0)

add_executable(chopper src/main.c src/utils.c) # src/cJSON.c
target_link_libraries(chopper PRIVATE PkgConfig::LIBNL)