/*
 * Pinbelegung ist in hal.h zu finden.
 * Timer0 wird für periodische Statusnachricht über CAN und die Blinker LEDs
 * genutzt. Timer1 wird für das Bremslicht PWM genutzt.
 *
 * Author: Dean Schneider (GYT26)
 */

#include "blinker_controller.c"
#include "can.h"
#include "hal.h"
#include "uart_AM16M1.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

#define LOWER_LIGHT_BIT 0
#define BLINKER_BIT 1
#define UPPER_WEAK_LIGHT_BIT 2
#define UPPER_FULL_LIGHT_BIT 3

static const char PRINT_UART_COMMANDS = 0xff;
static const uint32_t CAN_ID_STATE_MESSAGE = 0x703;

#if defined(BLINKER_FRONT) && defined(BLINKER_RIGHT)
static const uint32_t CAN_ID = 0x604;
#elif defined(BLINKER_FRONT) && defined(BLINKER_LEFT)
static const uint32_t CAN_ID = 0x603;
#elif defined(BLINKER_BACK) && defined(BLINKER_RIGHT)
static const uint32_t CAN_ID = 0x602;
#else // defined(BLINKER_BACK) && defined(BLINKER_LEFT)
static const uint32_t CAN_ID = 0x601;
#endif

static uint8_t light_state = 0;
static bool send_light_state = false;

/// @returns 1 for error and 0 for successful
uint8_t process_command_from_uart(const char command_char) {
  switch (command_char) {
  case '1':
    light_state |= (1 << BLINKER_BIT);
    break;
  case '2':
    light_state &= ~(1 << BLINKER_BIT);
    break;
  case '3':
    light_state |= (1 << LOWER_LIGHT_BIT);
    break;
  case '4':
    light_state &= ~(1 << LOWER_LIGHT_BIT);
    break;
  case '5':
    light_state |= (1 << UPPER_WEAK_LIGHT_BIT);
    break;
  case '6':
    light_state |= (1 << UPPER_FULL_LIGHT_BIT);
    break;
  case '7':
    light_state &= ~(1 << UPPER_WEAK_LIGHT_BIT);
    light_state &= ~(1 << UPPER_FULL_LIGHT_BIT);
    break;
  default:
    uart_puts("error: unknown command\n");
  case PRINT_UART_COMMANDS:
    uart_puts("info: UART Befehle:\n");
    uart_puts("info: an='1', aus='2': Blinker\n");
#ifdef BLINKER_FRONT
    uart_puts("info: an='3', aus='4': Tagfahrlicht\n");
    uart_puts("info: '5': Abblendlicht\n");
    uart_puts("info: '6': Fernlicht\n");
#else // BLINKER_BACK
    uart_puts("info: an='3', aus='4': Rueckfahrlicht\n");
    uart_puts("info: '5': Ruecklicht\n");
    uart_puts("info: '6': Bremslicht\n");
#endif
    uart_puts("info: '7': kein oberes Licht\n");
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

      if (process_command_from_uart(command_from_uart)) {
        continue; // help is printed inside previous called funktion
      }
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

    if (light_state & (1 << LOWER_LIGHT_BIT)) {
      SET(LED_LOWER_LIGHT);
    } else {
      RESET(LED_LOWER_LIGHT);
    }

    if (light_state & (1 << BLINKER_BIT)) {
      bc_blinker_on();
    } else if (bc_is_blinker_enabled()) {
      bc_blinker_off();
    }

    if (light_state & (1 << UPPER_WEAK_LIGHT_BIT)) {
      bc_upper_weak_light();
    } else if (light_state & (1 << UPPER_FULL_LIGHT_BIT)) {
      bc_upper_full_light();
    } else {
      bc_upper_light_off();
    }

  continue_main_loop:
    if (send_light_state) {
      send_light_state = false;

      const can_t light_state_message = {
          .id = CAN_ID_STATE_MESSAGE,
          .flags.rtr = false,
          .flags.extended = true,
          .length = 5,
          .data =
              {
                  (uint8_t)(CAN_ID >> 8 * 3),
                  (uint8_t)(CAN_ID >> 8 * 2),
                  (uint8_t)(CAN_ID >> 8 * 1),
                  (uint8_t)(CAN_ID >> 8 * 0),
                  light_state,
              },
      };

      uart_puts("info: send light_state over CAN\n");
      can_send_message(&light_state_message);
    }
  }
}
