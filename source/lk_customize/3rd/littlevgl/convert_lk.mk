VPATH := $(patsubst :%, %, $(VPATH))
MODULE_SRCS += $(patsubst %.c,$(join $(VPATH)/, %.c), $(CSRCS))
MODULE_CFLAGS += $(CFLAGS)
CFLAGS :=
VPATH :=
CSRCS :=
