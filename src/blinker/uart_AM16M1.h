/*
        RS232 Bibliothek von MS 24.05.10
        für die ATMegareihe.
*/

#if !defined(UART_16M1_H) && defined(__AVR_ATmega16M1__)
#define UART_16M_H

#define BAUD 9600UL

// Formeln vom Datenblatt
// BTR entspricht skalierung
#define UART_BTR (uint8_t)32
#define UART_BRR (uint16_t)(F_CPU / ((uint16_t)(UART_BTR) * BAUD) - 1UL)

#include <avr/io.h>
#include <stdbool.h>

void uart_init(void) {
  // reset UART
  LINCR = 1 << LSWRES;
  // Timing Register (scale)
  LINBTR = UART_BTR;
  // Baud Rate Register
  LINBRR = UART_BRR;
  // UART no parity, enable UART, UART TX and RX
  LINCR = 1 << LENA | 0x7;
}

/// Received char over UART?
bool uart_has_received(void) { return LINSIR >> LRXOK & 1; }

/// Read char from UART.
/// @returns 0 if no char was sent
uint8_t uart_getc(void) {
  if (!uart_has_received()) {
    return 0;
  }
  return LINDAT;
}

/// Send char
void uart_putc(char c) {
  while (LINSIR & (1 << LBUSY))
    ;
  LINDAT = c;
}

/// Send null terminated string
void uart_puts(char *s) {
  while (*s) {
    uart_putc(*s);
    s++;
  }
}

void uart_puthex(uint8_t data) {
  const uint8_t first_digit = data >> 4;
  uart_putc(first_digit <= 9 ? '0' + first_digit : 'A' + (first_digit - 10));

  const uint8_t second_digit = data & 0x0f;
  uart_putc(second_digit <= 9 ? '0' + second_digit : 'A' + (second_digit - 10));
}

#endif
