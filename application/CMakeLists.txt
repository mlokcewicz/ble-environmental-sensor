# main app target

target_sources(app PRIVATE main.c)
target_sources(app PRIVATE ui_manager.c)
target_sources(app PRIVATE env_manager.c)
target_sources(app PRIVATE ble_manager.c)
target_sources(app PRIVATE services/lb_service.c)
target_sources(app PRIVATE services/thp_service.c)

target_include_directories(app PRIVATE .)
target_include_directories(app PRIVATE services)

target_link_libraries(app PRIVATE
    utils 
    platform
)
