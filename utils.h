#ifndef UTILS_H
#define UTILS_H

void reset_machine(Machine_t* m);

void led_off(uint pin):
void led_on(uint pin);
void led_toggle(uint pin);

void setup_gpio(uint pin, bool out);
void setup_gpio_nopull(uint pin, bool out);

void set_prev_state(bool st);
bool is_falling_edge(uint pin);

bool pressed(uint pin);
bool released(uint pin);


#endif
