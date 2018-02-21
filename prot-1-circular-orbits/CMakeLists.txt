cmake_minimum_required(VERSION 3.2)
project(prot-1)

# Build environment setup (for GNU G++): -----------------------------------------------------------
set(CMAKE_CXX_STANDARD 11)          # C++11 Standard.
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Force required standard.
set(CMAKE_CXX_EXTENSIONS ON)        # Enable compiler extensions (gnu++11).
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-reorder -O2 -pthread")

# Package dependencies: ----------------------------------------------------------------------------
find_package(RapidJSON REQUIRED)
include_directories(SYSTEM ${RapidJSON_INCLUDE_DIRS})
add_definitions(-DRAPIDJSON_HAS_STDSTRING=1)
add_definitions(-DRAPIDJSON_WRITE_DEFAULT_FLAGS=3)

# Project's header locations: ----------------------------------------------------------------------
file(GLOB_RECURSE headers_list "*.hpp")
set(global_include_dirs "")
foreach(hfile ${headers_list})
    get_filename_component(hdir ${hfile} DIRECTORY)
    list(APPEND global_include_dirs ${hdir})
endforeach()
list(REMOVE_DUPLICATES global_include_dirs)
include_directories(${global_include_dirs})     # Adds all the folders with headers insdide.)


# Project's source files to compile: ---------------------------------------------------------------
file(GLOB_RECURSE sources_list "src/*.cpp")
add_library(dependencies STATIC ${sources_list})


# Project's executable (and main file): ------------------------------------------------------------
add_executable(prot-1 main.cpp)
target_link_libraries(prot-1 dependencies)

set_target_properties(prot-1
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/../lib"
    LIBRARY_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/../lib"
    RUNTIME_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/../bin"
)
