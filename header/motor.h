#ifndef LAAKKEET_MOTOR_H
#define LAAKKEET_MOTOR_H
#include <pico/types.h>
#include "states.h"


/* Turn motor with specific step count, blocking */
void turn_motor_8th(Machine_t* m, int n);
void turn_motor_full_rot(Machine_t* m);
void calibrate_motor(Machine_t* m);

//void dispense(Machine_t* m);

#endif //LAAKKEET_MOTOR_H