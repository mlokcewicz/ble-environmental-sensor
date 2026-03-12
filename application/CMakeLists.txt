# main app target

target_sources(app PRIVATE main.c)

target_include_directories(app PRIVATE .)

target_link_libraries(app PRIVATE
    utils 
    platform
)
