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

// --- PORTB ---
#define IN_BLINKER_LEFT B, 6
#define IN_BLINKER_RIGHT B, 6
#define IN_LIGHT_SWITCH B, 6
#define IN_WARNBLINKER B, 6
#define IN_GEAR_UP B, 6
#define IN_GEAR_DOWN B, 6

// --- PORTC ---
#define LED_BLINKER_LEFT B, 6
#define LED_BLINKER_RIGHT B, 6
#define LED_LIGHT_SWITCH B, 6
#define LED_WARNBLINKER B, 6
#define LED_GEAR_UP B, 6
#define LED_GEAR_DOWN B, 6

#define HORN_PWM B, 6
#define IN_MOTOR_START B, 6
#define IN_MOTOR_REVERSE B, 6

#define UART0_TX B, 6
#define UART0_RX B, 6
#define UART1_TX_DISPAY B, 6
#define UART1_RX_DISPAY B, 6

// --- PORTD ---

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

void hal_init(void) {}

#endif
