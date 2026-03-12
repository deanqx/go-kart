/*
 * Bibliothek um Blinker zu steuern.
 * Verwendet timer0 und timer1
 *
 * Author: Dean Schneider (GYT26)
 */

#include "hal.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

static uint8_t bc_current_stage = 0;
static uint8_t bc_interrupt_count = 0;

/// Have to enable global interrupts with sei()
/// See project documentation for more details about calculation
void bc_init_timer0_timer1(void) {
  // --- Timer0 ---
  // clear timer on compare match with OCRA
  TCCR0A = 1 << WGM01;
  // F_CPU / 1024 = 16 MHz / 1024 = 15625 Hz
  TCCR0B = 1 << CS02 | 1 << CS00;
  OCR0A = 84; // Timer0 at 186 Hz

  // --- Timer1 ---
  // Clear OC1B on compare match, set OC1B at 1023 (large OCR1B value == bright)
  // clear timer on compare match with 1023
  TCCR1A = 1 << COM1B1 | 1 << WGM11 | 1 << WGM10;
  // Timer1 clock: no prescaling
  TCCR1B = 1 << WGM12 | 1 << CS10;
}

/// Fern-/Bremsenlicht anschalten und Abblend-/Rücklicht ausschalten
void bc_upper_full_light(void) {
  OCR1B = 1023; // 100 % brightness
}

/// Abblend-/Rücklicht anschalten und Fern-/Bremsenlicht ausschalten
void bc_upper_weak_light(void) {
  OCR1B = 251; // 25 % brightness
}

/// Fern-/Bremsenlicht ausschalten und Abblend-/Rücklicht ausschalten
void bc_upper_light_off(void) {
  OCR1B = 0; // 0 % brightness
}

void bc_blinker_on(void) {
  // enable interrupt TIMER1_COMPA
  TIMSK0 |= 1 << OCIE0A;
}

void bc_blinker_off(void) {
  // disable interrupt TIMER1_COMPA
  TIMSK0 &= ~(1 << OCIE0A);

  // reset blinker
  bc_current_stage = 0;
  bc_interrupt_count = 0;
  reset_blinker_leds();
}

// @returns true when blinker is enabled
bool bc_is_blinker_enabled(void) {
  // check if interrupt TIMER1_COMPA is active
  return TIMSK0 >> OCIE0A & 1;
}

ISR(TIMER0_COMPA_vect) {
  // timer frequency / SOFT_PRESCALER = frequency
  static const uint8_t SOFT_PRESCALER = 8; // about 23.25 Hz

  bc_interrupt_count++;

  if (bc_interrupt_count != SOFT_PRESCALER) {
    return;
  }

  bc_interrupt_count = 0;

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

  if (bc_current_stage == 14 + (7 - 1)) {
    reset_blinker_leds();
    RESET(LED_BLINKER_0);
  }

  bc_current_stage++;

  // 14 LEDs, 7 iterations with all enabled
  if (bc_current_stage >= 14 + 7) {
    bc_current_stage = 0;
  }
}
