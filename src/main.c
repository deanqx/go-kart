#include <avr/io.h>
#include <util/delay.h>

static const uint8_t TXD1 = PD3;

int main(void) {
  DDRD = 1 << TXD1;

  while (1) {
    PORTD ^= 1 << TXD1;
    _delay_ms(500);
  }

  return 0;
}
