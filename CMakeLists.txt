cmake_minimum_required(VERSION 4.1)
project(go-kart C)

set(BUILD_TARGET CACHE STRING "Choose target for compilation. Usage: -DBUILD_TARGET=blinker or -DBUILD_TARGET=cockpit")
set_property(CACHE BUILD_TARGET PROPERTY STRINGS blinker cockpit)

message(STATUS "Build target: ${BUILD_TARGET}")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
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
set(CMAKE_EXE_LINKER_FLAGS "-mmcu=${MCU}")

add_compile_definitions(SUPPORT_FOR_AT90CAN__)
add_subdirectory(lib/can)
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
