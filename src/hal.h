/*
 * HAL (Hardware Abstraction Layer) ist eine Programmierschicht,
 * die die Hardware eines Mikrocontrollers durch einfache,
 * einheitliche Funktionen abstrahiert.
 * Dadurch wird der Code leichter lesbar, wartbarer und besser portierbar.
 * Allerdings oft etwas langsamer und größer als reiner Low-Level-Code.
 *
 * Author: Dean Schneider
 */

#ifndef HAL_H
#define HAL_H

#include <avr/io.h>

// Port buffers
unsigned char portd = 0;

#define PORT_TXD1 portd

#define DDR_TXD1 DDRD

#define TXD1 PD3

// Diese Makros werden erst auf den Port durch hal_apply() angewendet.
#define hal_io_set(port, pin, state) port = port & ~(1 << pin) | (state << pin)
#define hal_io_get(port, pin) port = port >> pin & 1
#define hal_io_toggle(port, pin) port ^= port << pin

static const unsigned char OUTPUT = 1;
static const unsigned char INPUT = 0;

// Initialisiert Ausgänge
static inline void hal_init(void) { hal_io_set(DDR_TXD1, TXD1, OUTPUT); }

// Übernimmt gesetzte Ports
static inline void hal_apply(void) { PORTD = portd; }

#endif
