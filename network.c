#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include "network.h"


#define CMD_TEST_COMMS "AT\r\n"
#define CMD_READ_FW "AT+VER\r\n"
#define CMD_READ_DEVEUI "AT+ID=DevEui\r\n"

// Join routine commands
#define CMD_MODE "AT+MODE=LWOTAA\r\n"
#define CMD_KEY "AT+KEY=APPKEY,\"734ff565033a3af65bef1520bbdc1e99\"\n"
#define CMD_CLASS "AT+CLASS=A\r\n"
#define CMD_PORT "AT+PORT=8\r\n"
#define CMD_JOIN "AT+JOIN\r\n"

//#define CMD_SEND_MSG "+MSG=\"text message\"\r\n"

#define MAX_TRIES 5
#define TIMEOUT_MS 5000
#define MAX_RES 40
#define ROUTINE_LEN 5

static bool network_join_routine(uart_t* uart);

bool join_lora_network(uart_t* uart)
{
        printf("PICO: Trying to connect to LoRa ...\r\n");

        for(int i = 0; i < MAX_TRIES; ++i)
        {
                printf("PICO: Try to connect %d times ... \r\n", i+1);
                uart_puts(uart->inst, CMD_TEST_COMMS);

                char result[MAX_RES];
                if (uart_read_str(uart, result, MAX_RES, TIMEOUT_MS))
                {
                        /* if Good response, set state to next and early return */
                        if(!strcmp(result, "+AT: OK\r\n"))
                        {
                                printf("PICO: Connected to LoRa module.\r\n");
                                if (network_join_routine(uart))
                                        return true;
                        }
                }
        }
        /* Fail if break loop on i cond */
        printf("PICO: LORA Module not responding.\r\n");
        return false;
}

static bool network_join_routine(uart_t* uart)
{
        char* cmds_seq[ROUTINE_LEN] = {CMD_MODE, CMD_KEY, CMD_CLASS, CMD_PORT, CMD_JOIN};

        for(int i = 0; i < ROUTINE_LEN; ++i)
        {
                uart_puts(uart->inst, cmds_seq[i]);
                char result[MAX_RES];
                if (uart_read_str(uart, result, MAX_RES, TIMEOUT_MS))
                        printf("%s", result);
                else
                        return false;
        }
        return true;
}