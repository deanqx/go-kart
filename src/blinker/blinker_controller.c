#include "hal.h"
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdint.h>

uint8_t bc_current_stage = 0;

/// Have to enable global interrupts with sei()
/// Uses timer1 and resets timer value at interrupt
/// See project documentation for more details about calculation
void bc_init_timer_interrupt(void) {
  // TIMER1 clock: 16 MHz / 8 = 2 MHz
  TCCR1B = 1 << CS11;
  // reset when register A matches
  OCR1A = 48888;
}

void bc_enable_blinker(void) {
  // enable interrupt TIMER1_COMPA
  TIMSK1 = 1 << OCIE1A;
}

void bc_disable_blinker(void) {
  // disable interrupt TIMER1_COMPA
  TIMSK1 &= ~(1 << OCIE1A);

  // reset blinker
  bc_current_stage = 0;
  reset_all_blinker_leds();
}

// @returns true when blinker is enabled
bool bc_is_blinker_enabled(void) {
  // check if interrupt TIMER1_COMPA is active
  return TIMSK1 >> OCIE1A & 1;
}

ISR(TIMER1_COMPA_vect) {
  TCNT1 = 0;

  switch (bc_current_stage) {
  case 0:
    SET(LED_BLINKER_0);
    break;
  case 1:
    SET(LED_BLINKER_1);
    break;
  case 2:
    SET(LED_BLINKER_2);
    break;
  case 3:
    SET(LED_BLINKER_3);
    break;
  case 4:
    SET(LED_BLINKER_4);
    break;
  case 5:
    SET(LED_BLINKER_5);
    break;
  case 6:
    SET(LED_BLINKER_6);
    break;
  case 7:
    SET(LED_BLINKER_7);
    break;
  case 8:
    SET(LED_BLINKER_8);
    break;
  case 9:
    SET(LED_BLINKER_9);
    break;
  case 10:
    SET(LED_BLINKER_10);
    break;
  case 11:
    SET(LED_BLINKER_11);
    break;
  case 12:
    SET(LED_BLINKER_12);
    break;
  case 13:
    SET(LED_BLINKER_13);
    break;
  }

  if (bc_current_stage == 14 + (12 - 1)) {
    reset_all_blinker_leds();
  }

  bc_current_stage++;

  // 14 LEDs, 12 iterations with all enabled and 1 reset iteration
  if (bc_current_stage >= 14 + 12) {
    bc_current_stage = 0;
  }
}
