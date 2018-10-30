file(GLOB_RECURSE utils_list "*.cpp")

add_library(utils STATIC ${utils_list})
add_library(utils_test STATIC ${utils_list})

target_link_libraries(utils
    common
)
target_link_libraries(utils_test
    common_test
)

set_target_properties(utils_test
    PROPERTIES
    COMPILE_FLAGS   "${COVERAGE_CFLAGS}"
    LINK_FLAGS      "${COVERAGE_LDFLAGS}"
)
