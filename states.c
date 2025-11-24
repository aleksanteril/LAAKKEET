/* TÄHÄN FILEEN STATE FUNKTIOT */
#include "states.h"
#include "metropolia_board.h"
#include "utils.h"

static void change_state(Machine_t *m, state next_state)
{
        m->state(m, eExit);
        m->state = next_state;
        m->state(m, eEnter);
}

void standby (Machine_t* m, Events_t e)
{
        switch(e)
        {
        case eEnter:
                reset_machine(m);
                break;
        case eExit:
                led_off(LED_D1_PIN);
                break;
        case eTick:
                led_toggle(LED_D1_PIN);
                break;
        case eSW0:
                change_state(m, check_calibration);
        }
}

void check_calibration (Machine_t* m, Events_t e)
{
        switch(e)
        {
        case eEnter:
                calibrate();
                break;
        case eExit:
                break;
        case eTick:
                //Sitä rataa eteenpäin ehkä
                break;
        }
}
