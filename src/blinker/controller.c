#include "can.h"
#include <avr/io.h>
#include <util/delay.h>

void main_loop(void) {
  DDRD = 1 << PD2;

  while (1) {
    PORTD ^= 1 << PD2;
    _delay_ms(500);
  }
}
