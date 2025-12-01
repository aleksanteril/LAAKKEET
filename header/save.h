#ifndef SAVE_H
#define SAVE_H

#include "machine.h"

void init_eeprom();

void save_machine(Machine_t* m);
bool load_machine(Machine_t* m);

#endif //SAVE_H