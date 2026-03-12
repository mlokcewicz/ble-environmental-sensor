cmake_minimum_required(VERSION 3.23 FATAL_ERROR)

# zephyr external modules
list(APPEND EXTRA_ZEPHYR_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/hal/ext_drivers/custom_driver_module)

# zephyr package addition
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# variable from CMake
zephyr_get(BOARD_REVISION)

# project configuration
project("ble_environmental_sensor" C CXX ASM) 

# global headers (FreeRTOSConfig.h)
include_directories(application)

# Application modules
add_subdirectory(application)
add_subdirectory(middlewares)
add_subdirectory(external)
add_subdirectory(hal)
