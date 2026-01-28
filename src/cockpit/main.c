#define COCKPIT_DISPLAY_IMPLEMENTATION
#include "cockpit_display.h"
#include "usart.h"
#include <avr/io.h>
#include <util/delay.h>

int main(void) {
  uart0_init(BAUD_CALC(9600UL));
  uart0_puts("JCS-BK_1091_GO-KART\r\n");
  uart0_puts("Lenkrad und Anzeige\r\n");
  uart0_puts("von Dean Schneider (GYT26)\r\n");

  uart1_init(BAUD_CALC(9600UL));

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
