#include "pico/stdlib.h"
#include "io.h"

void led_off(uint pin)
{
        gpio_put(pin, 0);
}

void led_on(uint pin)
{
        gpio_put(pin, 1);
}

void led_toggle(uint pin)
{
        gpio_put(pin, !gpio_get(pin));
}

void setup_gpio(uint pin, bool out)
{
        setup_gpio_nopull(pin, out);
        if(out == GPIO_IN)
                gpio_pull_up(pin);      
}

void setup_gpio_nopull(uint pin, bool out)
{
        gpio_init(pin);
        gpio_set_dir(pin, out);
}

/* Need to set prev_state before,
 * starting to poll gpio for falling edge! */
static bool prev_state = false;
void set_prev_state(bool st)
{
        prev_state = st;
}

bool is_falling_edge(uint pin)
{
        bool state = gpio_get(pin);
        if(state != prev_state && !state)
                return true;
        prev_state = state;
        return false;
}

/* Checks for if button is pressed, returns constant true */
bool pressed(uint pin)
{
        bool normal = gpio_is_pulled_up(pin);
        return gpio_get(pin) != normal;
}

