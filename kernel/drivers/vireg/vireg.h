#ifndef _VIRTUAL_REG_H_
#define _VIRTUAL_REG_H_

#include <linux/cdev.h>
#include <linux/semaphore.h>

#define VIREG_DEVICE_NODE_NAME  "vireg"
#define VIREG_DEVICE_FILE_NAME  "vireg"
#define VIREG_DEVICE_PROC_NAME  "vireg"
#define VIREG_DEVICE_CLASS_NAME "vireg"

struct virtual_reg_dev {
    int val;
    struct semaphore sem;
    struct cdev dev;
};

#endif
