#include <math.h>
#include "hardware/gpio.h"
#include "pico/time.h"
#include "metropolia_board.h"
#include "states.h"
#include "motor.h"

//Values for calculating the steps
#define CALIB_SAMPLE_SIZE 2
#define STEPS 8
#define BLOCKS 8
#define THEORETICAL_STEPS_IN_A_REVOLUTION 4096

static void one_step(uint step) {
        static bool cycle[STEPS][4] = {
                {1, 0, 0, 0},
                {1, 1, 0, 0},
                {0, 1, 0, 0},
                {0, 1, 1, 0},
                {0, 0, 1, 0},
                {0, 0, 1, 1},
                {0, 0, 0, 1},
                {1, 0, 0, 1}
        };

        gpio_put(IN1, cycle[step][0]);
        gpio_put(IN2, cycle[step][1]);
        gpio_put(IN3, cycle[step][2]);
        gpio_put(IN4, cycle[step][3]);
        sleep_ms(3);
}

static int step_checker(uint step) {
        //Checks if the steps have made a full circle and returns it back to start
        return step > 7 ? 0 : step;
}

static void full_round(Machine_t *m) {
        //Drives full round (8 steps) so that it stops at step 7 so the driving can start from the step 0
        int start_up_step = -1;
        for (int i = 0; i < STEPS; i++)
        {
                start_up_step = step_checker(++start_up_step);
                one_step(start_up_step);
        }
        m->step_cycle = start_up_step;
}

static bool limit_switch_check()
{
        return !gpio_get(OPT_SW_PIN);
}

static void error_check(int steps_total, bool* error, int revolutions)
{
        if (revolutions <= 0)
        {
                if (steps_total > THEORETICAL_STEPS_IN_A_REVOLUTION * 1.1) {
                        *error = true;
                }
        }
        else
        {
                if (steps_total / revolutions > THEORETICAL_STEPS_IN_A_REVOLUTION * 1.1) {
                        *error = true;
                }
        }
}

static void calibrate_position(Machine_t *m)
{
        int revolutions = -1;
        double steps_total = 0;
        double average_steps_revolution = 0;
        double steps_total_gaps = 0;
        double average_steps_gap = 0;
        double steps_left = 0;
        bool in_gap = false;
        double steps_in_block = 0;
        bool error = false;

        //Find the first rising edge
        while (revolutions == -1 && error == false)
        {        // Voisko tän funktion simplifioida
                m->step_cycle = step_checker(++m->step_cycle);
                one_step(m->step_cycle);
                if (limit_switch_check() && !in_gap)
                {
                        in_gap = true;
                }
                else if (in_gap)
                {
                        if (!limit_switch_check())
                        {
                                in_gap = false;
                                revolutions++;
                        }
                }
                error_check(++steps_total, &error, revolutions);
        }
        steps_total = 1; //Step counter is set to 1 after finding the first rising edge

        //Start the calibration after finding the first rising edge
        while (revolutions < CALIB_SAMPLE_SIZE && !error)
        {
                m->step_cycle = step_checker(++m->step_cycle);
                one_step(m->step_cycle);
                if (limit_switch_check())
                {
                        if (!in_gap)
                        {
                                in_gap = true;
                        }
                        steps_total_gaps++;
                }
                else if (in_gap)
                {
                        in_gap = false;
                        revolutions++;
                }
                error_check(++steps_total, &error, revolutions);
        }

        if (error)
        {
                m->calibrated = false;
                return;
        }

        average_steps_revolution = steps_total / CALIB_SAMPLE_SIZE;
        steps_in_block = average_steps_revolution / BLOCKS;
        average_steps_gap = steps_total_gaps / CALIB_SAMPLE_SIZE;

        //Device "starts seeing" the gap x steps too late and "stops seeing" the gap x steps too early.
        //Subtracting x from full blocks steps the wheel turns in a correct position
        steps_left = steps_in_block - average_steps_gap / 2;
        for (int i = 0; i < round(steps_left); i++)
        {
                m->step_cycle = step_checker(++m->step_cycle);
                one_step(m->step_cycle);
        }
        m->step_in_revolution = (int)round(average_steps_revolution);
        m->calibrated = true;
}

void calibrate(Machine_t *m)
{
        full_round(m);
        calibrate_position(m);
}