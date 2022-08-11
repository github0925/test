include $(MKDIR)/build.mk

MODULE_CFLAGS += -I./inc -I./fw

C_SRC = ./src/dw_umctl2.c\
        ./src/run_ddr_init_seq.c\
        ./fw/streaming_str.c

ifdef DDR_FW_EMBEDDED
C_SRC += ./fw/lpddr4_pmu_train_1d_string.c\
        ./fw/lpddr4_pmu_train_2d_string.c \
        ./fw/lpddr4x_pmu_train_1d_string.c \
        ./fw/lpddr4x_pmu_train_2d_string.c \
        ./fw/ddr4_pmu_train_1d_string.c \
        ./fw/ddr4_pmu_train_2d_string.c \
        ./fw/ddr3_pmu_train_1d_string.c \
        ./src/ddr_init_seq_tmplt.c\

S_SRC = ./fw/fw.S
ifneq ($(DIAG),)
S_SRC += ./fw/fw_diag.S
endif
endif

include $(MKDIR)/objs_subdir.mk
include $(MKDIR)/rules.mk
