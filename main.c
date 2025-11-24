#include <stdio.h>
#include "pico/stdlib.h"
#include "states.h"



int main() 
{

        // Init pins here!

        // Init machine here!
        Machine_t mn = { standby, 0 };
        Events_t e = NOP; // Näitä eventtejä sit haetaan tohon state machineen queue tai jonkun kautta

        // Start machine here
        while (true) {
                // Run state machine here
                mn.state(&mn, e);
                sleep_ms(50);
        }
}
