ARCH_CFLAGS += -mcpu=$(CPU)
ifeq ($(TCH),armcc)
ARCH_CFLAGS += --target=arm-arm-none-eabi
endif

ARCH_CFLAGS += -mfloat-abi=softfp

# By default, all ATB code will be compiled as THUM code
THUMBCFLAGS := -mthumb -D__thumb__

ifdef D
OPT_CFLAGS ?= -O0
else
# Unless performance/space is an issue, we will not enable optimization.
# This is mainly for function safety consideration.
OPT_CFLAGS ?= -O2
endif
ARCH_CFLAGS += ${OPT_CFLAGS}
ARCH_CFLAGS += -g -std=c99 -Wall -Werror

ARCH_CFLAGS += -D__ARM_ARCH=7

# use linker garbage collectio
ARCH_CFLAGS += -ffunction-sections -fdata-sections
ARCH_CFLAGS += -mno-unaligned-access

STDLIB_CFLAGS ?= -DNO_STDLIB
ifneq ($(TCH),armcc)
LDFLAGS += -Wl,--gc-sections #--print-gc-sections
STDLIB_LDFLAGS ?= -nostartfiles -nostdlib
else
ARCH_CFLAGS += -nostdlib
STDLIB_LDFLAGS ?= --no_scanlib
endif
LDFLAGS += ${STDLIB_LDFLAGS}
ARCH_CFLAGS += ${STDLIB_CFLAGS}
