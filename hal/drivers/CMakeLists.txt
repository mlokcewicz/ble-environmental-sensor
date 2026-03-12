# drivers target
add_library(drivers STATIC)

target_include_directories(drivers PUBLIC .)

target_sources(drivers PRIVATE dummy.c)