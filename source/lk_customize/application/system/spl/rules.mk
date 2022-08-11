LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/spl.c \
    $(LOCAL_DIR)/crc32.c

MODULE_DEPS += \
	lib/partition \
	lib/fastboot_common \
	lib/md5 \
	lib/libavb \
	lib/boot

ifeq ($(VERIFIED_BOOT), true)
MODULE_DEPS += \
	lib/verified_boot
endif

DDR_SEQ_PARSER := $(LOCAL_DIR)/tools/ddr_seq_parser
ifneq ($(DDR_TYPE),)
    DDR_FIRMWARE_NAME := $(LOCAL_DIR)/tools/$(DDR_TYPE)_training_fw.pac
    DDR_SCRIPT := $(BUILDROOT)/target/$(TARGET)/secure/ddr_init_script/$(DDR_TYPE)/ddr_init_$(DDR_TYPE)_$(DDR_SIZE)_$(DDR_FREQ).c
    MODULE_SRCDEPS += ddr_fw_target
endif

.PHONY:ddr_fw_target
ddr_fw_target:
	$(AT) mkdir -p $(BUILDDIR)
	$(AT) $(DDR_SEQ_PARSER) if=$(DDR_SCRIPT) of=$(BUILDDIR)/ddr_init_seq.bin
	$(AT) cp -f $(DDR_FIRMWARE_NAME) $(BUILDDIR)/ddr_fw.bin

include make/module.mk
