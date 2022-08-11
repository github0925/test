# SPDX-License-Identifier: GPL-2.0
ccflags-y += -g -O1 \
	-DUSE_LINUX \
	-Wno-unused-variable \
	-DKO_VERSION=\"$(shell date +%Y-%m-%d-%H:%M:%S)\"

sdrv-g2d-objs := \
	sdrv_g2d.o \
	g2d_normal.o \
	g2d_lite.o \
	g2d_core.o \
	pipes/gpipe_g2d.o \
	pipes/spipe_g2d.o

obj-m += sdrv-g2d.o

# obj-m := my_module.o
# Author: infi.chen
CROSS_COMPILE?="$(ANDROID_TOOLCHAIN)/aarch64-linux-android-"
KDIR?=$(ANDROID_PRODUCT_OUT)/obj/KERNEL_OBJ/
CROSS_ARCH:=ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE}
PWD:=$(shell pwd)

all:
	$(info please make yocto or make android !!)

yocto: clean
	$(info yocto KDIR: $(KDIR))
	unset LDFLAGS
	$(MAKE) $(CROSS_ARCH) -C $(KDIR) M=$(PWD) modules
	@mkdir -p ./package/$@
	@cp -v $(obj-m:.o=.ko) ./package/$@

android: clean
	@if [ "$(ANDROID_BUILD_TOP)_yes" = "_yes" ]; then echo "source android env first!!!!!" &&exit 1; fi
	#@if [ ! $(CONFIG_DRM_FBDEV_EMULATION)_yes = "_yes" ]; then echo "you should disable fb when build android!!" && exit 1; fi
	$(info android KDIR: $(KDIR))
	$(MAKE) $(CROSS_ARCH) -C $(KDIR) M=$(PWD) modules
	@mkdir -p ./package/$@
	@cp -v $(obj-m:.o=.ko) ./package/$@

dtbs:
	$(MAKE) dtbs $(CROSS_ARCH) -C $(KDIR)

.PHONY: modules package clean

modules: clean
	$(MAKE) $(CROSS_ARCH) -C $(KDIR) M=$(PWD) modules

package:
	@mkdir -p ./package
	@cp -v $(obj-m:.o=.ko) ./package

clean:
	rm -rf package
	$(MAKE) $(CROSS_ARCH) -C $(KDIR) M=$(PWD) clean
