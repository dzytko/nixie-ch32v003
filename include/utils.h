#ifndef CH32V003_NIXIE_UTILS_H
#define CH32V003_NIXIE_UTILS_H

#include <stdint.h>

static const float adc_vref = 3.3f;
static const uint8_t divider_ratio = 61;

static inline uint8_t adc_count_to_voltage(const uint16_t adc_value) {
    const float voltage = ((float) adc_value / 1024.0f) * adc_vref * divider_ratio;
    return (uint8_t) voltage;
}

static inline uint16_t voltage_to_adc_count(const uint8_t voltage) {
    const float adc_value = voltage * 1024.0f / divider_ratio / adc_vref;
    return (uint16_t) adc_value;
}

#endif // CH32V003_NIXIE_UTILS_H