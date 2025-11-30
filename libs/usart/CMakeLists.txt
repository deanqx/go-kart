add_library(usart
    src/usart.c
)

target_include_directories(usart
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
