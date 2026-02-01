/*
 * Pinbelegung ist in hal.h zu finden.
 */

#include "can.h"
#include "hal.h"
#include <avr/interrupt.h>
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
      //uart0_puts("error: can_send_message\r\n");
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
