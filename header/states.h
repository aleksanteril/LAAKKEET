#ifndef STATES_H
#define STATES_H

#include "network.h"
#include "pico/types.h"

// Time between eTick events in (ms) milliseconds
#define TICK_SLEEP 50
// Time to determine dispense fail a.k.a no piezo event detected until this time. (ms) milliseconds
#define TIME_TO_DISPENSE_FAIL 200
// Time of wait between dispensing the medicine in (s) seconds
#define DISPENSE_INTERVAL 30

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
        state state; // In EEPROM this is converted to HEX val depend. on state
        uint8_t pill_count;
        uint8_t turn_count;
        uint8_t step;
        uint16_t steps_dispense;
        // To EEPROM
        bool calibrated;
        uint32_t timer;
        uart_t* uart;
} Machine_t;

void init_sm(Machine_t *m, state init_state);

/* STATES FUNKTIOT*/
void standby (Machine_t* m, Events_t e);
void check_calibration (Machine_t* m, Events_t e);
void calibrated (Machine_t* m, Events_t e);
void dispense_wait (Machine_t* m, Events_t e);
void dispense_pill (Machine_t* m, Events_t e);
void dispense_fail (Machine_t* m, Events_t e);

#endif
