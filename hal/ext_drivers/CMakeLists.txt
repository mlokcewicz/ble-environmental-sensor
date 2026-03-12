# ext_drivers target
add_library(ext_drivers STATIC)

target_include_directories(ext_drivers PUBLIC .)

target_sources(ext_drivers PRIVATE dummy.c)
