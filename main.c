#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#include "version.h"


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


struct register_handlers {
    register_read_handler_t read;
    register_write_handler_t write;
};


struct register_handlers register_handlers[6] = {
    {sw_version_major_register_read, NULL},
    {sw_version_minor_register_read, NULL},
    {sw_version_patch_register_read, NULL},
    {display_register_read, display_register_write},
    {voltage_register_read, voltage_register_write},
    {duty_register_read, NULL},
};


static int register_read_callback(uint8_t address) {
    if (address >= 6 || !register_handlers[address].read) {
        return -1;
    }

    uint8_t value;
    register_handlers[address].read(&value);
    // TODO send back read value
}

static int register_write_callback(const uint8_t address, const uint8_t value) {
    if (address >= 6 || !register_handlers[address].write) {
        return -1;
    }

    register_handlers[address].write(value);
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
    // TODO implement
}

static void voltage_register_write(uint8_t value) {
    // TODO implement
}

static void duty_register_read(uint8_t *value) {
    // TODO implement
}


int main(void) {
    return 0;
}
