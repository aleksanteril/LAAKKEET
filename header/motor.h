#ifndef LAAKKEET_MOTOR_H
#define LAAKKEET_MOTOR_H

#include "machine.h"

/* Turn motor with specific step count, blocking */
void calibrate(Machine_t* m);
void dispense(Machine_t* m);
void re_calibrate(Machine_t* m);
void recall_position(Machine_t* m);

#endif //LAAKKEET_MOTOR_H