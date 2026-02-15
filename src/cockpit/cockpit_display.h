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

#include "usart.h"
#include <stdbool.h>
#include <stdint.h>

// these macros are used to choose between usart0 or usart1
#define CD_UART_PUTC uart0_putc
#define CD_UART_PUTS uart0_puts

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
void cd_set_rpm(uint16_t rpm);
/// @param speed_kmh range in [0; 99]
void cd_set_speed(uint8_t speed_kmh);
void cd_set_gear(GearState gear_state);
void cd_set_battery(BatteryState battery_state);
/// @param percent range in [0; 100]
void cd_set_throttle(uint8_t percent);
/// @param percent range in [0; 100]
void cd_set_brake(uint8_t percent);

#ifdef COCKPIT_DISPLAY_IMPLEMENTATION

#include <stdlib.h>

void _cd_terminate(void) {
  CD_UART_PUTC(0xFF);
  CD_UART_PUTC(0xFF);
  CD_UART_PUTC(0xFF);
}

void cd_set_blinker_left(const bool enable) {
  CD_UART_PUTS("bL.pic=");

  if (enable) {
    CD_UART_PUTC('1');
  } else {
    CD_UART_PUTC('0');
  }

  _cd_terminate();
}

void cd_set_blinker_right(const bool enable) {
  CD_UART_PUTS("bR.pic=");

  if (enable) {
    CD_UART_PUTC('3');
  } else {
    CD_UART_PUTC('2');
  }

  _cd_terminate();
}

void cd_set_warnblinker(const bool enable) {
  CD_UART_PUTS("warnB.pic=1");

  if (enable) {
    CD_UART_PUTC('6');
  } else {
    CD_UART_PUTC('5');
  }

  _cd_terminate();
}

void cd_set_light(const LightState light_state) {
  CD_UART_PUTS("warnB.pic=1");

  switch (light_state) {
  case LIGHT_LOW_BEAM:
    CD_UART_PUTC('7');
    break;
  case LIGHT_HIGH_BEAM:
    CD_UART_PUTC('8');
    break;
  case LIGHT_PARKING:
    CD_UART_PUTC('9');
    break;
  }

  _cd_terminate();
}

/// @param digit range: [0; 9]
void _cd_set_custom_digit(const uint8_t digit) {
  const uint8_t digit_shifted = digit + 4;

  if (digit_shifted >= 10) { // values in the range: [10; 13]
    const uint8_t digit_shifted_ones = digit_shifted - 10;
    CD_UART_PUTC('1');
    CD_UART_PUTC(digit_shifted_ones + '0');
  } else {
    CD_UART_PUTC(digit_shifted + '0');
  }
}

void cd_set_rpm(uint16_t rpm) {
  const uint8_t ones = rpm % 10;

  CD_UART_PUTS("rpm3.pic=");
  _cd_set_custom_digit(ones);
  _cd_terminate();

  rpm -= ones;
  const uint8_t tens = (rpm / 10) % 10;

  CD_UART_PUTS("rpm2.pic=");
  _cd_set_custom_digit(tens);
  _cd_terminate();

  rpm -= tens * 10;
  const uint8_t hundreds = (rpm / 100) % 10;

  CD_UART_PUTS("rpm1.pic=");
  _cd_set_custom_digit(hundreds);
  _cd_terminate();
}

void cd_set_speed(uint8_t speed_kmh) {
  const uint8_t ones = speed_kmh % 10;

  CD_UART_PUTS("speed2.pic=");
  _cd_set_custom_digit(ones);
  _cd_terminate();

  speed_kmh -= ones;
  const uint8_t tens = (speed_kmh / 10) % 10;

  CD_UART_PUTS("speed1.pic=");
  _cd_set_custom_digit(tens);
  _cd_terminate();
}

void cd_set_gear(const GearState gear_state) {
  CD_UART_PUTS("gear.pic=");

  switch (gear_state) {
  case GEAR_NEUTRAL:
    CD_UART_PUTC('1');
    CD_UART_PUTC('4');
    break;
  case GEAR_FIRST:
    CD_UART_PUTC('5');
    break;
  case GEAR_SECOND:
    CD_UART_PUTC('6');
    break;
  case GEAR_THIRD:
    CD_UART_PUTC('7');
    break;
  }

  _cd_terminate();
}

void cd_set_battery(const BatteryState battery_state) {
  CD_UART_PUTS("battery.pic=2");

  switch (battery_state) {
  case BATTERY_EMPTY:
    CD_UART_PUTC('0');
    break;
  case BATTERY_ONE_FOURTH:
    CD_UART_PUTC('1');
    break;
  case BATTERY_TWO_FOURTH:
    CD_UART_PUTC('2');
    break;
  case BATTERY_THREE_FOURTH:
    CD_UART_PUTC('3');
    break;
  case BATTERY_FOUR_FOURTH:
    CD_UART_PUTC('4');
    break;
  case BATTERY_CHARGING:
    CD_UART_PUTC('5');
    break;
  }

  _cd_terminate();
}

void cd_set_throttle(const uint8_t percent) {
  char percent_str[7]; // heading, 5 digit bytes, NULL
  utoa(percent, percent_str, 10);

  CD_UART_PUTS("throttle.val=");
  CD_UART_PUTS(percent_str);
  _cd_terminate();
}

void cd_set_brake(const uint8_t percent) {
  char percent_str[7]; // heading, 5 digit bytes, NULL
  utoa(percent, percent_str, 10);

  CD_UART_PUTS("brake.val=");
  CD_UART_PUTS(percent_str);
  _cd_terminate();
}

#endif
#endif
