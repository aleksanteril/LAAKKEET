#ifndef LAAKKEET_MOTOR_H
#define LAAKKEET_MOTOR_H
#include "states.h"


/* Turn motor with specific step count, blocking */
void calibrate(Machine_t* m);
void dispense(Machine_t* m);

#endif //LAAKKEET_MOTOR_H