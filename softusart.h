#ifndef SOFTUSART_H
#define SOFTUSART_H

#include <ctype.h>
#include <stdio.h>

extern FILE *softusart;

void softusart_init();
void softusart_send_blocking(uint8_t c);
int softusart_send_nonblocking(uint8_t c);

int softusart_status();

#endif
