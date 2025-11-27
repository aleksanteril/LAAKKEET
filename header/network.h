#ifndef NETWORK_H
#define NETWORK_H

#include "uart.h"

// CHANGE THIS TO YOUR OWN APP KEY
#define APP_KEY "734ff565033a3af65bef1520bbdc1e99"

bool join_lora_network(uart_t* uart, uint tries);
void send_msg(uart_t* uart, char* msg);
bool online(); //Returns true or false

#endif
