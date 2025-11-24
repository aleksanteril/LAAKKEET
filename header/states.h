#ifndef STATES_H
#define STATES_H

#include "pico/types.h"

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
        state state;
        uint8_t pill_count;
        uint32_t timer;
} Machine_t;

void init_sm(Machine_t *m, state init_state);

/* STATES FUNKTIOT*/
void standby (Machine_t* m, Events_t e);
void check_calibration (Machine_t* m, Events_t e);

#endif
