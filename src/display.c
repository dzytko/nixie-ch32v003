#include "display.h"

#include <errno.h>

#include "ch32v003fun.h"


static const uint8_t digit_to_pin[] = {
    PA1, // 0
    PA2, // 1
    PC0, // 2
    PD0, // 3
    PC3, // 4
    PC4, // 5
    PD4, // 6
    PD5, // 7
    PD6, // 8
    PD7, // 9
};


int display_write(const uint8_t digit) {
    if (digit > 9) {
        return -EINVAL;
    }

    display_clear();

    funDigitalWrite(digit_to_pin[digit], 1);

    return 0;
}

int display_clear(void) {
    for (uint8_t i = 0; i < sizeof(digit_to_pin); i++) {
        funDigitalWrite(digit_to_pin[i], 0);
    }

    return 0;
}
