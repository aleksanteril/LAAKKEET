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

typedef enum { BACKWARD = 0, FORWARD = 1 } Direction;

static void drive_pins(Machine_t* m, Direction dir)
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
        if (dir == FORWARD)
                m->step = ++m->step % 8;
        else
                m->step = --m->step % 8;

        sleep_us(SLEEP_BETWEEN);
        gpio_put(IN1, hf_step_m[m->step][0]);
        gpio_put(IN2, hf_step_m[m->step][1]);
        gpio_put(IN3, hf_step_m[m->step][2]);
        gpio_put(IN4, hf_step_m[m->step][3]);
}

static int turn_until_opt_fall(Machine_t* m, Direction dir)
{
        set_prev_state(gpio_get(OPT_SW_PIN));
        absolute_time_t time;
        time = make_timeout_time_ms(TIMEOUT_TURN);

        int i = 0;
        for (; !is_falling_edge(OPT_SW_PIN); ++i)
        {
                if(time_reached(time))
                        return TIMEOUT_ERR; //TIMEOUT on turn
                drive_pins(m, dir);
        }
        return i;
}

void correct_offset(Machine_t* m, Direction dir)
{
        for(int i = 0; i < OFFSET_CORR; ++i)
        {
                drive_pins(m, dir);
        }
}

void calibrate(Machine_t* m)
{
        m->calibrated = false; //Set flag on entry to false

        /* Initial orientation */
        if (turn_until_opt_fall(m, FORWARD) == TIMEOUT_ERR)
                return; //Exit fail calib

        /* Calc average */
        uint steps = 0;
        for (int i = 0; i < TURNS; ++i)
        {
                int result = turn_until_opt_fall(m, FORWARD);
                if(result == TIMEOUT_ERR)
                        return; //Exit fail calib
                steps += result;
        }

        //Drive the motor to the middle of the hole, from the edge 144 steps
        correct_offset(m, FORWARD);

        m->calibrated = true;
        m->steps_dispense = (steps/TURNS)/WHOLE_TURN;
}

void dispense(Machine_t* m)
{
        for (int i = 0; i < m->steps_dispense; ++i)
        {
                drive_pins(m, FORWARD);
        }
}

void recall_position(Machine_t* m)
{
        //Drive the motor until the last empty block on top of the hole
        for (int i = 0; i < m->turn_count; ++i)
        {
                dispense(m);
        }
        m->calibrated = true;;
}

void re_calibrate(Machine_t* m)
{
        m->calibrated = false;
        int steps_taken = 0;
        //Drives backwards until the falling edge is found
        steps_taken = turn_until_opt_fall(m, BACKWARD);

        if (steps_taken == TIMEOUT_ERR) //|| steps_taken > m->steps_dispense * m-> turn_count + 5
                return; //Re-calibration failed. Reset the device

        correct_offset(m, BACKWARD);
}