#include <stdlib.h>
#include <stdint.h>

#include "ch32v003fun.h"
#include "flyback_driver.h"
#include "i2c.h"


int main(void) {
    uint32_t intsyscr = __get_INTSYSCR();
    intsyscr |= 1 << 1; // enable interrupt nesting
    __set_INTSYSCR(intsyscr);

    int ret = i2c_init();
    while (ret < 0) {
        (void)0;
    }

    ret = flyback_init();
    while (ret < 0) {
        (void)0;
    }

    while (1) {

    }
}
