/* TÄHÄN FILEEN STATE FUNKTIOT */
#include "states.h"
#include <stdio.h>
#include "metropolia_board.h"
#include "io.h"

void init_sm(Machine_t *m, state init_state)
{
        m->state = init_state;
        m->state(m, eEnter);
}

static void reset_machine(Machine_t* m)
{
        m->pill_count = 0;
        m->turn_count = 0;
        m->calibrated = false;
}

static char* get_state_name(state next_state)
{
        if (next_state == calibrated) return "CALIBRATED";
        if (next_state == standby) return "STANDBY";
        if (next_state == check_calibration) return "CHECK_CALIBRATION";
        if (next_state == dispense_wait) return "DISPENSE_WAIT";
        if (next_state == dispense_pill) return "DISPENSE_PILL";
        if (next_state == dispense_fail) return "DISPENSE_FAIL";
        return "UNKNOWN";
}

static void change_state(Machine_t *m, state next_state)
{
        printf("Changing state to %s.\r\n", get_state_name(next_state));
        m->state(m, eExit);
        m->state = next_state;
        m->state(m, eEnter);
}

void standby(Machine_t* m, Events_t e)
{
        switch(e)
        {
        case eEnter:
                m->timer = 0;
                reset_machine(m);
                break;
        case eExit:
                led_off(LED_D1_PIN);
                break;
        case eTick:
                if(++m->timer > 20)
                {
                        m->timer = 0;
                        led_toggle(LED_D1_PIN);
                }
                break;
        case eSW0:
                change_state(m, check_calibration);
                break;
        }
}

void check_calibration(Machine_t* m, Events_t e)
{
        switch(e)
        {
        case eEnter:
                calibrate(m);
                break;
        case eExit:
                break;
        case eTick:
                change_state(m, m->calibrated ? calibrated : standby);
                break;
        }
}

void calibrated(Machine_t* m, Events_t e)
{
        switch(e)
        {
        case eEnter:
                led_on(LED_D1_PIN);
                break;
        case eExit:
                led_off(LED_D1_PIN);
                break;
        case eTick:
                break;
        case eSW1:
                change_state(m, dispense_wait);
                break;
        }
}

void dispense_wait(Machine_t* m, Events_t e)
{
        switch(e)
        {
        case eEnter:
                if(m->turn_count >= 7)
                        change_state(m, standby);
                m->timer = 0;
                break;
        case eExit:
                break;
        case eTick:
                if(++m->timer >= 100)
                {
                        ++m->turn_count;
                        change_state(m, dispense_pill);
                }
                break;
        }
}

void dispense_pill(Machine_t* m, Events_t e)
{
        switch(e)
        {
        case eEnter:
                m->timer = 0;
                dispense(m);
                break;
        case eExit:
                break;
        case eTick:
                if(++m->timer >= 4)
                        change_state(m, dispense_fail);
                break;
        case ePiezo:
                ++m->pill_count;
                change_state(m, dispense_wait);
                break;
        }
}

void dispense_fail(Machine_t* m, Events_t e)
{
        switch(e)
        {
        case eEnter:
                m->timer = 0;
                break;
        case eExit:
                led_off(LED_D1_PIN);
                break;
        case eTick:
                if(++m->timer % 20 == 0)
                        led_toggle(LED_D1_PIN);
                if(m->timer >= 200)
                        change_state(m, dispense_wait);
                break;
        }
}
