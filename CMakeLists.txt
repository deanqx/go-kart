cmake_minimum_required(VERSION 4.1)
project(go-kart C)

set(BUILD_TARGET CACHE STRING "Choose target for compilation. Usage: -DBUILD_TARGET=blinker or -DBUILD_TARGET=cockpit")
set_property(CACHE BUILD_TARGET PROPERTY STRINGS blinker cockpit)

message(STATUS "Build target: ${BUILD_TARGET}")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_UNITY_BUILD OFF)
# Generic: no OS
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER avr-gcc)

set(F_CPU 16000000UL)          
set(PROGRAMMER usbasp)

if(BUILD_TARGET STREQUAL "blinker")
    set(MCU atmega16m1)
    add_executable(${PROJECT_NAME}.elf
        src/blinker/main.c
    )
elseif(BUILD_TARGET STREQUAL "cockpit")
    set(MCU at90can32)
    add_executable(${PROJECT_NAME}.elf
        src/cockpit/main.c
    )
else()
    message(FATAL_ERROR "Invalid value BUILD_TARGET='${COMPILE_FOR}', Usage: -DBUILD_TARGET=blinker or -DBUILD_TARGET=cockpit")
endif()

# Compiler flags
add_definitions(-DF_CPU=${F_CPU})
set(CMAKE_C_FLAGS "-mmcu=${MCU} -Os -Wall -fshort-enums")
set(CMAKE_EXE_LINKER_FLAGS "-mmcu=${MCU} -Wall")

add_subdirectory(lib/avr-can-lib)
target_compile_definitions(can PUBLIC
    CAN_CONFIG_LOADED
    # Select if you want to use 29 bit identifiers.
    SUPPORT_EXTENDED_CANID=1
    # Select if you want to use timestamps.
    # Timestamps are sourced from a register internal to the AT90CAN.
    # Selecting them on any other controller will have no effect, they will
    # be 0 all the time.
    SUPPORT_TIMESTAMPS=0
    SUPPORT_MCP2515=0
    SUPPORT_AT90CAN=1
    SUPPORT_SJA1000=0
    MCP2515_CS="B,2"
    MCP2515_INT="B,4"
    CAN_RX_BUFFER_SIZE=16
    CAN_TX_BUFFER_SIZE=8
    # only available if CAN_TX_BUFFER_SIZE > 0
    CAN_FORCE_TX_ORDER=1
)
target_link_libraries(${PROJECT_NAME}.elf PRIVATE can)

if(MCU STREQUAL "at90can32")
    # usart library is not compatible with ATmega16M1
    add_subdirectory(lib/usart)
    target_link_libraries(${PROJECT_NAME}.elf PRIVATE usart)
endif()

# convert ELF to Intel HEX
add_custom_command(
    OUTPUT ${PROJECT_NAME}.hex
    DEPENDS ${PROJECT_NAME}.elf
    COMMAND avr-objcopy -O ihex -R .eeprom ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
)

add_custom_target(hex
    DEPENDS ${PROJECT_NAME}.hex
)

add_custom_target(upload
    ALL
    DEPENDS hex
    COMMAND avrdude -p ${MCU} -c ${PROGRAMMER} -U flash:w:${PROJECT_NAME}.hex
)
