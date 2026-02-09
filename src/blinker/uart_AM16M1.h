/*
        RS232 Bibliothek von MS 24.05.10
        für die ATMegareihe.

*/
#ifndef BAUD
#define BAUD 19200UL // Baudrate
#endif

// Berechnungen
#define UBRR_VAL ((F_CPU + BAUD * 8) / (BAUD * 16) - 1) // clever runden
#define BAUD_REAL (F_CPU / (16 * (UBRR_VAL + 1)))       // Reale Baudrate
#define BAUD_ERROR                                                             \
  ((BAUD_REAL * 1000) / BAUD) // Fehler in Promille, 1000 = kein Fehler.

#include <avr/io.h>

#if ((BAUD_ERROR < 990) || (BAUD_ERROR > 1010))
#error Systematischer Fehler der Baudrate groesser 1% und damit zu hoch!
#endif

void usart_init(void) {
#if defined(__AVR_ATmega16M1__)

  //	#warning "Der Wert fuer die Baurate betraegt "(__UBRR_VAL__)" "BAUD"
  //"BAUD_REAL" "BAUD_ERROR" ---"
  LINCR = 0b10000111; // Software zurücksetzen LSWRES LIN13 LCONF[1:0] LENA
                      // LCMD[2:0]
  LINBRRH = (uint8_t)(UBRR_VAL >> 8); // Baudrate setzen
  LINBRRL = (uint8_t)(UBRR_VAL);
  //	LINBRR = 51;			//--> !!Baudrate fest auf 9600!!
  LINCR = 0b00001111;
  // LIN_UARTenable(LENA=1) Uartaktiv(LCMD2=1) und RXaktiv(LCMD1=1) und
  // TXaktiv(LCMD0=1) LSWRES(1=> LINReset), LIN13(1=>LIN1.3,0=>LIN2.1)
  // LCONF[1:0](00=8-bit, no parity;01=8bit,even parity;10=8-bit,odd
  // parity;11=8bit,no parity listen)
#else
  UBRRH = (uint8_t)(UBRR_VAL >> 8);
  UBRRL = (uint8_t)(UBRR_VAL);

  // Enable receiver and transmitter; enable RX interrupt
  UCSRB = (1 << RXEN) | (1 << TXEN); //  | (1 << RXCIE);
  // asynchronous 8 N 1
  UCSRC = (1 << URSEL) | (3 << UCSZ0);
#endif
}

/* Zusaetzlich zur Baudrateneinstellung und der weiteren Initialisierung: */
void usart_EnableRX(void) {
#if defined(__AVR_ATmega16M1__)
  LINCR |=
      (1 << LCMD2) | (1 << LCMD1); // Uartaktiv(LCMD2=1) und RXaktiv(LCMD1=1)
#else
  UCSRB |= (1 << RXEN);
#endif
}

/* Warten, bis Zeichen empfangen */
uint8_t usart_getc(void) {
#if defined(__AVR_ATmega16M1__)
  while (!(LINSIR & (1 << LRXOK)))
    ;            // warten bis Zeichen verfuegbar
  return LINDAT; // Zeichen aus LINDAT an Aufrufer zurueckgeben
#else
        while (!((UCSRA & (1<<RXC)));			// warten bis Zeichen verfuegbar
	return UDR;								// Zeichen aus UDR an Aufrufer zurueckgeben
#endif
}

/* Zeichen empfangen */
uint8_t usart_getc_free(void) {
#if defined(__AVR_ATmega16M1__)
  if (LINSIR & (1 << LRXOK))
    return LINDAT; // Falls Zeichen verfügbar, es aus UDR auslesen und
                   // zurückgeben
#else
  if ((UCSRA & (1 << RXC)))
    return UDR; // Falls Zeichen verfügbar, es aus UDR auslesen und zurückgeben
#endif
  return 0;
}

/* Zeichen abfragen */
uint8_t kbhit(void) {
#if defined(__AVR_ATmega16M1__)
  if (LINSIR & (1 << LRXOK))
    return 1; // Falls Zeichen empfangen wurde, "1" zurückgeben
#else
  if ((UCSRA & (1 << RXC)))
    return 1; // Falls Zeichen empfangen wurde, "1" zurückgeben
#endif
  return 0;
}

/* Zeichen senden */
void usart_putc(char c) {
#if defined(__AVR_ATmega16M1__)
  while ((LINSIR & (1 << LBUSY)))
    ;         // Warten, bis Zeichen versendet werden kann
  LINDAT = c; // Zeichen senden
#else
  while (!(UCSRA & (1 << UDRE)))
    ;      // Warten, bis Zeichen versendet werden kann
  UDR = c; // Zeichen senden
#endif
}

/* Zeichenkette senden */
void usart_puts(char *s) {
  while (*s) // Solange wie "s" Zeichen hat
  {
    usart_putc(*s); // Zeichen senden
    s++;            // abzufragendes Zeichen um 1 erhöhen
  }
}

void usart_puthex(uint8_t data) {
  const uint8_t first_digit = data >> 4;
  usart_putc(first_digit <= 9 ? '0' + first_digit : 'A' + (first_digit - 10));

  const uint8_t second_digit = data & 0x0f;
  usart_putc(second_digit <= 9 ? '0' + second_digit
                               : 'A' + (second_digit - 10));
}
