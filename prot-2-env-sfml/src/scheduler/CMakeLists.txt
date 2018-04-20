file(GLOB_RECURSE src_list "*.cpp")

add_library(scheduler STATIC ${src_list})
