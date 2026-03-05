/*
 * Pinbelegung ist in hal.h zu finden.
 * uart0 für Steuerung des Displays.
 * uart1 für Ausgabe über Bluetooth.
 *
 * Author: Dean Schneider (GYT26)
 */

#include "can.h"
#include "hal.h"
#include "usart.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/iocan32.h>
#include <stdint.h>
#include <util/delay.h>

#define COCKPIT_DISPLAY_IMPLEMENTATION
#include "cockpit_display.h"

#define REVERSELIGHT_BIT 0
#define BLINKER_BIT 1
#define BACKLIGHT_BIT 2
#define BRAKE_BIT 3

typedef enum {
  STOP,
  FORWARD,
  REVERSE,
} MotorState;

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

MotorState motor_state;
GearState gear_state = GEAR_NEUTRAL;
LightState light_state = LIGHT_OFF;
bool horn;
volatile bool blinker_left_live;
volatile bool light_switch;
volatile bool gear_up;
volatile bool gear_down;
volatile bool blinker_right_live;

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

#define TIMER0_SOFT_DIVISION 4

ISR(TIMER0_COMP_vect) {
  static uint8_t interrupt_count = 0;

  if (interrupt_count < TIMER0_SOFT_DIVISION) {
    interrupt_count++;
    return;
  }
  interrupt_count = 0;

  static bool previous_blinker_left = false;
  static bool previous_light_switch = false;
  static bool previous_gear_up = false;
  static bool previous_gear_down = false;
  static bool previous_blinker_right = false;

  // --- Einlesen ---
  const bool current_blinker_left = GET(SW_BLINKER_LEFT);
  const bool current_light_switch = GET(SW_LIGHT_SWITCH);
  const bool current_gear_up = GET(SW_GEAR_UP);
  const bool current_gear_down = GET(SW_GEAR_DOWN);
  const bool current_blinker_right = GET(SW_BLINKER_RIGHT);

  // --- Verarbeiten ---
  if (current_blinker_left && !previous_blinker_left) {
    // button pressed
    previous_blinker_left = true;
    blinker_left_live = !blinker_left_live;
  } else if (!current_blinker_left && previous_blinker_left) {
    // button released
    previous_blinker_left = false;
  } // current == previous -> nothing has changed

  if (current_light_switch && !previous_light_switch) {
    // button pressed
    previous_light_switch = true;
    SET(LED_LIGHT_SWITCH);
  } else if (!current_light_switch && previous_light_switch) {
    // button released
    previous_light_switch = false;
    RESET(LED_LIGHT_SWITCH);
    light_switch = true; // is set to false when processed
  } // current == previous -> nothing has changed

  if (current_gear_up && !previous_gear_up) {
    // button pressed
    previous_gear_up = true;
    SET(LED_GEAR_UP);
  } else if (!current_gear_up && previous_gear_up) {
    // button released
    previous_gear_up = false;
    RESET(LED_GEAR_UP);
    gear_up = true; // is set to false when processed
  } // current == previous -> nothing has changed

  if (current_gear_down && !previous_gear_down) {
    // button pressed
    previous_gear_down = true;
    SET(LED_GEAR_DOWN);
  } else if (!current_gear_down && previous_gear_down) {
    // button released
    previous_gear_down = false;
    RESET(LED_GEAR_DOWN);
    gear_down = true; // is set to false when processed
  } // current == previous -> nothing has changed

  if (current_blinker_right && !previous_blinker_right) {
    // button pressed
    previous_blinker_left = true;
    blinker_right_live = !blinker_right_live;
  } else if (!current_blinker_right && previous_blinker_right) {
    // button released
    previous_blinker_right = false;
  } // current == previous -> nothing has changed
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

  // # configure timer for toggle buttons
  // clear timer value on compare match
  // and set prescaler to 1024: 16 MHz / 1024 = 15625 Hz
  TCCR0A = 1 << WGM01 | 1 << CS02 | 1 << CS00;
  // enable interrupt on compare match with OCR0A
  TIMSK0 = 1 << OCIE0A;
  // 15625 Hz / 255 = 61.27 Hz
  OCR0A = 255;
  // with a software division of 4: Timer0 at 15.32 Hz

  // these have default values at this point
  cd_set_gear(gear_state);
  cd_set_light(light_state);
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

    cd_set_light(LIGHT_DAY);
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

  GearState prev_gear_state = gear_state;
  LightState prev_light_state = light_state;
  bool prev_blinker_left = false;
  bool prev_horn = false;
  bool prev_blinker_right = false;

  while (true) {
    // --- Einlesen ---
    const bool blinker_left = blinker_left_live;
    const bool blinker_right = blinker_right_live;
    horn = GET(SW_WARNBLINKER_HORN);

    if (GET(SW_MOTOR_START_LA)) {
      if (GET(SW_MOTOR_REVERSE_LA)) {
        motor_state = STOP;
      } else {
        motor_state = REVERSE;
      }
    } else {
      if (GET(SW_MOTOR_REVERSE_LA)) {
        motor_state = FORWARD;
      } else {
        motor_state = STOP;
        // error: Vorwärts- und Rückwärtsfahren nicht gleichzeitig
        // möglich. Taster auf defekt überprüfen.
      }
    }

    // --- Verarbeiten ---
    if (light_switch) {
      light_switch = false;
      light_state = (light_state + 1) % LIGHT_STATE_COUNT;
    }

    if (gear_up) {
      gear_up = false;

      if (gear_state < GEAR_STATE_COUNT - 1) {
        gear_state++;
      }
    } else if (gear_down) {
      gear_down = false;

      if (gear_state > 0) {
        gear_state--;
      }
    }

    // --- Ausgeben ---
    if (blinker_left != prev_blinker_left) {
      prev_blinker_left = blinker_left;
      cd_set_blinker_left(blinker_left);
      if (blinker_left) {
        SET(LED_BLINKER_LEFT);
      } else {
        RESET(LED_BLINKER_LEFT);
      }
    }

    if (light_state != prev_light_state) {
      cd_set_light(light_state);
    }

    if (horn != prev_horn) {
      if (horn) {
        SET(LED_WARNBLINKER_HORN);
        SET(HORN_PWM);
      } else {
        RESET(LED_WARNBLINKER_HORN);
        RESET(HORN_PWM);
      }
    }

    if (gear_state != prev_gear_state) {
      cd_set_gear(gear_state);
    }

    if (blinker_right != prev_blinker_right) {
      cd_set_blinker_right(blinker_right);
      if (blinker_right) {
        SET(LED_BLINKER_RIGHT);
      } else {
        RESET(LED_BLINKER_RIGHT);
      }
    }

    // some button LEDs are set in the ISR for simplicity

    prev_gear_state = gear_state;
    prev_light_state = light_state;
    prev_horn = horn;
    prev_blinker_right = blinker_right;
  }

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
