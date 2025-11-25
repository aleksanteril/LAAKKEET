#ifndef UART_H
#define UART_H

#include "hardware/uart.h"
#include "hardware/gpio.h"

#define RX_FIFO_SIZE 32
#define TX_FIFO_SIZE 32

typedef struct uart_t {
        uart_inst_t *inst;
        uint tx_pin;
        uint rx_pin;
} uart_t;

uart_t* init_uart_routine(uint uart_nr, uint baud);

int uart_read_str(uart_t* uart, char* buffer, int size, uint32_t timeout_ms);

#endif //UART_H