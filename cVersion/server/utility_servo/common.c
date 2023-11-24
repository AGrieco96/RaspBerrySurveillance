#include <stdio.h>
#include <stdlib.h>

#include "gpio.h"
#include "common.h"

int gpio_mode = BOARD;
unsigned int gpio;
const int pin_to_gpio_rev1[41] = {-1, -1, -1, 0, -1, 1, -1, 4, 14, -1, 15, 17, 18, 21, -1, 22, 23, -1, 24, 10, -1, 9, 25, 11, 8, -1, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
const int pin_to_gpio_rev2[41] = {-1, -1, -1, 2, -1, 3, -1, 4, 14, -1, 15, 17, 18, 27, -1, 22, 23, -1, 24, 10, -1, 9, 25, 11, 8, -1, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
const int pin_to_gpio_rev3[41] = {-1, -1, -1, 2, -1, 3, -1, 4, 14, -1, 15, 17, 18, 27, -1, 22, 23, -1, 24, 10, -1, 9, 25, 11, 8, -1, 7, -1, -1, 5, -1, 6, 12, 13, -1, 19, 16, 26, 20, -1, 21 };
int setup_error = 0;
int module_setup = 0;

int check_gpio_priv(void){
    // check module has been imported cleanly
    if (setup_error)
    {
        fprintf(stderr, "Module not imported correctly!\n" );
        return 1;
    }

    // check mmap setup has worked
    if (!module_setup)
    {
        fprintf(stderr, "No access to /dev/mem.  Try running as root!\n" );
        return 2;
    }
    return 0;
}

int get_gpio_number(int channel, unsigned int *gpio){
	// check channel number is in range
    if ( (gpio_mode == BOARD && (channel < 1 || channel > 26) && rpiinfo.p1_revision != 3)
      		|| (gpio_mode == BOARD && (channel < 1 || channel > 40) && rpiinfo.p1_revision == 3)){
        fprintf(stderr, "The channel sent is invalid on a Raspberry Pi\n" );
        return 4;
    }
    if (*(pin_to_gpio_rev1+channel) == -1){
        fprintf(stderr, "The channel sent is invalid on a Raspberry Pi\n");
        return 5;
    } else {
        *gpio = *(pin_to_gpio_rev1+channel);
    }

}





