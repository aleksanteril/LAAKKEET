#include "save.h"
#include "eeprom.h"
#include "states.h"

/* We will use the last 64byte page for save data (0x7fc0 - 0x7fff)
 * save all data with CRC signature, that area is reserved for that purpose */
#define SAVE_ADDR (MAX_ADDR-63)
#define DATA_SIZE 7

typedef enum state_hex_t{
        HEX_INVALID = 0x0,
        HEX_STANDBY = 0x1,
        HEX_CHECK_CALIBRATION = 0x2,
        HEX_CALIBRATED = 0x3,
        HEX_DISPENSE_WAIT = 0x4,
        HEX_DISPENSE_PILL = 0x5,
        HEX_DISPENSE_FAIL = 0x6,
        HEX_RECALIBRATE = 0x7
} state_hex_t;

static uint8_t state_to_hex(state state)
{
        if (state == calibrated) return HEX_STANDBY;
        if (state == standby) return HEX_CHECK_CALIBRATION;
        if (state == check_calibration) return HEX_CALIBRATED;
        if (state == dispense_wait) return HEX_DISPENSE_WAIT;
        if (state == dispense_pill) return HEX_DISPENSE_PILL;
        if (state == dispense_fail) return HEX_DISPENSE_FAIL;
        if (state == recalibrate) return HEX_RECALIBRATE;
        return HEX_INVALID;
}

static state hex_to_state(uint8_t hex)
{
        switch (hex)
        {
        case HEX_STANDBY: return calibrated;
        case HEX_CHECK_CALIBRATION: return standby;
        case HEX_CALIBRATED: return check_calibration;
        case HEX_DISPENSE_WAIT: return dispense_wait;
        case HEX_DISPENSE_PILL: return dispense_pill;
        case HEX_DISPENSE_FAIL: return dispense_fail;
        case HEX_RECALIBRATE: return recalibrate;
        default: return standby;
        }
}

void init_eeprom(Machine_t* m)
{
        init_i2c();
}

/* To EEPROM size is 5 bytes + crc (2bytes)
state state; //turned into hex uint8_t
uint8_t pill_count;
uint8_t turn_count;
uint16_t steps_dispense;
*/

void save_machine(Machine_t* m)
{
        // Build the save frame
        uint8_t data[DATA_SIZE] = {};
        data[0] = state_to_hex(m->state);
        data[1] = m->pill_count;
        data[2] = m->turn_count;
        data[3] = (uint8_t) (m->steps_dispense >> 8); //MSB
        data[4] = (uint8_t) (m->steps_dispense & 0xFF); //LSB

        /* Add CRC */
        uint16_t crc = crc16(data, 5);
        data[5] = (uint8_t) (crc >> 8);
        data[6] = (uint8_t) crc;

        write_page(SAVE_ADDR, data, DATA_SIZE);
}

bool load_machine(Machine_t* m)
{
        uint8_t data[DATA_SIZE] = {};
        read_page(SAVE_ADDR, data, DATA_SIZE);

        if (crc16(data, DATA_SIZE) != 0)
                return false; //Not valid save frame

        // Reload the save frame data to struct
        m->state = hex_to_state(data[0]);
        m->pill_count = data[1];
        m->turn_count = data[2];
        m->steps_dispense = (uint16_t) data[3] << 8; //MSB
        m->steps_dispense |= (uint16_t) data[4]; //LSB
        return true;
}