LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include/

MODULE_SRCS += \
	$(LOCAL_DIR)/sram_conf.c \
	$(LOCAL_DIR)/ce.c \
	$(LOCAL_DIR)/sx_hash.c \
	$(LOCAL_DIR)/sx_cipher.c \
	$(LOCAL_DIR)/sx_pke_conf.c \
	$(LOCAL_DIR)/sx_rsa_pad.c \
	$(LOCAL_DIR)/sx_dma.c \
	$(LOCAL_DIR)/sx_rsa.c \
	$(LOCAL_DIR)/sx_trng.c \
	$(LOCAL_DIR)/sx_ecc_keygen.c \
	$(LOCAL_DIR)/sx_ecdsa.c \
	$(LOCAL_DIR)/sx_sm2.c \
	$(LOCAL_DIR)/sx_dsa.c \
	$(LOCAL_DIR)/sx_srp.c \
	$(LOCAL_DIR)/sx_eddsa.c \
	$(LOCAL_DIR)/sx_pke_funcs.c \
	$(LOCAL_DIR)/sx_ecc.c \
	$(LOCAL_DIR)/sx_math.c \
	$(LOCAL_DIR)/sx_prime.c

GLOBAL_DEFINES += \
	WAIT_PK_WITH_REGISTER_POLLING=0\
	RSA_PERFORMANCE_TEST=1

include make/module.mk
