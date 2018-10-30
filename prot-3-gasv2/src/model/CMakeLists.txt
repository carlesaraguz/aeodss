file(GLOB_RECURSE src_list "*.cpp")

add_library(model STATIC ${src_list})
add_library(model_test STATIC ${src_list})

target_link_libraries(model
    common
    scheduler
    utils
)
target_link_libraries(model_test
    common_test
    scheduler_test
    utils_test
)

set_target_properties(model_test
    PROPERTIES
    COMPILE_FLAGS   "${COVERAGE_CFLAGS}"
    LINK_FLAGS      "${COVERAGE_LDFLAGS}"
)
