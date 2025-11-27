#include "metropolia_board.h"
#include "uart.h"

#define TIMEOUT_MS 500
#define TIMEOUT_US (TIMEOUT_MS*1000)

static uart_t u1 = {
        .inst = uart1,
        .tx_pin = UART1_TX,
        .rx_pin = UART1_RX
};

static uart_t* get_uart_t(uint uart_nr)
{
        return &u1;
}

uart_t* init_uart_routine(uint uart_nr, uint baud)
{
        uart_t *uart = get_uart_t(uart_nr);

        /*Set gpio funcs to tell hardware that it is uart controlled */
        gpio_set_function(uart->tx_pin, UART_FUNCSEL_NUM(uart->inst, uart->tx_pin));
        gpio_set_function(uart->rx_pin, UART_FUNCSEL_NUM(uart->inst, uart->rx_pin));

        uart_init(uart->inst, baud);

        return uart;
}

/* Read a string from uart (BLOCKING) read until \n encountered,
 * return bytes read, adds null terminator */
int uart_read_str(uart_t* uart, char* buffer, int size, uint32_t timeout_ms)
{
        int bytes = 0;
        while(uart_is_readable_within_us(uart->inst, timeout_ms*1000))
        {
                *buffer = uart_getc(uart->inst);
                ++bytes;

                if(*buffer++ == '\n' || bytes >= size-1)
                        break;
        }
        *buffer = '\0';
        return bytes;
}