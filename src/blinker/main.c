/*
 * Pinbelegung ist in hal.h zu finden.
 *
 * TODO write ZWL and timers
 */

#define BAUD 9600UL

#include "blinker_controller.c"
#include "can.h"
#include "hal.h"
#include "uart_AM16M1.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

static const uint32_t CAN_ID = 0x00001234;

#define BREAK_BIT 0
#define BLINKER_BIT 1
#define BACKLIGHT_BIT 2

int main(void) {
  hal_init();
  usart_init();
  bc_init_timer_interrupt();

  usart_puts("JCS-BK_1091_GO-KART\r\n");
  usart_puts("von Dean Schneider (GYT26)\r\n");
  usart_puts("Blinker-Modul mit CAN-ID: 0x");
  usart_puthex((uint8_t) (CAN_ID >> 8 * 3));
  usart_puthex((uint8_t) (CAN_ID >> 8 * 2));
  usart_puthex((uint8_t) (CAN_ID >> 8 * 1));
  usart_puthex((uint8_t) (CAN_ID >> 8 * 0));
  usart_puts("\r\n");

  if (!can_init(BITRATE_125_KBPS)) {
    usart_puts("fatal error: while can initialization\r\n");

    while (true) {
      TOGGLE(INTERNAL_LED_LA);
      _delay_ms(200);
    }
  }

  can_filter_t can_filter = {
      .id = CAN_ID,
      .mask = 0x1FFFFFFF,    // only accept exactly this ID
      .flags.extended = 0x3, // filter with extended id
      .flags.rtr = 0x2,      // receive both RTR and normal
  };

  if (!can_set_filter(0, &can_filter)) {
    usart_puts("fatal error: while setting can message filters\r\n");

    while (true) {
      TOGGLE(INTERNAL_LED_LA);
      _delay_ms(200);
    }
  }

  sei();

  can_t received_can_message;
  char command = usart_getc_free();

  while (1) {
    // --- Einlesen ---
    // read command from UART if one was send
    command = usart_getc_free();

    if (command == 0) { // no uart command
      if (!can_get_message(&received_can_message)) {
        continue;
      }

      if (received_can_message.flags.rtr == true) {
        const can_t healthy_message = {
          .id = 0x200,
          .flags.rtr = false,
          .flags.extended = true,
          .length = 0,
        };

        can_send_message(&healthy_message);
        continue;
      }

      if (received_can_message.length != 1) {
        usart_puts("error: received command but DLC does not equal 1\r\n");
        continue;
      }

      command = received_can_message.data[0];
    }

    // --- Verarbeiten und Ausgaben ---
    if (command & (1 << BREAK_BIT)) {
      SET(LED_BREAK);
    } else {
      RESET(LED_BREAK);
    }

    if (command & (1 << BLINKER_BIT)) {
      bc_enable_blinker();
    } else if (bc_is_blinker_enabled()) {
      bc_disable_blinker();
    }

    if (command & (1 << BACKLIGHT_BIT)) {
      SET(LED_BACKLIGHT);
    } else {
      RESET(LED_BACKLIGHT);
    }
  }
}
