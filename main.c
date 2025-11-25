#include <stdio.h>
#include "metropolia_board.h"
#include "pico/stdlib.h"
#include "states.h"
#include "io.h"
#include "pico/util/queue.h"

#define QUEUE_SIZE 30

/* Event queue used in this MAIN_C */
static queue_t event_q;

void check_buttons()
{
        Events_t e_add = NOP;
        if (pressed(SW0_PIN))
                e_add = eSW0;
        else if (pressed(SW1_PIN))
                e_add = eSW1;
        if (e_add != NOP)
                queue_try_add(&event_q, &e_add);
}

static void piezo_irq(uint gpio, uint32_t event_mask)
{
        Events_t e_add = ePiezo;
        queue_try_add(&event_q, &e_add);
}

int main() 
{
        stdio_init_all();

        // Init pins here!
        setup_gpio(LED_D1_PIN, GPIO_OUT);
        setup_gpio(SW0_PIN, GPIO_IN);
        setup_gpio(SW1_PIN, GPIO_IN);
        setup_gpio(PIEZO_SW_PIN, GPIO_IN);

        /* Init irq routine */
        gpio_set_irq_enabled_with_callback(PIEZO_SW_PIN, GPIO_IRQ_EDGE_FALL, true, piezo_irq);

        // Init queue
        queue_init(&event_q, sizeof(Events_t), QUEUE_SIZE);
        Events_t e;

        // Init machine here!
        Machine_t mn;
        init_sm(&mn, standby);

        printf("Boot up\r\n");
        // Start machine here
        while (true)
        {
                // Poll buttons
                check_buttons();

                // Run state machine here
                while(queue_try_remove(&event_q, &e))
                        mn.state(&mn, e);
                mn.state(&mn, eTick);
                sleep_ms(TICK_SLEEP);
        }
}

#define DEBUG
#ifdef DEBUG
void dispense(Machine_t* m)
{
        printf("Dispensing pill turn %d.\r\n", m->turn_count);
        sleep_ms(5000);
        printf("Pill dispensed.\r\n");
}

void calibrate(Machine_t* m)
{
        printf("Calibrating machine.\r\n");
        sleep_ms(5000);
        m->calibrated = true;
        printf("Machine calibrated.\r\n");
}
#endif
