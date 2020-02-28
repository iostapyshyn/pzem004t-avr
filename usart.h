#ifndef USART_H
#define USART_H

#include <stdbool.h>
#include <ctype.h>

void usart_init(uint16_t baudrate);
int usart_read(uint8_t *data);
void usart_send(uint8_t data);
void usart_sendbuf(uint8_t *buf, uint16_t len);

#endif
