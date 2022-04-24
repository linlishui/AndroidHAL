#define LOG_TAG "FregHALStub"

#include <hardware/hardware.h>
#include <hardware/vireg.h>

#include <fcntl.h>
#include <errno.h>

#include <cutils/log.h>
#include <cutils/atomic.h>

#include <malloc.h>
#include <stdint.h>
#include <sys/time.h>

#include <hardware/hardware.h>
#include <system/audio.h>
#include <hardware/audio.h>



#define DEVICE_NAME "/dev/vireg"
#define MODULE_NAME "Vireg"
#define MODULE_AUTHOR "lishui.lin@qq.com"

static int vireg_device_open(const struct hw_module_t* module, const char* id, struct hw_device_t** device);
static int vireg_device_close(struct hw_device_t* device);
static int vireg_set_val(struct vireg_device_t* dev, int val);
static int vireg_get_val(struct vireg_device_t* dev, int* val);

static struct hw_module_methods_t vireg_module_methods = {
        open: vireg_device_open
};

struct vireg_module_t HAL_MODULE_INFO_SYM = {
        common: {
                tag: HARDWARE_MODULE_TAG,
                version_major: 1,
                version_minor: 0,
                id: VIREG_HARDWARE_MODULE_ID,
                name: MODULE_NAME,
                author: MODULE_AUTHOR,
                methods: &vireg_module_methods,
        }
};

static int vireg_device_open(const struct hw_module_t* module, const char* id, struct hw_device_t** device) {
        if(!strcmp(id, VIREG_HARDWARE_DEVICE_ID)) {
                struct vireg_device_t* dev;

                dev = (struct vireg_device_t*)malloc(sizeof(struct vireg_device_t));
                if(!dev) {
                        ALOGE("Failed to alloc space for vireg_device_t.");
                        return -EFAULT;
                }

                memset(dev, 0, sizeof(struct vireg_device_t));

                dev->common.tag = HARDWARE_DEVICE_TAG;
                dev->common.version = 0;
                dev->common.module = (hw_module_t*)module;
                dev->common.close = vireg_device_close;
                dev->set_val = vireg_set_val;
                dev->get_val = vireg_get_val;

                if((dev->fd = open(DEVICE_NAME, O_RDWR)) == -1) {
                        ALOGE("Failed to open device file /dev/vireg -- %s.", strerror(errno));
                        free(dev);
                        return -EFAULT;
                }

                *device = &(dev->common);

                ALOGI("Open device file /dev/vireg successfully.");

                return 0;
        }

        return -EFAULT;
}

static int vireg_device_close(struct hw_device_t* device) {
        struct vireg_device_t* vireg_device = (struct vireg_device_t*)device;
        if(vireg_device) {
                close(vireg_device->fd);
                free(vireg_device);
        }

        return 0;
}

static int vireg_set_val(struct vireg_device_t* dev, int val) {
        if(!dev) {
                ALOGE("Null dev pointer.");
                return -EFAULT;
        }

        ALOGI("Set value %d to device file /dev/vireg.", val);
        write(dev->fd, &val, sizeof(val));

        return 0;
}

static int vireg_get_val(struct vireg_device_t* dev, int* val) {
        if(!dev) {
                ALOGE("Null dev pointer.");
                return -EFAULT;
        }

        if(!val) {
                ALOGE("Null val pointer.");
                return -EFAULT;
        }

        read(dev->fd, val, sizeof(*val));

        ALOGI("Get value %d from device file /dev/vireg.", *val);

        return 0;
}