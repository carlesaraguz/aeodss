file(GLOB_RECURSE src_list "*.cpp")

add_library(model STATIC ${src_list})
