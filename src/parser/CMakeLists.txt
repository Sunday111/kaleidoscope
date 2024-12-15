cmake_minimum_required(VERSION 3.16)

project(Kaleidoscope-Parser)
include(set_compiler_options)

set(target_name kaleidoscope-parser)

set(include_dir ${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB_RECURSE hpp_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS "${include_dir}/*")

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)
file(GLOB_RECURSE cpp_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS "${src_dir}/*")

add_library(${target_name} STATIC ${hpp_files} ${cpp_files})
set_generic_compiler_options(${target_name} PRIVATE)
target_include_directories(${target_name} PUBLIC ${include_dir})
target_link_libraries(${target_name} PUBLIC kaleidoscope-lexer fast_float)
