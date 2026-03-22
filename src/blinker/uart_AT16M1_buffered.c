/*
 * UART Bibliothek mit Buffer für ATmega16M1.
 * Author: Dean Schneider (GYT26)
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>
#include <util/delay.h>

#if !defined(UART_16M1_H) && defined(__AVR_ATmega16M1__)
#define UART_16M_H

#define BAUD 9600UL
// maximum: 8 bit = 255
#define UART_BUFFER_SIZE 70

// Formeln vom Datenblatt
// BTR entspricht skalierung
#define UART_BTR 32
#define UART_BRR (uint16_t)(F_CPU / ((uint16_t)(UART_BTR) * BAUD) - 1UL)

volatile static char uart_buf[UART_BUFFER_SIZE];
volatile static uint16_t uart_tx_queue_len = 0;
volatile static uint16_t uart_buf_current = 0;

/// only allowed to be called when no transmission is in progress
static void _uart_start_tx(void) { LINDAT = uart_buf[uart_buf_current]; }

static void _uart_buffer(char c) {
  // wait until there is space in buffer
  while (1) {
    cli();
    if (uart_tx_queue_len != UART_BUFFER_SIZE) {
      sei();
      break;
    }
    sei();

    if (~LINSIR & (1 << LBUSY)) { // if not currently sending
      sei();
      _uart_start_tx();
    }
  }

  cli();
  const uint8_t uart_buf_end =
      (uart_buf_current + uart_tx_queue_len) % UART_BUFFER_SIZE;
  uart_buf[uart_buf_end] = c;
  uart_tx_queue_len++;
  sei();
}

// interrupt for transfer complete
ISR(LIN_TC_vect) {
  uart_buf_current = (uart_buf_current + 1) % UART_BUFFER_SIZE;
  uart_tx_queue_len--;

  if (uart_tx_queue_len > 0) {
    _uart_start_tx();
  } else {
    // transmission complete, clears LBUSY flag
    LINSIR |= 1 << LTXOK;
  }
}

void uart_init(void) {
  // reset UART
  LINCR = 1 << LSWRES;
  // enable Transmit Performed Interrupt
  LINENIR = 1 << LENTXOK;
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

/// start clearing queued messages
void _uart_flush(void) {
  if (~LINSIR & (1 << LBUSY)) { // if not currently sending
    _uart_start_tx();
  }
}

/// Send char
void uart_putc(char c) {
  _uart_buffer(c);
  _uart_flush();
}

/// Send null terminated string
void uart_puts(char *s) {
  while (*s) {
    _uart_buffer(*s);
    s++;
  }

  _uart_flush();
}

void uart_puthex(uint8_t data) {
  const uint8_t first_digit = data >> 4;
  const uint8_t second_digit = data & 0x0f;
  _uart_buffer(first_digit <= 9 ? '0' + first_digit : 'A' + (first_digit - 10));
  _uart_buffer(second_digit <= 9 ? '0' + second_digit
                                 : 'A' + (second_digit - 10));
  _uart_flush();
}

#endif
