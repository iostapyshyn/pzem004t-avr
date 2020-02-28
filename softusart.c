#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdio.h>

#include "softusart.h"

#define TXPIN 7
#define TXDDR DDRD
#define TXPORT PORTD

#define BAUDRATE 9600
#define STOPBITS 2

#define tx_sethigh() (TXPORT |= (1 << TXPIN))
#define tx_setlow() (TXPORT &= ~(1 << TXPIN))

#define TX_BITS_START (9 + STOPBITS) /* start bit + 8 data bits + stop bits */

/* holds number of bits left to transmit */
static volatile uint8_t tx_bits = 0;
static volatile uint8_t txbyte;

static int softusart_putchar(char c, FILE *stream) {
    if (c == '\n')
        softusart_send_blocking('\r');
    softusart_send_blocking(c);
    return 0;
}
static FILE _softusart = FDEV_SETUP_STREAM(softusart_putchar, NULL, _FDEV_SETUP_WRITE);

FILE *softusart = &_softusart;

void softusart_init() {
    /* set the tx pin for output */
    TXDDR |= (1 << TXPIN);

    /* ORC0A is calculated as follows:
     * BAUD_DIV = (F_CPU / DIV) / BAUDRATE,
     * where DIV is the TCCR0B divisor, 8 is used here */
    OCR0 = (F_CPU / 8) / BAUDRATE;

    /* waveform generation mode: clear timer on compare match with OCR0 */
    TCCR0 = 0b10 /* div by 8 */ | (1 << WGM01);

	TIMSK |= (1 << OCIE0); /* allow TIMER0_COMP interrupt */
    sei();
}

void softusart_send_blocking(uint8_t c) {
    /* wait for the previous transmission to end */
    while (tx_bits);

    txbyte = c;
    tx_bits = TX_BITS_START;
}

int softusart_send_nonblocking(uint8_t c) {
    if (tx_bits) {
        return 0;
    } else {
        txbyte = c;
        tx_bits = TX_BITS_START;
    }
}

int softusart_status() {
    return !!!tx_bits;
}

ISR(TIMER0_COMP_vect) {
    if (tx_bits != 0) {
        if (tx_bits == TX_BITS_START) {
            tx_setlow();
            tx_bits--;
        } else if (tx_bits <= STOPBITS) {
            tx_sethigh();
            tx_bits--;
        } else {
            if (txbyte & 1) {
                tx_sethigh();
            } else {
                tx_setlow();
            }
            tx_bits--;
            txbyte >>= 1;
        }
    }
}
