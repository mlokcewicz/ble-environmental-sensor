# startup target
add_library(startup STATIC)

target_include_directories(startup PUBLIC .)

aux_source_directory(. STARTUP_SRC)
target_sources(startup PUBLIC ${STARTUP_SRC})
