#ifndef LAAKKEET_MACHINE_H
#define LAAKKEET_MACHINE_H

#include "pico/types.h"
#include "uart.h"

typedef enum Events_t {
        eEnter,
        eExit,
        eTick,
        eSW0,
        eSW1,
        ePiezo,
        NOP
} Events_t;

typedef struct Machine_t Machine_t;
typedef void (*state) (Machine_t* m, Events_t e);
typedef struct Machine_t {
        // To EEPROM
        state state;
        uint8_t pill_count;
        uint8_t turn_count;
        uint16_t steps_dispense;
        // To EEPROM
        uint32_t timer;
        bool calibrated;
        uint8_t step;
        uart_t* uart;
} Machine_t;

#endif //LAAKKEET_MACHINE_H