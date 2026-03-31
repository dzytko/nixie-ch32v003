#include "i2c.h"

#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <stdbool.h>

#include "ch32v003fun.h"
#include "version.h"
#include "flyback_driver.h"


// TODO
#define I2C_BASE_ADDRESS 0x10 // 0b0010xxx  , bits xxx are set by gpio


typedef void (*register_read_handler_t)(uint8_t *value);

typedef void (*register_write_handler_t)(uint8_t value);


static void sw_version_major_register_read(uint8_t *value);

static void sw_version_minor_register_read(uint8_t *value);

static void sw_version_patch_register_read(uint8_t *value);

static void display_register_read(uint8_t *value);

static void display_register_write(uint8_t value);

static void voltage_register_read(uint8_t *value);

static void voltage_register_write(uint8_t value);

static void duty_register_read(uint8_t *value);

static void flyback_enable_register_read(uint8_t *value);

static void flyback_enable_register_write(uint8_t value);


struct register_handlers {
    register_read_handler_t read;
    register_write_handler_t write;
};

struct i2c_slave_state {
    bool first_write;
    uint8_t offset;
};


static struct register_handlers register_handlers[7] = {
    {sw_version_major_register_read, NULL},
    {sw_version_minor_register_read, NULL},
    {sw_version_patch_register_read, NULL},
    {display_register_read, display_register_write},
    {voltage_register_read, voltage_register_write},
    {duty_register_read, NULL},
    {flyback_enable_register_read, flyback_enable_register_write},
};

static struct i2c_slave_state i2c_slave_state;


static int get_address(uint8_t *value) {
    uint8_t address = 0;
    address |= GPIOC->INDR & 0x07 << 5;
    *value = (I2C_BASE_ADDRESS & ~0x07) | address;
    return 0;
}

static int register_read_callback(uint8_t address) {
    if (address >= sizeof(register_handlers) / sizeof(register_handlers[0])) {
        I2C1->DATAR = 0;
        return -ENOMEM;
    }

    if (!register_handlers[address].read) {
        I2C1->DATAR = 0;
        return -ENOTSUP;
    }

    uint8_t value;
    register_handlers[address].read(&value);
    I2C1->DATAR = value;
    return 0;
}

static int register_write_callback(const uint8_t address, const uint8_t value) {
    if (address >= sizeof(register_handlers) / sizeof(register_handlers[0])) {
        return -ENOMEM;
    }
    if (!register_handlers[address].write) {
        return -ENOTSUP;
    }

    register_handlers[address].write(value);
    return 0;
}

static void sw_version_major_register_read(uint8_t *value) {
    *value = GIT_VERSION_MAJOR;
}

static void sw_version_minor_register_read(uint8_t *value) {
    *value = GIT_VERSION_MINOR;
}

static void sw_version_patch_register_read(uint8_t *value) {
    *value = GIT_VERSION_PATCH;
}

static void display_register_read(uint8_t *value) {
    // TODO implement
}

static void display_register_write(uint8_t value) {
    // TODO implement
}

static void voltage_register_read(uint8_t *value) {
    flyback_get_voltage(value);
}

static void voltage_register_write(const uint8_t value) {
    flyback_set_voltage(value);
}

static void duty_register_read(uint8_t *value) {
    flyback_get_duty_percent(value);
}

static void flyback_enable_register_read(uint8_t *value) {
    *value = flyback_is_running();
}

static void flyback_enable_register_write(uint8_t value) {
    if (value && !flyback_is_running()) {
        flyback_start();
    } else if (!value && flyback_is_running()) {
        flyback_stop();
    }
}


// event handler
__attribute__((interrupt)) void I2C1_EV_IRQHandler(void) {
    const uint16_t star1 = I2C1->STAR1;
    const uint16_t star2 = I2C1->STAR2;

    // start
    if (star1 & I2C_STAR1_ADDR) {
        i2c_slave_state.first_write = true; // next write will be the offset
    }

    // write
    if (star1 & I2C_STAR1_RXNE) {
        if (i2c_slave_state.first_write) { // first write
            i2c_slave_state.offset = I2C1->DATAR;
            i2c_slave_state.first_write = false;
        } else { // normal write
            register_write_callback(i2c_slave_state.offset, I2C1->DATAR);
            i2c_slave_state.offset++;
        }
    }

    // read
    if (star1 & I2C_STAR1_TXE) {
        register_read_callback(i2c_slave_state.offset);
        i2c_slave_state.offset++;
    }

    // stop
    if (star1 & I2C_STAR1_STOPF) {
        I2C1->CTLR1 &= ~(I2C_CTLR1_STOP); // Clear stop
    }
}

// error handler
__attribute__((interrupt)) void I2C1_ER_IRQHandler(void) {
    const uint16_t star1 = I2C1->STAR1;

    if (star1 & I2C_STAR1_BERR) {
        // bus error
        I2C1->STAR1 &= ~(I2C_STAR1_BERR); // clear
    }

    if (star1 & I2C_STAR1_ARLO) {
        // arbitration lost
        I2C1->STAR1 &= ~(I2C_STAR1_ARLO); // clear
    }

    if (star1 & I2C_STAR1_AF) {
        // acknowledge failure
        I2C1->STAR1 &= ~(I2C_STAR1_AF); // clear
    }
}


int i2c_init(void) {
    funGpioInitC();
    funPinMode(PC1, GPIO_Speed_10MHz | GPIO_CNF_OUT_OD_AF); // SDA
    funPinMode(PC2, GPIO_Speed_10MHz | GPIO_CNF_OUT_OD_AF); // SCL
    funPinMode(PC5, GPIO_Speed_10MHz | GPIO_CNF_IN_PUPD); // ADDR0
    funPinMode(PC6, GPIO_Speed_10MHz | GPIO_CNF_IN_PUPD); // ADDR1
    funPinMode(PC7, GPIO_Speed_10MHz | GPIO_CNF_IN_PUPD); // ADDR2

    GPIOC->OUTDR |= 1 << 5 | 1 << 6 | 1 << 7; // address pullups
    // GPIOC->OUTDR |= 1 << 1 | 1 << 2; // i2c pullups

    RCC->APB1PCENR |= RCC_APB1Periph_I2C1;
    RCC->APB1PRSTR |= RCC_APB1Periph_I2C1;
    RCC->APB1PRSTR &= ~RCC_APB1Periph_I2C1;

    I2C1->CTLR1 |= I2C_CTLR1_SWRST;
    I2C1->CTLR1 &= ~I2C_CTLR1_SWRST;

    // set module clock frequency
    const uint32_t prerate = 2000000;
    I2C1->CTLR2 |= (FUNCONF_SYSTEM_CORE_CLOCK / prerate) & I2C_CTLR2_FREQ;

    // enable interrupts
    I2C1->CTLR2 |= I2C_CTLR2_ITBUFEN | I2C_CTLR2_ITEVTEN | I2C_CTLR2_ITERREN;

    NVIC_EnableIRQ(I2C1_EV_IRQn); // event interrupt
    NVIC_SetPriority(I2C1_EV_IRQn, 2 << 4);
    NVIC_EnableIRQ(I2C1_ER_IRQn); // error interrupt
    NVIC_SetPriority(I2C1_ER_IRQn, 2 << 4);

    // set i2c clock frequency
    const uint32_t clockrate = 100 * 1000;
    // standard mode good to 100kHz
    I2C1->CKCFGR = (FUNCONF_SYSTEM_CORE_CLOCK / (2 * clockrate)) & I2C_CKCFGR_CCR;

    uint8_t address;
    const int ret = get_address(&address);
    if (ret < 0) {
        return ret;
    }

    I2C1->OADDR1 = address << 1;

    // enable peripheral
    I2C1->CTLR1 |= I2C_CTLR1_PE;

    // enable auto ack
    I2C1->CTLR1 |= I2C_CTLR1_ACK;

    return 0;
}
