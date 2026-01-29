/*
 * Pinbelegung ist in hal.h zu finden.
 */

#include "can.h"
#include "hal.h"
#include <avr/io.h>
#include <util/delay.h>

int main(void) {
  SET_OUTPUT(INTERNAL_LED_LA);
  RESET(INTERNAL_LED_LA); // light up internal LED

  SET_OUTPUT(LED_BREAK);
  SET_OUTPUT(LED_BACKLIGHT);
  SET_OUTPUT(LED_BLINKER_0);
  SET_OUTPUT(LED_BLINKER_1);
  SET_OUTPUT(LED_BLINKER_2);
  SET_OUTPUT(LED_BLINKER_3);
  SET_OUTPUT(LED_BLINKER_4);
  SET_OUTPUT(LED_BLINKER_5);
  SET_OUTPUT(LED_BLINKER_6);
  SET_OUTPUT(LED_BLINKER_7);
  SET_OUTPUT(LED_BLINKER_8);
  SET_OUTPUT(LED_BLINKER_9);
  SET_OUTPUT(LED_BLINKER_10);
  SET_OUTPUT(LED_BLINKER_11);
  SET_OUTPUT(LED_BLINKER_12);
  SET_OUTPUT(LED_BLINKER_13);

  while (0) {
    _delay_ms(500);
    SET(LED_BREAK);
    _delay_ms(500);
    SET(LED_BLINKER_0);
    _delay_ms(500);
    SET(LED_BLINKER_1);
    _delay_ms(500);
    SET(LED_BLINKER_2);
    _delay_ms(500);
    SET(LED_BLINKER_3);
    _delay_ms(500);
    SET(LED_BLINKER_4);
    _delay_ms(500);
    SET(LED_BLINKER_5);
    _delay_ms(500);
    SET(LED_BLINKER_6);
    _delay_ms(500);
    SET(LED_BLINKER_7);
    _delay_ms(500);
    SET(LED_BLINKER_8);
    _delay_ms(500);
    SET(LED_BLINKER_9);
    _delay_ms(500);
    SET(LED_BLINKER_10);
    _delay_ms(500);
    SET(LED_BLINKER_11);
    _delay_ms(500);
    SET(LED_BLINKER_12);
    _delay_ms(500);
    SET(LED_BLINKER_13);
    _delay_ms(500);
    SET(LED_BACKLIGHT);
    _delay_ms(500);

    PORTB = 0;
    PORTC = 0;
    PORTD = 0;
  }

  // TODO reduce mob count

  // Initialize MCP2515 with 250 kB/s because of 8 MHz crystal
  // actual bus speed is 125 kB/s
  // CS pin is configured in lib/can/include/config.h
  if (!can_init(BITRATE_250_KBPS)) {
    while (1) {
      TOGGLE(INTERNAL_LED_LA);
      _delay_ms(500);
    }
  }

  can_t hello_msg = {
      .id = 0x0000,
      .length = 8,
      .data = {'J', 'C', 'S', '1', '0', '9', '1'},
  };

  if (!can_send_message(&hello_msg)) {
    while (1) {
      TOGGLE(INTERNAL_LED_LA);
      _delay_ms(500);
    }
  }

  while (1) {
    if (!can_check_message()) {
      continue;
    }

    can_t received_command;

    if (!can_get_message(&received_command)) {
      // TODO send error
      continue;
    }

    if (received_command.id != 0x2000) {
      continue;
    }

    TOGGLE(INTERNAL_LED_LA);
  }
}
