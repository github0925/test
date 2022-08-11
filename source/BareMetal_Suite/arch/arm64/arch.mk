ARCH_CFLAGS += -march=armv8-a

ifdef D
OPT_CFLAGS ?= -O0
else
OPT_CFLAGS ?= -O2
endif
ARCH_CFLAGS += ${OPT_CFLAGS}
ARCH_CFLAGS += -g -std=c99 -Wall -Werror

# use linker garbage collectio
ARCH_CFLAGS += -ffunction-sections -fdata-sections

STDLIB_CFLAGS ?= -DNO_STDLIB
ifneq ($(TCH),armcc)
LDFLAGS += -Wl,--gc-sections #--print-gc-sections
LDFLAGS += -Wl,--build-id=none
STDLIB_LDFLAGS ?= -nostartfiles -nostdlib
else
ARCH_CFLAGS += -nostdlib
STDLIB_LDFLAGS ?= --no_scanlib
endif
LDFLAGS += ${STDLIB_LDFLAGS}
ARCH_CFLAGS += ${STDLIB_CFLAGS}
