file(GLOB_RECURSE src_list "*.cpp")

add_library(common STATIC ${src_list})
add_library(common_test STATIC ${src_list})

target_link_libraries(common
    # model
    # graphics
    # scheduler
    # utils
)
target_link_libraries(common_test
    # model_test
    # graphics_test
    # scheduler_test
    # utils_test
)

set_target_properties(common_test
    PROPERTIES
    COMPILE_FLAGS   "${COVERAGE_CFLAGS}"
    LINK_FLAGS      "${COVERAGE_LDFLAGS}"
)
