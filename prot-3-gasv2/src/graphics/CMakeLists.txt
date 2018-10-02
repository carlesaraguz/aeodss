file(GLOB_RECURSE src_list "*.cpp")

add_library(graphics STATIC ${src_list})
add_library(graphics_test STATIC ${src_list})

target_link_libraries(graphics
    common
    # model
)
target_link_libraries(graphics_test
    common_test
    # model_test
)

set_target_properties(graphics_test
    PROPERTIES
    COMPILE_FLAGS   "${COVERAGE_CFLAGS}"
    LINK_FLAGS      "${COVERAGE_LDFLAGS}"
)
