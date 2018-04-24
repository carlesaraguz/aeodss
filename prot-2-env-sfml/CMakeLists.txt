cmake_minimum_required(VERSION 3.2)
project(prot-2)

# Build environment setup (for GNU G++): -----------------------------------------------------------
set(CMAKE_CXX_STANDARD 11)          # C++11 Standard.
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Force required standard.
set(CMAKE_CXX_EXTENSIONS ON)        # Enable compiler extensions (gnu++11).
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-reorder -O2 -pthread")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# Package dependencies: ----------------------------------------------------------------------------
find_package(SFML 2.3 REQUIRED graphics window system)
if(SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    link_libraries(${SFML_LIBRARIES} ${SFML_DEPENDENCIES})
endif()

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
add_subdirectory(src)

# Project's executable (and main file): ------------------------------------------------------------
add_executable(prot-2 main.cpp)
target_link_libraries(prot-2
    model
    graphics
    common
    scheduler
)

set_target_properties(prot-2
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/../lib"
    LIBRARY_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/../lib"
    RUNTIME_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/../bin"
)
