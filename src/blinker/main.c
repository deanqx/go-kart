/*
 * Pinbelegung ist in hal.h zu finden.
 *
 * TODO write ZWL and timers
 */

#include "blinker_controller.c"
#include "can.h"
#include "hal.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

int main(void) {
  hal_init();
  bc_init_timer_interrupt();

  sei();

  SET(LED_BREAK);
  _delay_ms(500);
  RESET(LED_BREAK);
  _delay_ms(500);

  SET(LED_BACKLIGHT);
  _delay_ms(500);
  RESET(LED_BACKLIGHT);
  _delay_ms(500);

  while (0) {
    bc_enable_blinker();
    _delay_ms(5000);
    bc_disable_blinker();
    _delay_ms(2000);
  }

  if (!can_init(BITRATE_125_KBPS)) {
    // uart0_puts("error: while can initialization\r\n");

    while (true) {
      TOGGLE(INTERNAL_LED_LA);
      _delay_ms(200);
    }
  }

  can_filter_t can_filter = {
      .id = 0x00001234,
      .mask = 0x1FFFFFFF,
      .flags.extended = 0x3, // filter with extended id
      .flags.rtr = 0x2,      // receive both RTR and normal
  };

  if (!can_set_filter(0, &can_filter)) {
    // uart0_puts("error: while setting can message filters\r\n");

    while (true) {
      TOGGLE(INTERNAL_LED_LA);
      _delay_ms(200);
    }
  }

  const can_t msg = {
      .id = 0x12340000,
      .flags.extended = true,
      .flags.rtr = false,
      .length = 1,
      .data = {0xAA},
  };

  sei();
  while (1) {
    if (can_send_message(&msg) == 0) {
      // uart0_puts("error: can_send_message\r\n");
      _delay_ms(1000);
      continue;
    }

    can_t received_command;

    if (can_get_message(&received_command)) {
      received_command.id = 0x12340000;
      received_command.flags.extended = true;
      received_command.flags.rtr = false;
      can_send_message(&received_command); // echo for testing
    }

    _delay_ms(1000);
  }
}
