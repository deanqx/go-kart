/*
 * uart0 für Steuerung des Displays.
 * uart1 für Ausgabe über Bluetooth.
 */

#include "can.h"
#include "hal.h"
#include "usart.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#define COCKPIT_DISPLAY_IMPLEMENTATION
#include "cockpit_display.h"

#define REVERSELIGHT_BIT 0
#define BLINKER_BIT 1
#define BACKLIGHT_BIT 2
#define BRAKE_BIT 3

static const uint32_t CAN_ID_RX_BLINKER_STATE = 0x703;
static const uint32_t CAN_ID_RX_SPEED = 0x605;
static const uint32_t CAN_ID_RX_GEAR = 0x401;
static const uint32_t CAN_ID_RX_BATTERY = 0x302;
static const uint32_t CAN_ID_RX_POWER = 0x301;
static const uint32_t CAN_ID_RX_BRAKE = 0x201;

static const uint32_t CAN_ID_TX_BLINKER_FRONT_RIGHT = 0x604;
static const uint32_t CAN_ID_TX_BLINKER_FRONT_LEFT = 0x603;
static const uint32_t CAN_ID_TX_BLINKER_BACK_RIGHT = 0x602;
static const uint32_t CAN_ID_TX_BLINKER_BACK_LEFT = 0x601;
static const uint32_t CAN_ID_TX_DRIVE_STATE = 0x101;

/// @returns 1 for error and 0 for successful
uint8_t process_command_from_uart(const char command_char) {
  uart_puts("error: unknown command\n");
  uart_puts("info: UART Befehle:\n");
  uart_puts("info: Befehl | Wert\n");
  uart_puts("info: --------------------------\n");
  uart_puts("info: rpm    | \n");
  uart_puts("\ninfo: Beispiel: rpm=123\n");
  return 1;
}

void init(void) {
  uart0_init(BAUD_CALC(9600UL));
  uart1_init(BAUD_CALC(9600UL));
  hal_init();

  uart1_puts("\ninfo: JCS-BK_1091_GO-KART von Dean Schneider (GYT26)\n");
  uart1_puts("\ninfo: Cockpit Modul\n");

  if (!can_init(BITRATE_125_KBPS)) {
    while (true) {
      uart1_puts("fatal error: while can initialization\n");
      _delay_ms(1000);
    }
  }

  can_filter_t can_filter = {
      .id = 0,
      .mask = 0,             // disable ID filter, receive all
      .flags.extended = 0x3, // receive with extended id
      .flags.rtr = 0,        // only receive normal messaages (no RTR)
  };

  if (!can_set_filter(0, &can_filter)) {
    while (true) {
      uart1_puts("fatal error: while setting can message filters\n");
      _delay_ms(1000);
    }
  }
}

void test_display(void) {
  cd_set_rpm(789);
  cd_set_speed(12);

  cd_set_throttle(80);
  cd_set_brake(60);

  while (true) {
    cd_set_blinker_left(true);
    _delay_ms(200);
    cd_set_blinker_left(false);
    _delay_ms(200);

    cd_set_blinker_right(true);
    _delay_ms(200);
    cd_set_blinker_right(false);
    _delay_ms(200);

    cd_set_warnblinker(true);
    _delay_ms(500);
    cd_set_warnblinker(false);
    _delay_ms(500);

    cd_set_light(LIGHT_PARKING);
    _delay_ms(700);
    cd_set_light(LIGHT_LOW_BEAM);
    _delay_ms(700);
    cd_set_light(LIGHT_HIGH_BEAM);
    _delay_ms(700);

    cd_set_gear(GEAR_NEUTRAL);
    _delay_ms(200);
    cd_set_gear(GEAR_FIRST);
    _delay_ms(200);
    cd_set_gear(GEAR_SECOND);
    _delay_ms(200);
    cd_set_gear(GEAR_THIRD);
    _delay_ms(200);

    cd_set_battery(BATTERY_EMPTY);
    _delay_ms(500);
    cd_set_battery(BATTERY_ONE_FOURTH);
    _delay_ms(500);
    cd_set_battery(BATTERY_TWO_FOURTH);
    _delay_ms(500);
    cd_set_battery(BATTERY_THREE_FOURTH);
    _delay_ms(500);
    cd_set_battery(BATTERY_FOUR_FOURTH);
    _delay_ms(500);
    cd_set_battery(BATTERY_CHARGING);
    _delay_ms(500);

    _delay_ms(2000);
  }
}

int main(void) {
  init();
  sei();

  // test_display();

  while (true) {
    // --- Einlesen ---
    // --- Verarbeiten ---
    // --- Ausgeben ---
    const can_t test_message = {
        .id = CAN_ID_TX_BLINKER_BACK_LEFT,
        .flags.rtr = false,
        .flags.extended = true,
        .length = 1,
        // Bremslicht und Blinker anschalten
        .data = {1 << BRAKE_BIT | 1 << BLINKER_BIT},
    };

    uart1_puts("info: send test message over CAN to blinker back left\n");
    can_send_message(&test_message);
  }

emergency_loop:
  goto emergency_loop;
}
