include $(MKDIR)/build.mk

# Specify SUBDIRS here.
C_SRC += \
	avb_kernel_cmdline_descriptor.c \
	avb_chain_partition_descriptor.c \
	avb_sysdeps_posix.c \
	avb_cmdline.c \
	avb_property_descriptor.c \
	avb_vbmeta_image.c \
	avb_hash_descriptor.c \
	avb_util.c \
	avb_hashtree_descriptor.c \
	avb_version.c \
	avb_crc32.c \
	avb_descriptor.c \
	avb_footer.c \
	avb_crypto.c \
	avb_slot_verify.c \

ifeq ($(SUPPORT_CE_SDDRV), true)
C_SRC += \
	avb_sha_ce.c \
	avb_rsa_ce.c

CFG_CFLAGS += -DLIBAVB_USE_CE=1
else
C_SRC += \
	avb_rsa.c \
	avb_sha256.c \
	avb_sha512.c

endif

CFG_CFLAGS += -DAVB_COMPILATION

include $(MKDIR)/objs_subdir.mk
include $(MKDIR)/rules.mk
