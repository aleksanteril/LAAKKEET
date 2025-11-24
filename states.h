#ifndef STATES_H
#define STATES_H

#include "pico/types.h"

typedef struct Machine_t Machine_t;
typedef void (*state) (Machine_t* m, Events_t e);
typedef struct Machine_t {
        state state;
        uint pill_count;
} Machine_t;

typedef enum Events_t {
        eEnter,
        eExit,
        eTick,
        eSW0,
        eSW1,
        ePiezo,
        NOP
} Events_t;

/* STATES FUNKTIOT*/
void standby (Machine_t* m, Events_t e);
void check_calibration (Machine_t* m, Events_t e);

#endif
