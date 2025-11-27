#include "metropolia_board.h"
#include "motor.h"
#include "io.h"
#include <math.h>
#include "hardware/gpio.h"
#include "pico/time.h"

#define SLEEP_BETWEEN 3000
#define WHOLE_TURN 8
#define TURNS 1

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

static uint turn_until_opt_fall(Machine_t* m)
{
        set_prev_state(gpio_get(OPT_SW_PIN));

        uint i = 0;
        for (; !is_falling_edge(OPT_SW_PIN); ++i)
        {
                sleep_us(SLEEP_BETWEEN);
                drive_pins(m);
        }
        return i;
}

/* n = how many 1/8th revolutions */
void turn_motor_8th(Machine_t* m, int n)
{
        for (int i = 0; i < m->steps_per_turn*n; ++i)
        {
                sleep_us(SLEEP_BETWEEN);
                drive_pins(m);
        }
}

void calibrate(Machine_t* m)
{
        /* Initial orientation */
        turn_until_opt_fall(m);

        /* Calc average */
        uint steps = 0;
        for (int i = 0; i < TURNS; ++i)
                steps += turn_until_opt_fall(m);

        //Drive the motor to the middle of the hole, from the edge 128 steps
        for(int i = 0; i < 128; ++i)
        {
                sleep_us(SLEEP_BETWEEN);
                drive_pins(m);
        }

        m->calibrated = true;
        m->steps_per_turn = steps / TURNS;
}
