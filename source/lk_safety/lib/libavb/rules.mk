LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)


GLOBAL_INCLUDES += $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/avb_kernel_cmdline_descriptor.c \
	$(LOCAL_DIR)/avb_chain_partition_descriptor.c \
	$(LOCAL_DIR)/avb_sysdeps_posix.c \
	$(LOCAL_DIR)/avb_cmdline.c \
	$(LOCAL_DIR)/avb_property_descriptor.c \
	$(LOCAL_DIR)/avb_vbmeta_image.c \
	$(LOCAL_DIR)/avb_hash_descriptor.c \
	$(LOCAL_DIR)/avb_util.c \
	$(LOCAL_DIR)/avb_hashtree_descriptor.c \
	$(LOCAL_DIR)/avb_version.c \
	$(LOCAL_DIR)/avb_crc32.c \
	$(LOCAL_DIR)/avb_descriptor.c \
	$(LOCAL_DIR)/avb_footer.c \
	$(LOCAL_DIR)/avb_crypto.c \
	$(LOCAL_DIR)/avb_slot_verify.c \

ifeq ($(SUPPORT_CE_SDDRV), true)
GLOBAL_DEFINES += LIBAVB_USE_CE=1
MODULE_SRCS += \
	$(LOCAL_DIR)/avb_sha_ce.c \
	$(LOCAL_DIR)/avb_rsa_ce.c
else
MODULE_SRCS += \
	$(LOCAL_DIR)/avb_rsa.c \
	$(LOCAL_DIR)/avb_sha256.c \
	$(LOCAL_DIR)/avb_sha512.c

endif

GLOBAL_CFLAGS += -DAVB_COMPILATION
include make/module.mk
