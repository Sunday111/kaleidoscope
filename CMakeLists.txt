cmake_minimum_required(VERSION 3.16)

project(Kaleidoscope)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
# Set output directories for archives
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
# Set output directories for libraries
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Path to cmake modules
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(ZLIB_USE_STATIC_LIBS "ON")

# Enable LTO by default for all projects
include(lto_utility)
enable_lto_globally()

# Dependencies
include(FetchContent)

# Google Test
option(BUILD_GMOCK "" OFF)
option(INSTALL_GTEST "" OFF)
FetchContent_Declare(
  gtest
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG        "v1.15.2"
  GIT_SHALLOW    1
)
FetchContent_MakeAvailable(gtest)

# Magic Enum
FetchContent_Declare(
  magic_enum
  GIT_REPOSITORY https://github.com/Neargye/magic_enum
  GIT_TAG        "v0.9.7"
  GIT_SHALLOW    1
)
FetchContent_MakeAvailable(magic_enum)

# ASS
option(ASS_ENABLE_TESTING "" ON)
option(ASS_FETCH_GOOGLE_TESTS "" OFF)
option(ASS_FIND_GTEST_PACKAGE "" OFF)
FetchContent_Declare(
  ass
  GIT_REPOSITORY https://github.com/Sunday111/ass
  GIT_TAG        "main"
  GIT_SHALLOW    1
)
FetchContent_MakeAvailable(ass)

# fast float
option(FASTFLOAT_INSTALL "" OFF)
FetchContent_Declare(
  fast_float
  GIT_REPOSITORY https://github.com/fastfloat/fast_float
  GIT_TAG        "v7.0.0"
  GIT_SHALLOW    1
)
FetchContent_MakeAvailable(fast_float)

# # cpptrace
# FetchContent_Declare(
#   cpptrace
#   GIT_REPOSITORY https://github.com/jeremy-rifkin/cpptrace
#   GIT_TAG        "v0.7.4"
#   GIT_SHALLOW    1
# )
# FetchContent_MakeAvailable(cpptrace)

add_subdirectory(src)
