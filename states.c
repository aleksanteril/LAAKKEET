/* TÄHÄN FILEEN STATE FUNKTIOT */
#include <stdio.h>
#include "pico/time.h"

#include "states.h"
#include "metropolia_board.h"
#include "io.h"
#include "motor.h"
#include "network.h"
#include "save.h"

#define DISPENSE_TICKS (DISPENSE_INTERVAL*1000/TICK_SLEEP)
#define DISPENSE_FAIL_TICKS (TIME_TO_DISPENSE_FAIL/TICK_SLEEP)

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
        if (next_state == recalibrate) return "RECALIBRATE";
        return "UNKNOWN";
}

void init_sm(Machine_t *m, state init_state)
{
        if(init_state == dispense_pill)
        {
                printf("Dispenser shutdown while turning, starting from: RECALIBRATE\r\n");
                send_msg(m->uart, "Power lost on dispense");
                m->state = recalibrate;
        }
        else
        {
                printf("Starting from state: %s.\r\n", get_state_name(init_state));
                m->state = init_state;
        }
        char msg[32];
        snprintf(msg, 32, "Start from %s", get_state_name(m->state));
        send_msg(m->uart, msg);
        m->state(m, eEnter);
}

static void change_state(Machine_t *m, state next_state)
{
        printf("Changing state to %s.\r\n", get_state_name(next_state));
        m->state(m, eExit);

        m->state = next_state;
        save_machine(m);
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
                if(m->timer >= 600) //Reminder to fill every ~30sec, also incase empty msg was lost
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
                break;
        case eTick:
                if(m->calibrated)
                        change_state(m, calibrated);
                else
                {
                        printf("Calibration failed.\r\n");
                        send_msg(m->uart, "CALIB FAILED");
                        change_state(m, standby);
                }
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
                        change_state(m, dispense_pill);
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
                ++m->turn_count;
                break;
        case eExit:
                break;
        case eTick:
                if(++m->timer >= DISPENSE_FAIL_TICKS)
                        change_state(m, dispense_fail);
                break;
        case ePiezo:
                ++m->pill_count;
                change_state(m, dispense_ok);
                break;
        }
}

void dispense_ok(Machine_t* m, Events_t e)
{
        switch(e)
        {
        case eEnter:
                send_msg(m->uart, "Dispense OK");
                break;
        case eExit:
                break;
        case eTick:
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

void recalibrate(Machine_t* m, Events_t e)
{
        switch(e)
        {
        case eEnter:
                re_calibrate(m);
                if (m->calibrated == false)
                {
                        send_msg(m->uart, "RECALIB FAILED");
                        change_state(m, standby);
                }
                recall_position(m);
                send_msg(m->uart, "RECALIBRATED");
                break;
        case eExit:
                break;
        case eTick:
                change_state(m, dispense_pill);
                break;
        }
}
