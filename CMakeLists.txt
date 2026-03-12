cmake_minimum_required(VERSION 3.23 FATAL_ERROR)

set(CMAKE_OBJECT_PATH_MAX 250)

# zephyr external modules
list(APPEND EXTRA_ZEPHYR_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/hal/ext_drivers/custom_driver_module)

# zephyr package addition
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# variable from CMake
zephyr_get(BOARD_REVISION)

# toolchain configuration
# set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/hal/platforms/${BOARD_REVISION}/toolchain.cmake)

# project configuration
project("embedded_project_name" C CXX ASM) 

# global headers (FreeRTOSConfig.h)
include_directories(application)

# Application modules
add_subdirectory(application)
add_subdirectory(middlewares)
add_subdirectory(external)
add_subdirectory(hal)
