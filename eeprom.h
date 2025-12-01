#ifndef EEPROM_H
#define EEPROM_H

#include "pico/types.h"

#define MAX_ADDR 0x7FFF
#define MIN_ADDR 0x0000

void init_eeprom();

int write_byte(uint16_t addr, uint8_t byte);
int read_byte(uint16_t addr, uint8_t* byte);

int write_page(uint16_t addr, uint8_t* bytes, size_t size);
int read_page(uint16_t addr, uint8_t* bytes, size_t size);

uint16_t crc16(const uint8_t *data_p, size_t length);

#endif //EEPROM_H