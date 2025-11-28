/* TÄHÄN FILEEN STATE FUNKTIOT */
#include "states.h"
#include <stdio.h>
#include "metropolia_board.h"
#include "io.h"
#include "pico/time.h"
#include "motor.h"

#define DISPENSE_TICKS (DISPENSE_INTERVAL*1000/TICK_SLEEP)
#define DISPENSE_FAIL_TICKS (TIME_TO_DISPENSE_FAIL/TICK_SLEEP)

void init_sm(Machine_t *m, state init_state)
{
        m->state = init_state;
        m->state(m, eEnter);
}

static void reset_machine(Machine_t* m)
{
        m->pill_count = 0;
        m->turn_count = 0;
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
                if(++m->timer % 20 == 0)
                        led_toggle(LED_D1_PIN);
                if(m->timer >= 600) //Reminder to fill every 30sec, also incase empty msg was lost
                {
                        m->timer = 0;
                        if(!online())
                                join_lora_network(m->uart, 2);
                        send_msg(m->uart, "REMIND: Dispenser EMPTY");
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
                if(!m->calibrated)
                        printf("Calibration failed.\r\n");
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
                printf("Calibrated with %u steps per dispense.\r\n", m->steps_dispense);
                send_msg(m->uart, "CALIBRATED");
                led_on(LED_D1_PIN);
                break;
        case eExit:
                led_off(LED_D1_PIN);
                break;
        case eTick:
                break;
        case eSW1:
                send_msg(m->uart, "Dispenser START");
                change_state(m, dispense_wait);
                break;
        }
}

void dispense_wait(Machine_t* m, Events_t e)
{
        static absolute_time_t time;
        switch(e)
        {
        case eEnter:
                time = make_timeout_time_ms(DISPENSE_INTERVAL*1000);

                // Status msg
                char msg[42];
                snprintf(msg, 42, "STATUS: disp: %u, turn: %u", m->pill_count, m->turn_count);
                printf("%s\r\n", msg);

                send_msg(m->uart, msg);
                if(!online()) // If message failed, try to connect twice, send again.
                {
                        join_lora_network(m->uart, 2);
                        send_msg(m->uart, msg);
                }

                if(m->turn_count >= 7)
                {
                        send_msg(m->uart, "Dispenser EMPTY");
                        change_state(m, standby);
                }
                break;
        case eExit:
                break;
        case eTick:
                //if(++m->timer >= DISPENSE_TICKS)
                if(time_reached(time))
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
                if(++m->timer >= DISPENSE_FAIL_TICKS)
                        change_state(m, dispense_fail);
                break;
        case ePiezo:
                ++m->pill_count;
                send_msg(m->uart, "Dispense OK");
                change_state(m, dispense_wait);
                break;
        }
}

void dispense_fail(Machine_t* m, Events_t e)
{
        switch(e)
        {
        case eEnter:
                send_msg(m->uart, "Dispense FAIL");
                m->timer = 0;
                break;
        case eExit:
                led_off(LED_D1_PIN);
                break;
        case eTick:
                if(++m->timer % 20 == 0)
                        led_toggle(LED_D1_PIN);
                else if(m->timer >= 200)
                        change_state(m, dispense_wait);
                break;
        }
}
