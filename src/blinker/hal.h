/*
 * HAL (Hardware Abstraction Layer) ist eine Programmierschicht,
 * die die Hardware eines Mikrocontrollers durch einfache,
 * einheitliche Funktionen abstrahiert.
 * Dadurch wird der Code leichter lesbar, wartbarer und besser portierbar.
 * Allerdings oft etwas langsamer und größer als reiner Low-Level-Code.
 *
 * Author: Dean Schneider (GYT26)
 */

#ifndef HAL_H
#define HAL_H

#include <avr/io.h>

// Beispiel: PORTC PC6 => C, 6

// --- PORTB ---
#define ISP_SCK B, 1
#define ISP_MOSI B, 2
#define ISP_MISO B, 3
#define LED_BLINKER_5 B, 6
#define LED_BLINKER_6 B, 5
#define LED_BLINKER_8 B, 4
#define LED_BLINKER_9 B, 3
#define LED_BLINKER_11 B, 0
#define LED_BLINKER_12 B, 1
#define LED_BLINKER_13 B, 7

// --- PORTC ---
#define LED_UPPER_LIGHT_PWM C, 1
#define LED_LOWER_LIGHT C, 4
#define LED_BLINKER_3 C, 5
#define LED_BLINKER_4 C, 0
#define LED_BLINKER_7 C, 7
#define LED_BLINKER_10 C, 6

// --- PORTD ---
#define LED_BLINKER_0 D, 5
#define LED_BLINKER_1 D, 6
#define LED_BLINKER_2 D, 7
/// low active
#define INTERNAL_LED_LA D, 2

#define _SET2(port, bit) PORT##port |= (1 << bit)
#define _RESET2(port, bit) PORT##port &= ~(1 << bit)
#define _TOGGLE2(port, bit) PORT##port ^= (1 << bit)
#define _SET_INPUT2(port, bit) DDR##port |= (1 << bit)
#define _SET_OUTPUT2(port, bit) DDR##port |= (1 << bit)

#define SET(port_comma_bit) _SET2(port_comma_bit)
#define RESET(port_comma_bit) _RESET2(port_comma_bit)
#define TOGGLE(port_comma_bit) _TOGGLE2(port_comma_bit)
#define SET_INPUT(port_comma_bit) _SET_INPUT2(port_comma_bit)
#define SET_OUTPUT(port_comma_bit) _SET_OUTPUT2(port_comma_bit)

#define _P2(port, bit) (1 << bit)
#define _P(port_comma_bit) _P2(port_comma_bit)

void hal_init(void) {
  DDRB = _P(LED_BLINKER_5) | _P(LED_BLINKER_6) | _P(LED_BLINKER_8) |
         _P(LED_BLINKER_9) | _P(LED_BLINKER_11) | _P(LED_BLINKER_12) |
         _P(LED_BLINKER_13);

  DDRC = _P(LED_UPPER_LIGHT_PWM) | _P(LED_LOWER_LIGHT) | _P(LED_BLINKER_3) |
         _P(LED_BLINKER_4) | _P(LED_BLINKER_7) | _P(LED_BLINKER_10);

  DDRD = _P(INTERNAL_LED_LA) | _P(LED_BLINKER_0) | _P(LED_BLINKER_1) |
         _P(LED_BLINKER_2);
}

void reset_blinker_leds(void) {
  PORTB &= ~(_P(LED_BLINKER_5) | _P(LED_BLINKER_6) | _P(LED_BLINKER_8) |
             _P(LED_BLINKER_9) | _P(LED_BLINKER_11) | _P(LED_BLINKER_12) |
             _P(LED_BLINKER_13));

  PORTC &= ~(_P(LED_BLINKER_3) | _P(LED_BLINKER_4) | _P(LED_BLINKER_7) |
             _P(LED_BLINKER_10));

  PORTD &= ~(_P(LED_BLINKER_0) | _P(LED_BLINKER_1) | _P(LED_BLINKER_2));
}

#endif
