#ifndef EEPROM_LOGGER_H
#define EEPROM_LOGGER_H
#include "pico/types.h"

#define MAX_LOG_MSG 61 //includes the NULL

int write_log_entry(uint index, char* msg);
int read_log_entry(uint index, char* dest, size_t size);
int erase_log_entry(uint index);

void init_auto_index_log();
bool erase_log();
int write_log(char* msg);
void dump_log();

#endif //EEPROM_LOGGER_H