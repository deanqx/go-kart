/*
 * uart0 für Ausgabe über Bluetooth.
 * uart1 für Steuerung des Displays.
 */

#include "can.h"
#include "hal.h"
#include "usart.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#define COCKPIT_DISPLAY_IMPLEMENTATION
#include "cockpit_display.h"

static const uint32_t CAN_ID = 0x000;

void init(void) {
  uart0_init(BAUD_CALC(9600UL));
  uart1_init(BAUD_CALC(9600UL));
  hal_init();

  uart0_puts("\ninfo: JCS-BK_1091_GO-KART von Dean Schneider (GYT26)\n");
  uart0_puts("info: Blinker-Modul mit CAN-ID: 0x");
  uart0_puthex((uint8_t)(CAN_ID >> 8 * 3));
  uart0_puthex((uint8_t)(CAN_ID >> 8 * 2));
  uart0_puthex((uint8_t)(CAN_ID >> 8 * 1));
  uart0_puthex((uint8_t)(CAN_ID >> 8 * 0));
}

int main(void) {
  init();
  sei();

  while (1) {
  }
}
