cmake_minimum_required(VERSION 4.1)
project(go-kart C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Generic: no OS
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER avr-gcc)

set(F_CPU 16000000UL)          
set(PROGRAMMER usbasp)

option(COMPILE_BLINKER "Compile for blinker microcontroller (can only compile for one at a time)" OFF)
option(COMPILE_COCKPIT "Compile for cockpit microcontroller (can only compile for one at a time)" OFF)

if(COMPILE_BLINKER AND COMPILE_COCKPIT)
    message(FATAL_ERROR "Can only compile one at a time. Choose COMPILE_BLINKER or COMPILE_COCKPIT.")
elseif(COMPILE_BLINKER)
    add_compile_definitions(COMPILE_BLINKER)
    set(MCU atmega16m1)
elseif(COMPILE_COCKPIT)
    add_compile_definitions(COMPILE_COCKPIT)
    set(MCU at90can32)
else()
    message(FATAL_ERROR "You have to define a compile mode. Usage: -DCOMPILE_BLINKER=ON or -DCOMPILE_COCKPIT=ON")
endif()

# Compiler flags
add_definitions(-DF_CPU=${F_CPU})
set(CMAKE_C_FLAGS "-mmcu=${MCU} -Os -fshort-enums")
set(CMAKE_EXE_LINKER_FLAGS "-mmcu=${MCU}")

add_executable(${PROJECT_NAME}.elf
    src/main.c
)

add_compile_definitions(SUPPORT_FOR_AT90CAN__)
add_subdirectory(lib/can)
target_link_libraries(${PROJECT_NAME}.elf PRIVATE can)

if(COMPILE_COCKPIT)
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
