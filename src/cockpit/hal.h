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
#define HORN_PWM B, 4
/// LA = low active
#define SW_MOTOR_START_LA B, 5
#define SW_MOTOR_REVERSE_LA B, 7

// --- PORTC ---
#define SW_BLINKER_RIGHT C, 0
#define LED_BLINKER_RIGHT C, 3

// --- PORTD ---
#define UART1_RX D, 2
#define UART1_TX D, 3
#define LED_GEAR_UP D, 4

// --- PORTE ---
#define UART0_RX_DISPLAY E, 0
#define UART0_TX_DISPLAY E, 1
#define LED_BLINKER_LEFT E, 2
#define SW_BLINKER_LEFT E, 3
#define LED_LIGHT_SWITCH E, 4
#define SW_LIGHT_SWITCH E, 5
#define LED_WARNBLINKER_HORN E, 6
#define SW_WARNBLINKER_HORN E, 7

// --- PORTG ---
#define SW_GEAR_UP G, 0
#define SW_GEAR_DOWN G, 2
#define LED_GEAR_DOWN G, 3

#define _SET2(port, bit) PORT##port |= (1 << bit)
#define _GET2(port, bit) ((PIN##port >> bit) & 1)
#define _RESET2(port, bit) PORT##port &= ~(1 << bit)
#define _TOGGLE2(port, bit) PORT##port ^= (1 << bit)
#define _SET_INPUT2(port, bit) DDR##port |= (1 << bit)
#define _SET_OUTPUT2(port, bit) DDR##port |= (1 << bit)

// Beispiel: SET(LED_BLINKER_0);
// => _SET2(D, 5);
// => PORTD |= (1 << 5);

#define SET(port_comma_bit) _SET2(port_comma_bit)
#define GET(port_comma_bit) _GET2(port_comma_bit)
#define RESET(port_comma_bit) _RESET2(port_comma_bit)
#define TOGGLE(port_comma_bit) _TOGGLE2(port_comma_bit)
#define SET_INPUT(port_comma_bit) _SET_INPUT2(port_comma_bit)
#define SET_OUTPUT(port_comma_bit) _SET_OUTPUT2(port_comma_bit)

#define _P2(port, bit) (1 << bit)
#define _P(port_comma_bit) _P2(port_comma_bit)

void hal_init(void) {
  // enable pull up resistors
  SET(SW_MOTOR_START_LA);
  SET(SW_MOTOR_REVERSE_LA);

  // set output pins
  DDRB = _P(HORN_PWM);
  DDRC = _P(LED_BLINKER_RIGHT);
  DDRD = _P(UART1_TX) | _P(LED_GEAR_UP);
  DDRE = _P(UART0_TX_DISPLAY) | _P(LED_BLINKER_LEFT) | _P(LED_LIGHT_SWITCH) |
         _P(LED_WARNBLINKER_HORN);
  DDRG = _P(LED_GEAR_DOWN);
}

#endif
