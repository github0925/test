include $(MKDIR)/build.mk

SUBDIRS += mini_libc \
		   slib \
		   str

ifneq ($(VIRT_UART_MEMBASE), )
SUBDIRS += virt_uart
endif

ifeq ($(APP), t_loader)
SUBDIRS += partition \
		   storage_device \
		   mem_image \
		   boardinfo \
		   cksum \
		   libavb
endif

ifeq ($(VERIFIED_BOOT),true)
SUBDIRS += \
		verified_boot \
		sd_x509 \
		crypto
endif

ifeq ($(SUPPORT_HEAP),true)
SUBDIRS += heap
endif

include $(MKDIR)/objs_subdir.mk
include $(MKDIR)/rules.mk

