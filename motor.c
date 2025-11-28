#include "metropolia_board.h"
#include "motor.h"
#include "io.h"
#include <math.h>

#include "hardware/gpio.h"
#include "pico/time.h"

#define SLEEP_BETWEEN 3000
#define OFFSET_CORR 144
#define TIMEOUT_TURN 16000
#define WHOLE_TURN 8
#define TURNS 1
#define TIMEOUT_ERR (-1)

static void drive_pins(Machine_t* m)
{
        /* Half-Stepping binary array for motor steps */
        /* IN1 | IN2 | IN3 | IN4 */
        static bool hf_step_m[8][4] = {
                {1, 1, 0, 0},
                {0, 1, 0, 0},
                {0, 1, 1, 0},
                {0, 0, 1, 0},
                {0, 0, 1, 1},
                {0, 0, 0, 1},
                {1, 0, 0, 1},
                {1, 0, 0, 0}
        };

        /* Keep track of motor step state */
        m->step = ++m->step % 8;

        gpio_put(IN1, hf_step_m[m->step][0]);
        gpio_put(IN2, hf_step_m[m->step][1]);
        gpio_put(IN3, hf_step_m[m->step][2]);
        gpio_put(IN4, hf_step_m[m->step][3]);
}

static int turn_until_opt_fall(Machine_t* m)
{
        set_prev_state(gpio_get(OPT_SW_PIN));
        absolute_time_t time;
        time = make_timeout_time_ms(TIMEOUT_TURN);

        int i = 0;
        for (; !is_falling_edge(OPT_SW_PIN); ++i)
        {
                if(time_reached(time))
                        return TIMEOUT_ERR; //TIMEOUT on turn
                sleep_us(SLEEP_BETWEEN);
                drive_pins(m);
        }
        return i;
}

void calibrate(Machine_t* m)
{
        m->calibrated = false; //Set flag on entry to false

        /* Initial orientation */
        if (turn_until_opt_fall(m) == TIMEOUT_ERR)
                return; //Exit fail calib

        /* Calc average */
        uint steps = 0;
        for (int i = 0; i < TURNS; ++i)
        {
                int result = turn_until_opt_fall(m);
                if(result == TIMEOUT_ERR)
                        return; //Exit fail calib
                steps += result;
        }

        //Drive the motor to the middle of the hole, from the edge 144 steps
        for(int i = 0; i < OFFSET_CORR; ++i)
        {
                sleep_us(SLEEP_BETWEEN);
                drive_pins(m);
        }

        m->calibrated = true;
        m->steps_dispense = (steps/TURNS)/WHOLE_TURN;
}

void dispense(Machine_t* m)
{
        for (int i = 0; i < m->steps_dispense; ++i)
        {
                sleep_us(SLEEP_BETWEEN);
                drive_pins(m);
        }
}
