set(CMAKE_SYSTEM_NAME Generic)

# set(CMAKE_C_COMPILER    ${TOOLCHAIN_PATH}arm-zephyr-eabi-gcc.exe)
# set(AS                  ${TOOLCHAIN_PATH}arm-zephyr-eabi-as.exe)
# set(AR                  ${TOOLCHAIN_PATH}arm-zephyr-eabi-ar.exe)
# set(OBJCOPY             ${TOOLCHAIN_PATH}arm-zephyr-eabi-objcopy.exe)
# set(OBJDUMP             ${TOOLCHAIN_PATH}arm-zephyr-eabi-objdump.exe)
# set(SIZE                ${TOOLCHAIN_PATH}arm-zephyr-eabi-size.exe)
# set(NM                  ${TOOLCHAIN_PATH}arm-zephyr-eabi-nm.exe)
 
set(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS      ON)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_INCLUDES     ON)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_LIBRARIES    ON)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS    ON)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES   ON)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_LIBRARIES  ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS        	   ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(WARNING_FLAGS 
    -Wall 
    -Werror 
    -Wcomment 
    -Wdouble-promotion 
    # -Wextra 
    -Wno-error=cpp 
    -Wno-unused-parameter 
    -Wformat-security 
    # -Wfloat-equal 
    -Wshadow 
    -Wpointer-arith 
    -Wno-unused-function)

set(CORE_FLAGS 
    -mthumb 
    -mcpu=cortex-m4 
    # -mfloat-abi=hard 
    # -mfpu=fpv4-sp-d16
    )

set(ASM_FLAGS 
    -x 
    assembler-with-cpp)

set(C_FLAGS 
    --specs=nano.specs
    -fdata-sections 
    -ffunction-sections 
    -fshort-enums 
    -fno-builtin
    ) 

set(CXX_FLAGS 
    --specs=nano.specs
    -fdata-sections 
    -ffunction-sections 
    -fshort-enums 
    -fno-exceptions 
    -fno-rtti)

set(LINKER_FLAGS 
    # --specs=nano.specs 
    # --specs=nosys.specs 
    -mthumb 
    -mcpu=cortex-m4 
    LINKER:--gc-sections 
    LINKER:-Map=map_file.map 
    # -T${CMAKE_CURRENT_LIST_DIR}/linker_script_nrf52832.ld
    ) 

set(DEBUG_FLAGS
    -O0
    -g 
    -ggdb3)

set(RELEASE_FLAGS 
    -Os)

add_compile_options(
    "$<$<COMPILE_LANGUAGE:ASM>:${CORE_FLAGS};${ASM_FLAGS}>"
    "$<$<COMPILE_LANGUAGE:C>:${CORE_FLAGS};${C_FLAGS};${WARNING_FLAGS}>"
    "$<$<COMPILE_LANGUAGE:CXX>:${CORE_FLAGS};${CXX_FLAGS};${WARNING_FLAGS}>"
    "$<$<CONFIG:DEBUG>:${DEBUG_FLAGS}>"
    "$<$<CONFIG:RELEASE>:${RELEASE_FLAGS}>")

add_link_options(
    ${CORE_FLAGS} 
    ${LINKER_FLAGS})
