### AndroidHAL

Android HAL 入门可分为以下几个小节：
- 在Android 内核添加驱动
- 增加C可执行程序来访问硬件驱动程序
- 在Android硬件抽象层增加接口模块访问硬件驱动程序
- 编写JNI方法在应用程序框架层提供Java接口访问硬件
- 应用程序框架层增加硬件服务接口


### 在Android 内核添加驱动

> 可参考`kernel/drivers`目录

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
- 如果已经编译过内核，可能还需要删除out目录的`.config`文件，如：`rm ./out/target/product/x86/obj/kernel/.config`


### 增加C可执行程序来访问硬件驱动程序

> 可参考`external/vireg`目录

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
    vireg \
```