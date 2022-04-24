### AndroidHAL

Android HAL 入门可分为以下几个小节：
- Android内核添加驱动
- 测试硬件驱动程序
- 添加HAL层代码
- 添加JNI层代码
- 框架层添加硬件访问服务
- 自定义adb命令访问框架层服务


### Android内核添加驱动

> 代码路径：`kernel/drivers`

对应目录结构如下：
```
./kernel/drivers/
|-- Kconfig
|-- Makefile
`-- vireg
    |-- Kconfig
    |-- Makefile
    |-- vireg.c
    `-- vireg.h

```

在kernel目录下，新增了`vireg`虚拟驱动寄存器。

内核模块的加载类型：
- y：集成模块到内核中
- m：动态加载模块方式
- 空值则不编译此模块

驱动模块验证是否集成到设备上：
- 检查`/dev`路径是否有`vireg`文件
- 检查`/proc`路径是否有`vireg`文件
- 检查`/sys/class`路径是否有`vireg`目录


常用编译操作：
- 在`kernel`目录中清除已有配置命令：`make mrproper`
- 如果已经编译过内核，可能还需要删除out目录的`.config`文件，例如：`rm ./out/target/product/x86/obj/kernel/.config`


### 测试硬件驱动程序

> 代码路径：`external/vireg`

对应目录结构如下：
```
./external
`-- vireg
    |-- Android.mk
    `-- vireg.c
```

调用方式有：
1. 单编模块，重新生成`system.img`刷入
2. 引入声明的外部模块`vireg`

无论是何种调用方式，均会在设备`system/bin/`目录生成`vireg`可执行文件，执行后可通过它访问`/dev/vireg`驱动。

#### 单编外部模块

- 在工程根目录，单编vireg外部模块：`mmm ./external/vireg`
- 重新生成`system.img`镜像：`make snod`
- 刷入新生成的系统镜像

此时会在`out/target/product/x86/system/bin`路径目录生成vireg可执行文件。



#### 引入外部模块

采用全编方式，将外部模块vireg合入。

一般会在`device/generic/common/packages.mk`注册模块，如下：
```makefile
# Custom vireg bin
PRODUCT_PACKAGES += \
    vireg_test \
```


### 添加HAL层代码

> 代码路径：`hardware/libhardware`

对应目录结构如下：
```
./hardware/libhardware/
|-- include
|   `-- hardware
|       `-- vireg.h
`-- modules
    `-- vireg
        |-- Android.mk
        `-- vireg.cpp
```

成功加入驱动层代码后，接下来需要按照HAL的编写格式来添加代码。

单编命令：`mmm hardware/libhardware/modules/vireg`

编译成功后会在`out/target/product/x86/system/lib/hw`目录下生成`vireg.default.so`文件。

注意：上面的 x86 是设备产品名称。


### 添加JNI层代码

> 代码路径：`frameworks/base/services/core/jni/`

对应目录结构如下：
```
./frameworks/base/services/core/jni/
|-- Android.mk
|-- com_android_server_ViregService.cpp
`-- onload.cpp
./system/core/rootdir/
`-- ueventd.rc
```

- 新增`com_android_server_ViregService.cpp`文件，提供vireg驱动的读写操作

- 新增`Android.mk`文件，添加如下代码：
    ```makefile
    LOCAL_SRC_FILES += \
    $(LOCAL_REL_DIR)/com_android_server_ViregService.cpp \
    ```

- 在`onload.cpp`的 namespace android 块里添加如下代码: 
    ```cpp
    namespace android {
        int register_android_server_ViregService(JNIEnv* env);
    };

    using namespace android;

    extern "C" jint JNI_OnLoad(JavaVM* vm, void* /* reserved */)
    {

        // ...
        register_android_server_ViregService(env);

    return JNI_VERSION_1_4;
    }
    ```

- 添加硬件设备访问权限（`system/core/rootdir/ueventd.rc`）:`/dev/vireg      0666   root     root`

- 单编`mmm frameworks/base/services/core/jni`，打包system镜像`make snod`


### 框架层添加硬件访问服务


> 代码路径：`frameworks/base/`和`system/sepolicy/`

对应目录结构如下：
```
./frameworks/base/
|-- Android.mk
|-- core
|   `-- java
|       `-- android
|           `-- os
|               `-- IViregService.aidl
`-- services
    `-- core
        |-- java
        |   `-- com
        |       `-- android
        |           `-- server
        |               `-- ViregService.java

./system/sepolicy/
|-- device.te
|-- domain.te
|-- file_contexts
|-- service.te
|-- service_contexts
|-- system_app.te
|-- system_server.te
`-- untrusted_app.te
```

- 新增`ViregService.java`访问JNI层

- 新增`IViregService.aidl`提供给外部调用。默认 ViregService 运行在`system_server`进程中

- 在`sepolicy`设置 vireg 需要的安全权限

- 在SystemServer上相关位置进行服务注册：`ServiceManager.addService("freg", new ViregService());`


### 自定义adb命令访问框架层服务


> 代码路径：`frameworks/base/cmds/vireg`

对应目录结构如下：
```
frameworks/base/cmds/
`-- vireg
    |-- Android.mk
    |-- src
    |   `-- com
    |       `-- android
    |           `-- commands
    |               `-- vireg
    |                   `-- Vireg.java
    `-- vireg
```

可在`build/target/product/base.mk`或类似的mk引入vireg执行程序。
```makefile
PRODUCT_PACKAGES += \
    vireg \
```