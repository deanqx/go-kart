/*
 * # Beschreibung
 *
 * Mit dieser Bibliothek kann das Tacho Display gesteuert werden.
 * Die Nachrichten an das Display werden Über UART gesendet.
 * Orientiert an dem Projekt: JCS-BK_1046
 *
 * # Einbinden ins eigene Programm
 *
 * Kopiere cockpit_display.h und cockpit_display.c in das Projekt verzeichnis.
 *
 * ```
 * #include "cockpit_display.h"
 * #include "cockpit_display.c"
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
  GEAR_STATE_COUNT,
} GearState;

typedef enum {
  LIGHT_OFF,
  LIGHT_DAY,
  LIGHT_LOW_BEAM,
  LIGHT_HIGH_BEAM,
  LIGHT_STATE_COUNT,
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

#endif
