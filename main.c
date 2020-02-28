#include <avr/io.h>
#include <util/delay.h>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "softusart.h"
#include "pzem004t.h"

void setup() {
    softusart_init();
    pzem_init();

    fputs("Reset energy.", softusart);
    pzem_reset_energy(PZEM_DEFAULT_ADDR);

    fputs("Set address to 0x42.", softusart);
    pzem_setaddr(PZEM_DEFAULT_ADDR, 0x42);

    /* blinking led for debugging purposes */
    DDRB |= (1 << PB3);
}

int main() {
    setup();

    struct pzem_measurements m;
    while (1) {
        _delay_ms(1000);
        PORTB ^= (1 << PB3);

        if (pzem_read(0x42, &m)) {
            fputs("Reading ok!\n", softusart);
        } else {
            fputs("Unable to read PZEM004T.\n", softusart);
            continue;
        }

        fprintf(softusart, "Voltage: %.2f\n", m.voltage);
        fprintf(softusart, "Current: %.2f\n", m.current);
        fprintf(softusart, "Power: %.2f\n", m.power);
        fprintf(softusart, "Energy: %.2f\n", m.energy);
        fprintf(softusart, "Frequency: %.2f\n", m.frequency);
        fprintf(softusart, "Power factor: %.2f\n", m.pf);
        fputs(m.alarm ? "Alarm on.\n" : "Alarm off.\n", softusart);
        fputc('\n', softusart);
    }
}
