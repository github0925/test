LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES := \
	$(LOCAL_DIR)/inc/ $(GLOBAL_INCLUDES) $(LOCAL_DIR)/fw

MODULE_SRCS += \
	$(LOCAL_DIR)/src/dw_umctl2.c \
	$(LOCAL_DIR)/src/run_ddr_init_seq.c \
	$(LOCAL_DIR)/fw/fw.S \
    $(LOCAL_DIR)/fw/lpddr4_pmu_train_strings.c \
    $(LOCAL_DIR)/fw/lpddr4_pmu_train_2d_string.c \
    $(LOCAL_DIR)/fw/lpddr4x_pmu_train_1d_string.c \
    $(LOCAL_DIR)/fw/lpddr4x_pmu_train_2d_string.c \
    $(LOCAL_DIR)/fw/ddr4_pmu_train_1d_string.c \
    $(LOCAL_DIR)/fw/ddr4_pmu_train_2d_string.c \
    $(LOCAL_DIR)/fw/ddr3_pmu_train_1d_string.c \
    $(LOCAL_DIR)/fw/streaming_str.c

include make/module.mk
