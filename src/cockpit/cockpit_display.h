/*
 * # Beschreibung
 *
 * Mit dieser Bibliothek kann das Tacho Display gesteuert werden.
 * Die Nachrichten an das Display werden Über UART gesendet.
 * Orientiert an dem Projekt: JCS-BK_1046
 *
 * # Einbinden ins eigene Programm
 *
 * Diese Bibliothek verwendet den STB-Library-Stil.
 *
 * ```
 * #define COCKPIT_DISPLAY_IMPLEMENTATION
 * #include "cockpit_display.h"
 * ```
 *
 * Author: Dean Schneider (GYT26)
 */

#ifndef COCKPIT_DISPLAY_H
#define COCKPIT_DISPLAY_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  GEAR_NEUTRAL,
  GEAR_FIRST,
  GEAR_SECOND,
  GEAR_THIRD,
} GearState;

typedef enum {
  LIGHT_PARKING,
  LIGHT_LOW_BEAM,
  LIGHT_HIGH_BEAM,
} LightState;

typedef enum {
  BATTERY_EMPTY,
  BATTERY_ONE_FOURTH,
  BATTERY_TWO_FOURTH,
  BATTERY_THREE_FOURTH,
  BATTERY_FOUR_FOURTH,
  BATTERY_CHARGING,
} BatteryState;

// Not implemented for now
// void cd_set_distance(uint8_t x);

void cd_set_blinker_left(bool enable);
void cd_set_blinker_right(bool enable);
void cd_set_warnblinker(bool enable);
void cd_set_light(LightState light_state);
void cd_set_rpm(uint8_t rpm);
/// @param speed_kmh range in [0; 99]
void cd_set_speed(uint8_t speed_kmh);
void cd_set_gear(GearState gear_state);
void cd_set_battery(BatteryState battery_state);
/// @param value range in [0; 100]
void cd_set_throttle(uint8_t value);
/// @param value range in [0; 100]
void cd_set_brake(uint8_t value);

#ifdef COCKPIT_DISPLAY_IMPLEMENTATION

#include "usart.h"

void _cd_terminate(void) {
  uart_putc(0xFF);
  uart_putc(0xFF);
  uart_putc(0xFF);
}

void cd_set_blinker_left(const bool enable) {
  uart_puts("bL.pic=");

  if (enable) {
    uart_putc('1');
  } else {
    uart_putc('0');
  }

  _cd_terminate();
}

void cd_set_blinker_right(const bool enable) {
  uart_puts("bR.pic=");

  if (enable) {
    uart_putc('3');
  } else {
    uart_putc('2');
  }

  _cd_terminate();
}

void cd_set_warnblinker(const bool enable) {
  uart_puts("warnB.pic=1");

  if (enable) {
    uart_putc('6');
  } else {
    uart_putc('5');
  }

  _cd_terminate();
}

void cd_set_light(const LightState light_state) {
  uart_puts("warnB.pic=1");

  switch (light_state) {
  case LIGHT_LOW_BEAM:
    uart_putc('7');
    break;
  case LIGHT_HIGH_BEAM:
    uart_putc('8');
    break;
  case LIGHT_PARKING:
    uart_putc('9');
    break;
  }

  _cd_terminate();
}

/// @param digit range: [0; 9]
void _cd_set_custom_digit(const uint8_t digit) {
  const uint8_t digit_shifted = digit + 4;

  if (digit >= 10) { // values in the range: [10; 13]
    const uint8_t digit_shifted_ones = digit - 10;
    uart_putc('1');
    uart_putc(digit_shifted_ones + '0');
  } else {
    uart_putc(digit_shifted + '0');
  }
}

void cd_set_rpm(uint8_t rpm) {
  const uint8_t rpm_hundreds = rpm % 100;
  rpm -= rpm_hundreds * 100;
  const uint8_t rpm_tens = rpm % 10;
  rpm -= rpm_tens * 10;
  const uint8_t rpm_ones = rpm;

  uart_puts("rpm1.pic=");
  _cd_set_custom_digit(rpm_hundreds);
  uart_puts(" rpm2.pic=");
  _cd_set_custom_digit(rpm_tens);
  uart_puts(" rpm3.pic=");
  _cd_set_custom_digit(rpm_ones);
  _cd_terminate();
}

void cd_set_speed(uint8_t speed_kmh) {
  const uint8_t speed_tens = speed_kmh % 10;
  speed_kmh -= speed_tens * 10;
  const uint8_t speed_ones = speed_kmh;

  uart_puts("speed1.pic=");
  _cd_set_custom_digit(speed_tens);
  uart_puts(" speed2.pic=");
  _cd_set_custom_digit(speed_ones);
  _cd_terminate();
}

void cd_set_gear(const GearState gear_state) {
  uart_puts("gear.pic=");

  switch (gear_state) {
  case GEAR_NEUTRAL:
    uart_putc('1');
    uart_putc('4');
    break;
  case GEAR_FIRST:
    uart_putc('5');
    break;
  case GEAR_SECOND:
    uart_putc('6');
    break;
  case GEAR_THIRD:
    uart_putc('7');
    break;
  }

  _cd_terminate();
}

void cd_set_battery(const BatteryState battery_state) {
  uart_puts("battery.pic=2");

  switch (battery_state) {
  case BATTERY_EMPTY:
    uart_putc('0');
    break;
  case BATTERY_ONE_FOURTH:
    uart_putc('1');
    break;
  case BATTERY_TWO_FOURTH:
    uart_putc('2');
    break;
  case BATTERY_THREE_FOURTH:
    uart_putc('3');
    break;
  case BATTERY_FOUR_FOURTH:
    uart_putc('4');
    break;
  case BATTERY_CHARGING:
    uart_putc('5');
    break;
  }

  _cd_terminate();
}

void cd_set_throttle(const uint8_t value) {
  uart_puts("throttle.val=");
  uart_putuint(value);
  _cd_terminate();
}

void cd_set_brake(const uint8_t value) {
  uart_puts("brake.val=");
  uart_putuint(value);
  _cd_terminate();
}

#endif
#endif
