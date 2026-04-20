#ifndef CH32V003_NIXIE_DISPLAY_H
#define CH32V003_NIXIE_DISPLAY_H

#include <stdint.h>

int display_write(uint8_t digit);

int get_displayed_digit(uint8_t *digit);

int display_clear(void);

#endif // CH32V003_NIXIE_DISPLAY_H