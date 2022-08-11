FreeRTOS_SRC ?=
cur_mkfile := $(abspath $(lastword $(MAKEFILE_LIST)))
cur_path := $(patsubst %/, %, $(dir $(cur_mkfile)))
$(info cur_path=$(cur_path))
tmp_FreeRTOS_ROOT = $(cur_path)/../freertos/FreeRTOS
exist_tmp_FreeRTOS_ROOT = $(shell if [ -d $(tmp_FreeRTOS_ROOT) ]; then echo "exist"; else echo "notexist"; fi;)
ifeq ("$(exist_tmp_FreeRTOS_ROOT)","exist")
FreeRTOS_ROOT ?= $(cur_path)/../freertos/FreeRTOS
else
FreeRTOS_ROOT ?= $(cur_path)/../..
endif
FreeRTOS_INC := \
	. \
	$(FreeRTOS_ROOT)/Source/include \
	$(FreeRTOS_ROOT)/Source/portable/GCC/ARM_CR5

IAR_FreeRTOS_INC := \
	.\
	../freertos/FreeRTOS/Source/include \
	../freertos/FreeRTOS/Source/portable/GCC/ARM_CR5
export IAR_FreeRTOS_INC