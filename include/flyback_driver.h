#ifndef CH32V003_NIXIE_FLYBACK_DRIVER_H
#define CH32V003_NIXIE_FLYBACK_DRIVER_H

#include <stdint.h>


int flyback_init(void);

int flyback_start(void);

int flyback_stop(void);

int flyback_is_running(void);

int flyback_get_voltage(uint8_t *voltage);

int flyback_get_duty_percent(uint8_t *percent);

int flyback_set_voltage(uint8_t voltage);

#endif // CH32V003_NIXIE_FLYBACK_DRIVER_H