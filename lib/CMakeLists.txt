#add_library(usart
#    usart/src/usart.c
#)
#
#target_include_directories(usart
#    PUBLIC
#    usart/include
#)

add_library(can
    can/src/at90can.c
    can/src/at90can_buffer.c
    can/src/at90can_disable_dyn_filter.c
    can/src/at90can_error_register.c
    can/src/at90can_get_buf_message.c
    can/src/at90can_get_dyn_filter.c
    can/src/at90can_get_message.c
    can/src/at90can_private.h
    can/src/at90can_send_buf_message.c
    can/src/at90can_send_message.c
    can/src/at90can_set_dyn_filter.c
    can/src/at90can_set_mode.c
    can/src/can_buffer.c
    can/src/mcp2515.c
    can/src/mcp2515_buffer.c
    can/src/mcp2515_defs.h
    can/src/mcp2515_error_register.c
    can/src/mcp2515_get_dyn_filter.c
    can/src/mcp2515_get_message.c
    can/src/mcp2515_private.h
    can/src/mcp2515_read_id.c
    can/src/mcp2515_regdump.c
    can/src/mcp2515_send_message.c
    can/src/mcp2515_set_dyn_filter.c
    can/src/mcp2515_set_mode.c
    can/src/mcp2515_sleep.c
    can/src/mcp2515_static_filter.c
    can/src/mcp2515_write_id.c
    can/src/sja1000.c
    can/src/sja1000_buffer.c
    can/src/sja1000_error_register.c
    can/src/sja1000_get_message.c
    can/src/sja1000_send_message.c
    can/src/sja1000_set_mode.c
    can/src/spi.c
)

target_include_directories(can
    PUBLIC
    can/include
)
