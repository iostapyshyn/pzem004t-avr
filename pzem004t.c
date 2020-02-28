#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdbool.h>

#include "pzem004t.h"
#include "crc16.h"
#include "usart.h"

#define REG_VOLTAGE     0x0000
#define REG_CURRENT_L   0x0001
#define REG_CURRENT_H   0X0002
#define REG_POWER_L     0x0003
#define REG_POWER_H     0x0004
#define REG_ENERGY_L    0x0005
#define REG_ENERGY_H    0x0006
#define REG_FREQUENCY   0x0007
#define REG_PF          0x0008
#define REG_ALARM       0x0009

#define CMD_RHR         0x03
#define CMD_RIR         0X04
#define CMD_WSR         0x06
#define CMD_CAL         0x41
#define CMD_REST        0x42

#define WREG_ALARM_THR   0x0001
#define WREG_ADDR        0x0002

#define READ_TIMEOUT 1000000
#define PZEM_BAUD 9600

/* sets up the internal usart for pzem004t device */
void pzem_init() {
    usart_init(PZEM_BAUD);
}

static uint8_t pzem_receive(uint8_t *response, uint8_t len) {
    unsigned long timeout = READ_TIMEOUT;

    uint8_t i = 0;
    while (i < len && timeout--) {
        if (usart_read(&response[i])) {
            i++;
        }
    }

    /* TODO check crc */

    return i;
}

static bool pzem_cmd8(uint8_t addr, uint8_t cmd, uint16_t reg, uint16_t value, bool check) {
    uint8_t buffer[8];

    /* slave address and the command */
    buffer[0] = addr;
    buffer[1] = cmd;

    /* register selection */
    buffer[2] = reg >> 8;
    buffer[3] = reg;

    /* command parameter */
    buffer[4] = value >> 8;
    buffer[5] = value;

    /* crc */
    uint16_t crc = crc16(buffer, 6);
    buffer[6] = crc >> 8;
    buffer[7] = crc;

    usart_sendbuf(buffer, 8);

    if (check) {
        uint8_t response[8];

        if (!pzem_receive(response, 8)) {
            return false;
        }

        for (uint8_t i; i < 8; i++) {
            if (response[i] != buffer[i])
                return false;
        }
    }

    return true;
}

bool pzem_read(uint8_t addr, struct pzem_measurements *m) {
    static uint8_t response[25];

    /* read 10 registers starting at 0x00 */
    pzem_cmd8(addr, CMD_RIR, 0x0000, 10, false);

    if (pzem_receive(response, 25) != 25) {
        return false;
    }

    m->voltage =   ((uint32_t)response[3] << 8 | // Raw voltage in 0.1V
                    (uint32_t)response[4])/10.0;

    m->current =   ((uint32_t)response[5] << 8 | // Raw current in 0.001A
                    (uint32_t)response[6] |
                    (uint32_t)response[7] << 24 |
                    (uint32_t)response[8] << 16) / 1000.0;

    m->power =     ((uint32_t)response[9] << 8 | // Raw power in 0.1W
                    (uint32_t)response[10] |
                    (uint32_t)response[11] << 24 |
                    (uint32_t)response[12] << 16) / 10.0;

    m->energy =    ((uint32_t)response[13] << 8 | // Raw Energy in 1Wh
                    (uint32_t)response[14] |
                    (uint32_t)response[15] << 24 |
                    (uint32_t)response[16] << 16) / 1000.0;

    m->frequency = ((uint32_t)response[17] << 8 | // Raw Frequency in 0.1Hz
                    (uint32_t)response[18]) / 10.0;

    m->pf =        ((uint32_t)response[19] << 8 | // Raw pf in 0.01
                    (uint32_t)response[20])/100.0;

    m->alarm =     ((uint32_t)response[21] << 8 | // Raw alarm value
                    (uint32_t)response[22]);

}

bool pzem_setaddr(uint8_t addr, uint8_t newaddr) {
    return pzem_cmd8(PZEM_DEFAULT_ADDR, CMD_WSR, WREG_ADDR, addr, true);
}

bool pzem_reset_energy(uint8_t addr) {
    uint8_t buffer[4] = { addr, CMD_REST, 0x00, 0x00 };
    uint8_t reply[5];

    uint16_t crc = crc16(buffer, 2);
    buffer[2] = crc >> 8;
    buffer[3] = crc;

    usart_sendbuf(buffer, 4);

    /* response should be exactly 4 bytes long, no more no less */
    if (pzem_receive(reply, 5) != 4)
        return false;

    return true;
}
