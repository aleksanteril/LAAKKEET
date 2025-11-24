/* TÄHÄN FILEEN STATE FUNKTIOT */
#include "states.h"
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
}

static void change_state(Machine_t *m, state next_state)
{
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
                if (++m->timer > 20)
                {
                        m->timer = 0;
                        led_toggle(LED_D1_PIN);
                }
                break;
        case eSW0:
                change_state(m, check_calibration);
        }
}

void check_calibration(Machine_t* m, Events_t e)
{
        switch(e)
        {
        case eEnter:
                //calibrate();
                break;
        case eExit:
                break;
        case eTick:
                //Sitä rataa eteenpäin ehkä
                break;
        }
}
