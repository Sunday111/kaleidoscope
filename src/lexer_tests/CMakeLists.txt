cmake_minimum_required(VERSION 3.16)

project(Kaleidoscope-Lexer-Tests)
include(set_compiler_options)

set(target_name kaleidoscope-lexer-tests)

set(include_dir ${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB_RECURSE hpp_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS "${include_dir}/*")

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)
file(GLOB_RECURSE cpp_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS "${src_dir}/*")

add_executable(${target_name} ${hpp_files} ${cpp_files})
set_generic_compiler_options(${target_name} PRIVATE)
target_link_libraries(${target_name} PUBLIC kaleidoscope-lexer gtest_main)
