zephyr_library_named(custom_bme280) # instead of zephyr_library() to shorten library name and workaround CMake's object path length limit
zephyr_library_sources(custom_bme280.c)