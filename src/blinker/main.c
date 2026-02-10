/*
 * Pinbelegung ist in hal.h zu finden.
 * Timer0 wird für periodische Statusnachricht über CAN und die Blinker LEDs
 * genutzt. Timer1 wird für das Bremslicht PWM genutzt.
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
static const char PRINT_UART_COMMANDS = 0xff;
static const uint32_t CAN_ID = 0x00001234;
static const uint32_t CAN_ID_STATE_MESSAGE = 0x00001235;

static uint8_t light_state = 0;
static bool send_light_state = false;

/// @returns 1 for error and 0 for successful
uint8_t process_command_from_uart(const char command_char) {
  switch (command_char) {
  case '1':
    light_state |= (1 << CMD_BREAKLIGHT_BIT);
    break;
  case '2':
    light_state &= ~(1 << CMD_BREAKLIGHT_BIT);
    break;
  case '3':
    light_state |= (1 << CMD_BLINKER_BIT);
    break;
  case '4':
    light_state &= ~(1 << CMD_BLINKER_BIT);
    break;
  case '5':
    light_state |= (1 << CMD_REVERSELIGHT_BIT);
    break;
  case '6':
    light_state &= ~(1 << CMD_REVERSELIGHT_BIT);
    break;
  default:
    uart_puts("error: unknown command\n");
  case PRINT_UART_COMMANDS:
    uart_puts("info: UART Befehle:\n");
    uart_puts("info: An  | Aus | Funktion\n");
    uart_puts("info: ----------------------\n");
    uart_puts("info: '1' | '2'  | Bremslicht\n");
    uart_puts("info: '3' | '4'  | Blinker\n");
    uart_puts("info: '5' | '6'  | Rückfahrlicht\n");
    return 1;
  }

  return 0;
}

void add_timer0_state_message(void) {
  // Reusing timer0 from Blinker (blinker_controller.h)
  OCR0B = 100;
  // enable interrupt for B
  TIMSK0 |= 1 << OCIE0B;
}

ISR(TIMER0_COMPB_vect) {
  static uint8_t interrupt_count = 0;

  interrupt_count++;

  // about 1 Hz
  // timer frequency / SOFT_PRESCALER = frequency
  if (interrupt_count == OCR0A) {
    interrupt_count = 0;
    send_light_state = true;
  }
}

void init(void) {
  uart_init();
  hal_init();
  bc_init_timer0_timer1();
  add_timer0_state_message();

  uart_puts("\ninfo: JCS-BK_1091_GO-KART von Dean Schneider (GYT26)\n");
  uart_puts("info: Blinker-Modul mit CAN-ID: 0x");
  uart_puthex((uint8_t)(CAN_ID >> 8 * 3));
  uart_puthex((uint8_t)(CAN_ID >> 8 * 2));
  uart_puthex((uint8_t)(CAN_ID >> 8 * 1));
  uart_puthex((uint8_t)(CAN_ID >> 8 * 0));
  uart_puts("\ninfo: State Nachrichten werden an 0x");
  uart_puthex((uint8_t)(CAN_ID_STATE_MESSAGE >> 8 * 3));
  uart_puthex((uint8_t)(CAN_ID_STATE_MESSAGE >> 8 * 2));
  uart_puthex((uint8_t)(CAN_ID_STATE_MESSAGE >> 8 * 1));
  uart_puthex((uint8_t)(CAN_ID_STATE_MESSAGE >> 8 * 0));
  uart_puts(" gesendet.\n");
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
        goto continue_main_loop;
      }

      uart_puts("info: received command over CAN\n");

      if (received_can_message.length != 1) {
        uart_puts("error: DLC does not equal 1\n");
        goto continue_main_loop;
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

  continue_main_loop:
    if (send_light_state) {
      send_light_state = false;

      const can_t light_state_message = {
          .id = 0x200,
          .flags.rtr = false,
          .flags.extended = true,
          .length = 1,
          .data = {light_state},
      };

      uart_puts("info: send light_state over CAN\n");
      can_send_message(&light_state_message);
    }
  }
}
