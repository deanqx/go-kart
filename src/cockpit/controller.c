#include "usart.h"
#include <avr/io.h>
#include <util/delay.h>

void main_loop(void) {
  // uart0_puts("JCS-BK_1091_GO-KART\r\n");
  // uart0_puts("Lenkrad und Anzeige\r\n");
  // uart0_puts("von Dean Schneider (GYT26)\r\n");
  // DDRD = 1 << PD3;
  uart1_init(BAUD_CALC(9600UL));

  while (1) {
    uart1_puts("bL.pic=1\xFF\xFF\xFF");
    _delay_ms(500);
  }
}
