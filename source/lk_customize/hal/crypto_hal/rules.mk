LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/inc/

ifeq ($(SUPPORT_CE_SDDRV),true)
MODULE_SRCS += \
	$(LOCAL_DIR)/src/crypto_hal.c \
	$(LOCAL_DIR)/src/pka/sd_rsa.c \
    $(LOCAL_DIR)/src/cipher/sd_aes.c \
	$(LOCAL_DIR)/src/mac/sd_cmac.c \
	$(LOCAL_DIR)/src/hash/sd_hash.c \
	$(LOCAL_DIR)/src/mac/sd_hmac.c \
	$(LOCAL_DIR)/src/rng/sd_rng.c \
	$(LOCAL_DIR)/src/pka/sd_ecc.c \
	$(LOCAL_DIR)/src/pka/sd_sm2.c \
	$(LOCAL_DIR)/src/pka/sd_dsa.c \
	$(LOCAL_DIR)/src/pka/sd_ecdsa.c 
else
MODULE_SRCS += \
	$(LOCAL_DIR)/src/crypto_hal_weak.c
endif
include make/module.mk
