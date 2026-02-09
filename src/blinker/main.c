/*
 * Pinbelegung ist in hal.h zu finden.
 * Timer0 wird für periodische Statusnachricht über CAN genutzt.
 * Timer1 wird für die Blinker LEDs und Bremslicht PWM genutzt.
 */

#include "blinker_controller.c"
#include "can.h"
#include "hal.h"
#include "uart_AM16M1.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

#define CMD_BREAKLIGHT_BIT 0
#define CMD_BLINKER_BIT 1
#define CMD_REVERSELIGHT_BIT 2
static const char PRINT_UART_COMMANDS = '0' - 1;
static const uint32_t CAN_ID = 0x00001234;

static uint8_t light_state = 0;
static bool send_light_state = false;

/// @returns 1 for error and 0 for successful
uint8_t process_command_from_uart(const char command_char) {
  switch (command_char) {
  case '0':
    light_state |= (1 << CMD_BREAKLIGHT_BIT);
    break;
  case '1':
    light_state &= ~(1 << CMD_BREAKLIGHT_BIT);
    break;
  case '2':
    light_state |= (1 << CMD_BLINKER_BIT);
    break;
  case '3':
    light_state &= ~(1 << CMD_BLINKER_BIT);
    break;
  case '4':
    light_state |= (1 << CMD_REVERSELIGHT_BIT);
    break;
  case '5':
    light_state &= ~(1 << CMD_REVERSELIGHT_BIT);
    break;
  default:
    uart_puts("error: unknown command\n");
  case PRINT_UART_COMMANDS:
    uart_puts("info: UART Befehle:\n");
    uart_puts("info: An  | Aus | Funktion\n");
    uart_puts("info: ----------------------\n");
    uart_puts("info: '0' | '1'  | Bremslicht\n");
    uart_puts("info: '2' | '3'  | Blinker\n");
    uart_puts("info: '4' | '5'  | Rückfahrlicht\n");
    return 1;
  }

  return 0;
}

void init_timer0(void) {
  // clear timer on compare match with OCRA
  TCCR0A = 1 << WGM01;
  // F_CPU / 1024 = 16 MHz / 1024 = 15625 Hz
  TCCR0B = 1 << CS02 | 1 << CS00;
  OCR0A = 255; // Timer0 at 61 Hz
  TIMSK0 = 1 << OCIE0A;
}

ISR(TIMER0_COMPA_vect) {
  static const uint8_t SOFT_PRESCALER = 60; // about 1 Hz
  static uint8_t interrupt_count = 0;

  interrupt_count++;

  // timer frequency / SOFT_PRESCALER = frequency
  if (interrupt_count == SOFT_PRESCALER) {
    interrupt_count = 0;
    send_light_state = true;
  }
}

void init(void) {
  uart_init();
  hal_init();
  init_timer0();
  bc_init_timer1();

  uart_puts("\ninfo: JCS-BK_1091_GO-KART von Dean Schneider (GYT26)\n");
  uart_puts("info: Blinker-Modul mit CAN-ID: 0x");
  uart_puthex((uint8_t)(CAN_ID >> 8 * 3));
  uart_puthex((uint8_t)(CAN_ID >> 8 * 2));
  uart_puthex((uint8_t)(CAN_ID >> 8 * 1));
  uart_puthex((uint8_t)(CAN_ID >> 8 * 0));
  uart_putc('\n');
  process_command_from_uart(PRINT_UART_COMMANDS);

  if (!can_init(BITRATE_125_KBPS)) {
    uart_puts("fatal error: while can initialization\n");

    while (true) {
      TOGGLE(INTERNAL_LED_LA);
      _delay_ms(200);
    }
  }

  can_filter_t can_filter = {
      .id = CAN_ID,
      .mask = 0x1FFFFFFF,    // only accept exactly this ID
      .flags.extended = 0x3, // filter with extended id
      .flags.rtr = 0x2,      // receive both RTR and normal
  };

  if (!can_set_filter(0, &can_filter)) {
    uart_puts("fatal error: while setting can message filters\n");

    while (true) {
      TOGGLE(INTERNAL_LED_LA);
      _delay_ms(200);
    }
  }
}

int main(void) {
  init();
  sei();

  can_t received_can_message;

  while (1) {
    // --- Einlesen ---
    const char command_from_uart = uart_getc();

    if (command_from_uart != 0) {
      uart_puts("info: received command over UART: ");
      uart_putc(command_from_uart);
      uart_putc('\n');

      process_command_from_uart(command_from_uart);
    } else { // no uart command
      if (!can_get_message(&received_can_message)) {
        continue;
      }

      uart_puts("info: received command over CAN\n");

      if (received_can_message.length != 1) {
        uart_puts("error: DLC does not equal 1\n");
        continue;
      }

      light_state = received_can_message.data[0];
    }

    // --- Ausgeben ---
    uart_puts("info: light_state=");
    uart_puthex(light_state);
    uart_putc('\n');

    if (light_state & (1 << CMD_BREAKLIGHT_BIT)) {
      bc_break();
    } else {
      bc_backlight();
    }

    if (light_state & (1 << CMD_BLINKER_BIT)) {
      bc_enable_blinker();
    } else if (bc_is_blinker_enabled()) {
      bc_disable_blinker();
    }

    if (light_state & (1 << CMD_REVERSELIGHT_BIT)) {
      SET(LED_REVERSE);
    } else {
      RESET(LED_REVERSE);
    }

    if (send_light_state) {
      send_light_state = false;

      const can_t light_state_message = {
          .id = 0x200,
          .flags.rtr = false,
          .flags.extended = true,
          .length = 1,
          .data = {light_state},
      };

      can_send_message(&light_state_message);
    }
  }
}
