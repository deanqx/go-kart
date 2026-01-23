#include "hal.h"
#include "usart.h"
#include <avr/io.h>
#include <util/delay.h>

void main_loop(void) {
  hal_init();

  uart0_puts("JCS-BK_1091_GO-KART\r\n");
  uart0_puts("Lenkrad und Anzeige\r\n");
  uart0_puts("von Dean Schneider (GYT26)\r\n");

  while (1) {
    hal_io_toggle(PORT_TXD1, TXD1);

    hal_apply();
    _delay_ms(500);
  }
}
