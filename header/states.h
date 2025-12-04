#ifndef STATES_H
#define STATES_H

#include "machine.h"

// Time between eTick events in (ms) milliseconds
#define TICK_SLEEP 50
// Time to determine dispense fail a.k.a no piezo event detected until this time. (ms) milliseconds
#define TIME_TO_DISPENSE_FAIL 200
// Time of wait between dispensing the medicine in (s) seconds
#define DISPENSE_INTERVAL 30

void init_sm(Machine_t *m, state init_state);

/* STATES FUNKTIOT*/
void standby (Machine_t* m, Events_t e);
void check_calibration (Machine_t* m, Events_t e);
void calibrated (Machine_t* m, Events_t e);
void dispense_wait (Machine_t* m, Events_t e);
void dispense_pill (Machine_t* m, Events_t e);
void dispense_fail (Machine_t* m, Events_t e);
void dispense_ok(Machine_t* m, Events_t e);
void recalibrate(Machine_t* m, Events_t e);

#endif
