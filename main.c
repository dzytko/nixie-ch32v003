#include <stdlib.h>
#include <stdint.h>

#include "flyback_driver.h"
#include "i2c.h"


int main(void) {
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
