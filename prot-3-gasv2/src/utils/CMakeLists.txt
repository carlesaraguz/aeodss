file(GLOB_RECURSE utils_list "*.cpp")

add_library(utils STATIC ${utils_list})
add_library(utils_test STATIC ${utils_list})

set_target_properties(utils_test
    PROPERTIES
    COMPILE_FLAGS   "${COVERAGE_CFLAGS}"
    LINK_FLAGS      "${COVERAGE_LDFLAGS}"
)
