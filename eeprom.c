#include "eeprom.h"
#include <string.h>
#include "metropolia_board.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/binary_info/code.h"

/* 7bit address as specified,
 * the pico sdk does the R/W bit handling */
#define I2C_ADDR 0x50 // 0b101 000
#define EEPROM_I2C i2c0
#define TIMEOUT_US 8000 // 8ms

/* Uses I2C0 to communicate at ~100khz */
void init_eeprom()
{
        i2c_init(EEPROM_I2C, 100000);

        gpio_set_function(I2C0_SCL, GPIO_FUNC_I2C);
        gpio_set_function(I2C0_SDA, GPIO_FUNC_I2C);

        /* Pull ups are already provided by Crowtail board */
}

static int write_i2c(uint8_t* frame, size_t size)
{
        int result = i2c_write_timeout_us(EEPROM_I2C,I2C_ADDR,
                 frame,size,false,TIMEOUT_US);

        /* On succesful write wait for the eeprom internal write, max 10-5ms */
        if (result && result != PICO_ERROR_GENERIC)
                sleep_ms(10);
        return result;
}

static int read_i2c(uint8_t* addr_frame, uint8_t* dst, size_t size)
{
        /* Select address, then read from */
        int result = i2c_write_timeout_us(EEPROM_I2C,I2C_ADDR, addr_frame,
                2,true,TIMEOUT_US);

        if (result && result != PICO_ERROR_GENERIC)
        {
                result = i2c_read_timeout_us(EEPROM_I2C, I2C_ADDR, dst,
                        size, false, TIMEOUT_US);
        }
        return result;
}

/* We have 256k eeprom so first bit in addr is DONT CARE write as 0 */
int write_byte(uint16_t addr, uint8_t byte)
{
        /* We have to build a frame to send data */
        // [high addr, low addr, data]
        uint8_t frame[3];
        frame[0] = (uint8_t) (addr >> 8);
        frame[1] = (uint8_t) addr;
        frame[2] = byte;

        return write_i2c(frame, sizeof(frame));
}

int read_byte(uint16_t addr, uint8_t* byte)
{
        /* We have to build a frame to send addr */
        // [high addr, low addr]
        uint8_t frame[2];
        frame[0] = (uint8_t) (addr >> 8);
        frame[1] = (uint8_t) addr;

        return read_i2c(frame, byte, 1);
}

int write_page(uint16_t addr, uint8_t* bytes, size_t size)
{
        /* Build the frame, max frame 2 addr + 64 data bytes */
        uint8_t frame[66];
        frame[0] = (uint8_t) (addr >> 8);
        frame[1] = (uint8_t) addr;
        memcpy(frame+2, bytes, size);

        return write_i2c(frame, size+2);
}

int read_page(uint16_t addr, uint8_t* bytes, size_t size)
{
        /* We have to build a frame to send addr */
        // [high addr, low addr]
        uint8_t frame[2];
        frame[0] = (uint8_t) (addr >> 8);
        frame[1] = (uint8_t) addr;

        return read_i2c(frame, bytes, size);
}

// CRC Checksum calculation
uint16_t crc16(const uint8_t *data_p, size_t length)
{
        uint8_t x;
        uint16_t crc = 0xFFFF;

        while (length--)
        {
                x = crc >> 8 ^ *data_p++;
                x ^= x >> 4;
                crc = (crc << 8) ^ ((uint16_t) (x << 12)) ^ ((uint16_t) (x << 5)) ^ ((uint16_t) x);
        }
        return crc;
}

