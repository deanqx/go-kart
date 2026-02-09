/*
 * Pinbelegung ist in hal.h zu finden.
 *
 * TODO write ZWL and timers
 */

#include "blinker_controller.c"
#include "can.h"
#include "hal.h"
#include "uart_AM16M1.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

static const uint32_t CAN_ID = 0x00001234;

#define BREAK_BIT 0
#define BLINKER_BIT 1
#define BACKLIGHT_BIT 2

void print_uart_command_help(void) {
  uart_puts("info: UART Befehle:\n");
  uart_puts("info: Bremsenlicht an = '0'\n");
  uart_puts("info: Bremsenlicht aus = '1'\n");
  uart_puts("info: Blinkerlicht an = '2'\n");
  uart_puts("info: Blinkerlicht aus = '3'\n");
  uart_puts("info: Rücklicht an = '4'\n");
  uart_puts("info: Rücklicht aus = '5'\n");
}

int main(void) {
  uart_init();
  hal_init();
  bc_init_timer_interrupt();

  uart_puts("\ninfo: JCS-BK_1091_GO-KART von Dean Schneider (GYT26)\n");
  uart_puts("info: Blinker-Modul mit CAN-ID: 0x");
  uart_puthex((uint8_t)(CAN_ID >> 8 * 3));
  uart_puthex((uint8_t)(CAN_ID >> 8 * 2));
  uart_puthex((uint8_t)(CAN_ID >> 8 * 1));
  uart_puthex((uint8_t)(CAN_ID >> 8 * 0));
  uart_putc('\n');
  print_uart_command_help();

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

  sei();

  can_t received_can_message;
  char state = 0;

  while (1) {
    // --- Einlesen ---
    // read command from UART if one was send
    const char command_uart = uart_getc();

    if (command_uart == 0) { // no uart command
      if (!can_get_message(&received_can_message)) {
        continue;
      }

      uart_puts("info: received command over CAN\n");

      if (received_can_message.flags.rtr == true) {
        const can_t healthy_message = {
            .id = 0x200,
            .flags.rtr = false,
            .flags.extended = true,
            .length = 0,
        };

        can_send_message(&healthy_message);
        continue;
      }

      if (received_can_message.length != 1) {
        uart_puts("error: DLC does not equal 1\n");
        continue;
      }

      state = received_can_message.data[0];
      uart_puts("info: command=");
      uart_puthex(state);
      uart_putc('\n');
    } else {
      uart_puts("info: received command over UART: ");
      uart_putc(command_uart);
      uart_putc('\n');

      // --- Verarbeiten ---
      switch (command_uart) {
      case '0':
        state |= (1 << BREAK_BIT);
        break;
      case '1':
        state &= ~(1 << BREAK_BIT);
        break;
      case '2':
        state |= (1 << BLINKER_BIT);
        break;
      case '3':
        state &= ~(1 << BLINKER_BIT);
        break;
      case '4':
        state |= (1 << BACKLIGHT_BIT);
        break;
      case '5':
        state &= ~(1 << BACKLIGHT_BIT);
        break;
      default:
        uart_puts("error: unknown command\n");
        print_uart_command_help();
        continue;
      }
    }

    // TODO fix bug with state
    // TODO send command over CAN every few seconds
    // TODO breaklight and backlight with PWM, reverse light rename

    // --- Ausgaben ---
    if (state & (1 << BREAK_BIT)) {
      SET(LED_BREAK);
    } else {
      RESET(LED_BREAK);
    }

    if (state & (1 << BLINKER_BIT)) {
      bc_enable_blinker();
    } else if (bc_is_blinker_enabled()) {
      bc_disable_blinker();
    }

    if (state & (1 << BACKLIGHT_BIT)) {
      SET(LED_BACKLIGHT);
    } else {
      RESET(LED_BACKLIGHT);
    }
  }
}
