file(GLOB_RECURSE src_list "*.cpp")

add_library(scheduler STATIC ${src_list})
add_library(scheduler_test STATIC ${src_list})

set_target_properties(scheduler_test
    PROPERTIES
    COMPILE_FLAGS   "${COVERAGE_CFLAGS}"
    LINK_FLAGS      "${COVERAGE_LDFLAGS}"
)
