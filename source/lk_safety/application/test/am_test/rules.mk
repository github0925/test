AM_TEST_DIR := $(GET_LOCAL_DIR)

MODULE := $(AM_TEST_DIR)

MODULE_SRCS += \
	$(AM_TEST_DIR)/am_test.c


MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable \
		       -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast

include make/module.mk
#include $(AM_TEST_DIR)/../../../exdev/am_codec/rules.mk
#include $(AM_TEST_DIR)/../../../exdev/am_board/rules.mk
#MODULE_DEPS += $(AM_TEST_DIR)/../../../exdev/am_codec
#MODULE_DEPS += $(AM_TEST_DIR)/../../../exdev/am_board