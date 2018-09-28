file(GLOB_RECURSE src_list "*.cpp")

add_library(common STATIC ${src_list})
target_link_libraries(common graphics)

# add_library(utils_test STATIC ${src_list})

# set_target_properties(utils_test
#     PROPERTIES
#     COMPILE_FLAGS   "${COVERAGE_CFLAGS}"
#     LINK_FLAGS      "${COVERAGE_LDFLAGS}"
# )
