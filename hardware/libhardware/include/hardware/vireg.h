#ifndef ANDROID_VIREG_INTERFACE_H
#define ANDROID_VIREG_INTERFACE_H

#include <hardware/hardware.h>

__BEGIN_DECLS

/**
 * The id of this module
 */
#define VIREG_HARDWARE_MODULE_ID "vireg"

/**
 * The id of this device
 */
#define VIREG_HARDWARE_DEVICE_ID "vireg"

struct vireg_module_t {
        struct hw_module_t common;
};

struct vireg_device_t {
        struct hw_device_t common;
        int fd;
        int (*set_val)(struct vireg_device_t* dev, int val);
        int (*get_val)(struct vireg_device_t* dev, int* val);
};

__END_DECLS

#endif