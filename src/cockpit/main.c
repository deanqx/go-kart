#include "can.h"
#include "usart.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#define COCKPIT_DISPLAY_IMPLEMENTATION
#include "cockpit_display.h"

int main(void) {
  uart0_init(BAUD_CALC(9600UL));
  uart1_init(BAUD_CALC(9600UL));

  if (!can_init(BITRATE_125_KBPS)) {
    uart0_puts("error: while can initialization\r\n");

    while (true) {
    }
  }

  can_filter_t can_filter = {
      .id = 0x00001234,
      .mask = 0x1FFFFFFF,
      .flags.extended = 0x3, // filter with extended id
      .flags.rtr = 0x2,      // receive both RTR and normal
  };

  if (!can_set_filter(0, &can_filter)) {
    uart0_puts("error: while setting can message filters\r\n");

    while (true) {
    }
  }

  const can_t msg = {
      .id = 0x12340000,
      .flags.extended = true,
      .flags.rtr = false,
      .length = 1,
      .data = {0xAA},
  };

  sei();
  while (1) {
    if (can_send_message(&msg) == 0) {
      uart0_puts("error: can_send_message\r\n");
      continue;
    }

    can_t received_command;

    if (can_get_message(&received_command)) {
      received_command.id = 0x12340000;
      received_command.flags.extended = true;
      received_command.flags.rtr = false;
      can_send_message(&received_command); // echo for testing
    }

    _delay_ms(1000);
  }

  uart0_puts("JCS-BK_1091_GO-KART\r\n");
  uart0_puts("Lenkrad und Anzeige\r\n");
  uart0_puts("von Dean Schneider (GYT26)\r\n");

  cd_set_rpm(789);
  cd_set_speed(12);

  cd_set_throttle(80);
  cd_set_brake(60);

  while (1) {
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
