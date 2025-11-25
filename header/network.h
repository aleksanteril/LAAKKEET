#ifndef NETWORK_H
#define NETWORK_H

#include "uart.h"

bool join_lora_network(uart_t* uart);
void send_msg(uart_t* uart, char* msg, ...);

#endif