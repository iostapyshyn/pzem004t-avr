#ifndef PZEM004T_H
#define PZEM004T_H

#include <stdbool.h>

#define PZEM_DEFAULT_ADDR 0xF8

struct pzem_measurements {
    float voltage;
    float current;
    float power;
    float energy;
    float frequency;
    float pf;
    bool alarm;
};

void pzem_init();
bool pzem_reset_energy(uint8_t addr);
bool pzem_setaddr(uint8_t addr, uint8_t newaddr);
bool pzem_read(uint8_t addr, struct pzem_measurements *m);

#endif
