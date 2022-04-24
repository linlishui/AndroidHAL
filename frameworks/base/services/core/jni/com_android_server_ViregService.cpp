#define LOG_TAG "ViregServiceJNI"

#include "jni.h"
#include "JNIHelp.h"
#include "android_runtime/AndroidRuntime.h"

#include <utils/misc.h>
#include <utils/Log.h>
#include <hardware/hardware.h>
#include <hardware/vireg.h>

#include <stdio.h>

namespace android
{
        static void vireg_setVal(JNIEnv* env, jobject clazz, jint ptr, jint value) {
                vireg_device_t* device = (vireg_device_t*)ptr;
                if(!device) {
                        ALOGE("Device vireg is not open.");
                        return;
                }

                int val = value;

                ALOGI("Set value %d to device vireg.", val);

                device->set_val(device, val);
        }

        static jint vireg_getVal(JNIEnv* env, jobject clazz, jint ptr) {
                vireg_device_t* device = (vireg_device_t*)ptr;
                if(!device) {
                        ALOGE("Device vireg is not open.");
                        return 0;
                }

                int val = 0;

                device->get_val(device, &val);

                ALOGI("Get value %d from device vireg.", val);

                return val;
        }

        static inline int vireg_device_open(const hw_module_t* module, struct vireg_device_t** device) {
                return module->methods->open(module, VIREG_HARDWARE_DEVICE_ID, (struct hw_device_t**)device);
        }

        static jint vireg_init(JNIEnv* env, jclass clazz) {
                vireg_module_t* module;
                vireg_device_t* device;

                ALOGI("Initializing HAL stub vireg......");

                if(hw_get_module(VIREG_HARDWARE_MODULE_ID, (const struct hw_module_t**)&module) == 0) {
                        ALOGI("Device vireg found.");
                        if(vireg_device_open(&(module->common), &device) == 0) {
                                ALOGI("Device vireg is open.");
                                return (jint)device;
                        }

                        ALOGE("Failed to open device vireg.");
                        return 0;
                }

                ALOGE("Failed to get HAL stub vireg.");

                return 0;
        }

        static const JNINativeMethod method_table[] = {
                {"init_native", "()I", (void*)vireg_init},
                {"setVal_native", "(II)V", (void*)vireg_setVal},
                {"getVal_native", "(I)I", (void*)vireg_getVal},
        };

        int register_android_server_ViregService(JNIEnv *env) {
                return jniRegisterNativeMethods(env, "com/android/server/ViregService", method_table, NELEM(method_table));
        }
};