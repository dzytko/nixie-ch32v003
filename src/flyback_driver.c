#include "flyback_driver.h"

#include <sys/errno.h>

#include "ch32v003fun.h"
#include "pid.h"
#include "utils.h"


static const uint16_t max_duty = 15;

struct pid_pid flyback_pid;
uint16_t adc_count_setpoint;
uint16_t current_duty;


static void calc_presc_reload(
    uint32_t *target_frequency,
    uint32_t *target_resolution,
    uint16_t *prescaler,
    uint16_t *reload_value
) {
    const uint32_t cpu_freq = 48 * 1000 * 1000;
    *reload_value = cpu_freq / *target_frequency;
    *prescaler = *target_resolution / (1 << 16);

    *target_frequency = cpu_freq / ((*prescaler + 1) * *reload_value);
    *target_resolution = *reload_value;
}

static int init_adc(void) {
    RCC->APB2PCENR |=
        RCC_APB2Periph_ADC1 | // enable ADC
        RCC_APB2Periph_GPIOD; // enable GPIOD

    funPinMode(PD3, GPIO_CFGLR_IN_ANALOG); // CH4


    // enable DMA
    // RCC->AHBPCENR |= RCC_AHBPeriph_DMA1;

    // Reset the ADC to init all regs
    RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
    RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;

    // reset the DMA to init all regs
    // RCC->APB2PRSTR |= RCC_AHBPeriph_DMA1;
    // RCC->APB2PRSTR &= ~RCC_AHBPeriph_DMA1;

    // reset calibration
    ADC1->CTLR2 |= ADC_RSTCAL;
    while (ADC1->CTLR2 & ADC_RSTCAL) {
    }

    // calibrate
    ADC1->CTLR2 |= ADC_CAL | ADC_ADON;
    while (ADC1->CTLR2 & ADC_CAL) {
    }
    ADC1->CTLR2 &= ~ADC_ADON;

    ADC1->CTLR1 |=
        ADC_SCAN | // scan mode
        ADC_JAUTO; // automatic injected group conversion


    // enable end of conversion interrupt
    ADC1->CTLR1 |= ADC_EOCIE;


#define SAMPLE_TIME (0x7)  // 241 cycles
    ADC1->SAMPTR2 =
            SAMPLE_TIME << (4 * 3);

    ADC1->RSQR1 |= 0x0 << 20; // L = 1 conversion
    ADC1->RSQR3 |= 4 << 0;

    // i have no idea why i can only start that stupid adc via sw trigger,
    // according to datasheet it should start on ADC_ON, but doesn't, so here we are
    ADC1->CTLR2 |=
        ADC_CONT | // continuous conversion mode
        ADC_EXTTRIG | // enable external trigger
        ADC_EXTSEL_0 | ADC_EXTSEL_1 | ADC_EXTSEL_2; // set SWSTART as trigger source

    NVIC_EnableIRQ(ADC_IRQn);
    NVIC_SetPriority(ADC_IRQn, 2 << 6);

    return 0;
}

static int init_timer(void) {
    RCC->APB2PCENR |=
            RCC_APB2Periph_GPIOD |
            RCC_APB2Periph_TIM1 |
            RCC_APB2Periph_AFIO;

    funPinMode(PD2, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF);

    // reset TIM1 to init all regs
    RCC->APB2PRSTR |= RCC_APB2Periph_TIM1;
    RCC->APB2PRSTR &= ~RCC_APB2Periph_TIM1;

    // CTLR1: default is up, events generated, edge align
    TIM1->CTLR1 = 0;

    // CTLR2: set output idle states (MOE off) via OIS1 and OIS1N bits
    TIM1->CTLR2 = 0;

    // SMCFGR: default clk input is 48MHz CK_INT
    uint32_t resolution = (1 << 16) - 1;
    uint16_t prescaler = 0;
    uint16_t reload_value = 0;
    uint32_t pwm_frequency = 100 * 1000;
    calc_presc_reload(&pwm_frequency, &resolution, &prescaler, &reload_value);
    TIM1->PSC = prescaler;
    TIM1->ATRLR = reload_value - 1;

    // reload immediately
    TIM1->SWEVGR |= TIM_UG;

    // enable compare on channel 1
    TIM1->CCER |= TIM_CC1E;

    // CH1 Mode is output, PWM1 (CC1S = 00, OC1M = 110)
    TIM1->CHCTLR1 |= TIM_OC1M_2 | TIM_OC1M_1;

    // set the Capture Compare Register value to 0 initially
    TIM1->CH1CVR = 0;

    // enable TIM1
    TIM1->CTLR1 |= TIM_CEN;

    return 0;
}


__attribute__((interrupt)) void ADC1_IRQHandler(void) {
    const uint16_t adc_count = ADC1->RDATAR & 0xFFF;

    current_duty = (uint16_t) pid_update(
        &flyback_pid,
        adc_count_setpoint,
        adc_count
    );

    if (current_duty > max_duty) {
        current_duty = max_duty;
    }

    TIM1->CH1CVR = current_duty;
}


int flyback_init(void) {
    int res = init_adc();
    if (res < 0) {
        return res;
    }

    res = init_timer();
    if (res < 0) {
        return res;
    }

    flyback_stop();

    flyback_pid.k_p = 0.1f;
    flyback_pid.k_i = 0.001f;
    flyback_pid.k_d = 0.000f;

    flyback_pid.tau = 1;

    flyback_pid.lim_min = 0;
    flyback_pid.lim_max = max_duty;

    flyback_pid.lim_min_int = 0;
    flyback_pid.lim_max_int = max_duty;

    flyback_pid.t = 1;

    return 0;
}

int flyback_start(void) {
    pid_init(&flyback_pid);
    TIM1->BDTR |= TIM_MOE;
    ADC1->CTLR2 |= ADC_SWSTART | ADC_ADON;
    return 0;
}

int flyback_stop(void) {
    TIM1->BDTR &= ~TIM_MOE;
    ADC1->CTLR2 &= ~(ADC_SWSTART | ADC_ADON);
    return 0;
}

int flyback_is_running(void) {
    return (TIM1->BDTR & TIM_MOE) != 0;
}

int flyback_get_voltage(uint8_t *voltage) {
    *voltage = adc_count_to_voltage(adc_count_setpoint);
    return 0;
}

int flyback_get_duty_percent(uint8_t *percent) {
    *percent = (uint8_t) ((float) current_duty / max_duty * 100);
    return 0;
}

int flyback_set_voltage(const uint8_t voltage) {
    adc_count_setpoint = voltage_to_adc_count(voltage);
    return 0;
}