#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include "network.h"

// Module testing commands
#define CMD_TEST_COMMS "AT\r\n"
#define CMD_READ_FW "AT+VER\r\n"
#define CMD_READ_DEVEUI "AT+ID=DevEui\r\n"

// Join routine commands
#define CMD_MODE "AT+MODE=LWOTAA\r\n"
#define CMD_KEY "AT+KEY=APPKEY,\"" APP_KEY "\"\r\n"
#define CMD_CLASS "AT+CLASS=A\r\n"
#define CMD_PORT "AT+PORT=8\r\n"
#define CMD_JOIN "AT+JOIN\r\n"

// Send MSG when joined
#define CMD_SEND_MSG "AT+MSG=\"%s\"\r\n"

// JOIN AND MSG CONFIRM MSGS
#define MSG_DONE "+MSG: Done\r\n"
#define JOIN_DONE "+JOIN: Done\r\n"
#define JOIN_CONFIRM_PREFIX "+JOIN: NetID"
#define MODULE_OK "+AT: OK\r\n"

#define MAX_TRIES 5
#define TIMEOUT_MS 1000
#define MED_TIMEOUT_MS 5000
#define LONG_TIMEOUT_MS 15000
#define MAX_RES 64
#define PAYLOAD_SIZE 128
#define ROUTINE_LEN 4

static bool network_join_routine(uart_t* uart);
static bool is_online = false;

bool join_lora_network(uart_t* uart)
{
        printf("PICO: Trying to connect to LoRa ...\r\n");

        for(int i = 0; i < MAX_TRIES; ++i)
        {
                printf("PICO: Try to connect %d times ... \r\n", i+1);
                uart_puts(uart->inst, CMD_TEST_COMMS);

                char result[MAX_RES];
                if(uart_read_str(uart, result, MAX_RES, TIMEOUT_MS))
                {
                        if(!strcmp(result, MODULE_OK))
                        {
                                printf("PICO: Connected to LoRa module.\r\n");
                                if(network_join_routine(uart))
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
        char* cmds_seq[ROUTINE_LEN] = {CMD_MODE, CMD_KEY, CMD_CLASS, CMD_PORT};
        char result[MAX_RES] = {};

        //Setup AT module with the commands
        for(int i = 0; i < ROUTINE_LEN; ++i)
        {
                uart_puts(uart->inst, cmds_seq[i]);
                if(!uart_read_str(uart, result, MAX_RES, TIMEOUT_MS))
                        return false;
                printf("%s", result);
        }

        //Send JOIN msg, and wait upto 15s
        uart_puts(uart->inst, CMD_JOIN);

        while(strcmp(result, JOIN_DONE))
        {
                if(!uart_read_str(uart, result, MAX_RES, LONG_TIMEOUT_MS))
                        break; //TIMEOUT NO BYTES, COMM DOWN?

                printf("%s", result);
                if(strstr(result, JOIN_CONFIRM_PREFIX))
                        is_online = true;
        }
        return is_online;
}

uint32_t time_s_32()
{
        return time_us_64() / 1000000;
}


void send_msg(uart_t* uart, char* msg)
{
        if(!is_online || msg == NULL)
                return; //Silent fail if offline

        char payload[PAYLOAD_SIZE];
        int count = snprintf(payload, PAYLOAD_SIZE, "AT+MSG=\"%u: %s\"\r\n", time_s_32(), msg);
        if(count < 0)
        {
                printf("Error formatting MSG.\r\n");
                return;  
        }
        else if(count >= PAYLOAD_SIZE)
        {
                printf("MSG Didn't fit, not sending.\r\n");
                return;
        }

        uart_puts(uart->inst, payload);
        printf("%s", payload);

        char result[MAX_RES] = {};
        while(strcmp(result, MSG_DONE))
        {
                if (!uart_read_str(uart, result, MAX_RES, MED_TIMEOUT_MS))
                {
                        is_online = false;
                        break; //TIMEOUT NO BYTES, COMM DOWN?
                }
                printf("%s", result);
        }
}
