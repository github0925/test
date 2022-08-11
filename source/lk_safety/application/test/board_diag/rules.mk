LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)
GLOBAL_INCLUDES += $(LOCAL_DIR)/inc

MODULE_SRCS += \
	$(LOCAL_DIR)/src/board_start.c \
	$(LOCAL_DIR)/src/cqueue.c \
	$(LOCAL_DIR)/src/dqueue.c \
	$(LOCAL_DIR)/src/func_can.c \
	$(LOCAL_DIR)/src/func_cpt.c \
	$(LOCAL_DIR)/src/func_adc.c \
	$(LOCAL_DIR)/src/func_lin.c \
	$(LOCAL_DIR)/src/func_dio.c \
	$(LOCAL_DIR)/src/func_i2c.c \
	$(LOCAL_DIR)/src/func_ospi.c \
	$(LOCAL_DIR)/src/func_emmc.c \
	$(LOCAL_DIR)/src/func_ddr.c \
	$(LOCAL_DIR)/src/func_power_mgr.c \
	$(LOCAL_DIR)/src/func_sleep_mgr.c \
	$(LOCAL_DIR)/src/func_usart.c \
	$(LOCAL_DIR)/src/func_diag.c \
	$(LOCAL_DIR)/src/vcan_resp.c \
	$(LOCAL_DIR)/src/sw_time.c \
	$(LOCAL_DIR)/src/board_cfg.c \
	$(LOCAL_DIR)/src/board_init.c \
	$(LOCAL_DIR)/src/rtc.c \
	$(LOCAL_DIR)/src/wake_up.c \
	$(LOCAL_DIR)/src/power_down.c \
	$(LOCAL_DIR)/src/remote_test.c \
	$(LOCAL_DIR)/src/func_eth.c \
	$(LOCAL_DIR)/src/ioexpend_tca9539_dev.c \
	$(LOCAL_DIR)/src/clk_5p49_dev.c \
	$(LOCAL_DIR)/src/pmic_pf8200_dev.c \
	$(LOCAL_DIR)/src/func_usb.c \
	$(LOCAL_DIR)/src/func_release_version.c \
	$(LOCAL_DIR)/src/func_storage_stress.c  \
	$(LOCAL_DIR)/src/crc.c  \
	$(LOCAL_DIR)/src/func_eth_self_check.c  \
	$(LOCAL_DIR)/src/spread_spectrum.c  \
include make/module.mk
