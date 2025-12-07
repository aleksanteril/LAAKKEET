#include "logger.h"
#include <stdio.h>
#include <string.h>
#include "eeprom.h"
#include "pico/error.h"

// The log memory area Start - End
#define LOG_START_INDEX 0 // 0x0
#define LOG_LAST_INDEX (LOG_SIZE-1) // 0x20
#define LOG_SIZE 32
#define ADDR_OFFS 64
#define MAX_ENTRY (MAX_LOG_MSG-1) // 61 - null term
#define MAX_DATA ADDR_OFFS

static int write_index = LOG_START_INDEX;
void init_auto_index_log()
{
        /* On power up set up log pointer by reading logs till invalid */
        int result;
        do
        {
                result = read_log_entry(write_index, NULL, 0);
                if (result > 0) ++write_index;
        }
        while(result > 0);
}

/* Write using automatic running index between the first and last indexes */
int write_log(char *msg)
{
        if (write_index > LOG_LAST_INDEX)
                erase_log();
        return write_log_entry(write_index++, msg);
}

void dump_log()
{
        char msg[MAX_ENTRY+1];
        int i;
        for(i = 0; i < LOG_SIZE; ++i)
        {
                int result = read_log_entry(i, msg, MAX_ENTRY+1);
                if (result > 0)
                        printf("[LOG INDEX: %d] %s\r\n", i, msg);
                else // If invalid detected
                        break;
        }
        if (i == 0)
                printf("[LOG INFO] Nothing to read\r\n");
}

/* Erase whole log */
bool erase_log()
{
        for(int i = 0; i < LOG_SIZE; ++i)
                if (erase_log_entry(i) != 3) return false;
        write_index = 0;
        return true;
}

// Log entry max size 64 bytes, [61 char, \0, 2 bytes CRC]
int write_log_entry(uint index, char* msg)
{
        size_t c_size = strlen(msg);
        if (c_size > MAX_ENTRY || c_size < 1 )
                return 0;

        size_t d_size = c_size+1;

        /* Build the data frame */
        uint8_t data[MAX_DATA];
        memcpy(data, msg, d_size);

        /* Add CRC */
        uint16_t crc = crc16(data, d_size);
        data[d_size++] = (uint8_t) (crc >> 8);
        data[d_size++] = (uint8_t) crc;

        return write_page(index*ADDR_OFFS, data, d_size);
}

int read_log_entry(uint index, char* dest, size_t size)
{
        uint8_t data[MAX_DATA];

        int result = read_page(index*ADDR_OFFS, data, MAX_DATA);
        if (!result || result == PICO_ERROR_GENERIC)
                return result;

        /* Validate CRC, + null, crc_msb, crc_lsb */
        size_t d_size = strlen((char*) data)+1;
        if (crc16(data, d_size+2) != 0)
                return 0;

        /* For validation during startup no need to copy str*/
        if (dest == NULL)
                return result;

        /* Parse back to string if it fits the dest*/
        if (d_size > size)
                return 0;
        memcpy(dest, data, d_size);
        return result;
}

int erase_log_entry(uint index)
{
        return write_byte(index*ADDR_OFFS, 0x0);
}