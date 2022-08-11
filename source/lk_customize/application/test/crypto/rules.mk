LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/hash_data.c \
	$(LOCAL_DIR)/cipher_data.c \
	$(LOCAL_DIR)/rsa_data.c \
	$(LOCAL_DIR)/ecc_data.c \
	$(LOCAL_DIR)/ecdsa_data.c \
	$(LOCAL_DIR)/dsa_data.c \
	$(LOCAL_DIR)/hash_test.c \
	$(LOCAL_DIR)/cipher_test.c \
	$(LOCAL_DIR)/rsa_test.c \
	$(LOCAL_DIR)/trng_test.c \
	$(LOCAL_DIR)/ecc_keygen_test.c \
	$(LOCAL_DIR)/ecdsa_test.c \
	$(LOCAL_DIR)/sm2_test.c \
	$(LOCAL_DIR)/dsa_test.c \
	$(LOCAL_DIR)/ce_test.c

ifeq ($(ARCH), arm)
MODULE_SRCS += \
	$(LOCAL_DIR)/addr_access.S
else ifeq ($(ARCH), arm64)
MODULE_SRCS += \
	$(LOCAL_DIR)/addr_access_64.S
endif

GLOBAL_DEFINES += \
	TEST_CHECK_RESULT=1\
	HW_NOT_MEET=1

MODULE_DEPS += \
	lib/libc

include make/module.mk
