#include <avr/io.h>
#include <avr/interrupt.h>

#include "usart.h"

uint8_t usart_rxcount = 0;
volatile uint8_t usart_rxdata;

void usart_init(uint16_t baudrate) {
    const uint16_t ubrr = F_CPU / 16 / baudrate - 1;

    UBRRH = ubrr >> 8;
    UBRRL = ubrr;

    /* enable receiver and transmitter with receival interrupt */
    UCSRB = (1<<RXCIE) | (1<<RXEN) | (1<<TXEN);

    /* set frame format: 8data, 1stop bit */
    UCSRC = (1<<URSEL) | (3<<UCSZ0);
}

ISR(USART_RXC_vect) {
    usart_rxdata = UDR;
    usart_rxcount++;
}

int usart_read(uint8_t *data) {
    uint8_t tmp = usart_rxcount;

    if (usart_rxcount) {
        *data = usart_rxdata;
        usart_rxcount = 0;
    }

    return tmp;
}

void usart_send(uint8_t data) {
    /* wait for the previous transmission to end */
    while (!(UCSRA & (1<<UDRE)));

    /* put the data into the buffer */
    UDR = data;
}

void usart_sendbuf(uint8_t *buf, uint16_t len) {
    for (int i = 0; i < len; i++) {
        usart_send(buf[i]);
    }
}
