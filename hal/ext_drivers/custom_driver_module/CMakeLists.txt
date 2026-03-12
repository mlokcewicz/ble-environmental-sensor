# Subdirectories
add_subdirectory(drivers)

zephyr_syscall_include_directories(include)

# Include headers
zephyr_include_directories(drivers)
zephyr_include_directories(include)