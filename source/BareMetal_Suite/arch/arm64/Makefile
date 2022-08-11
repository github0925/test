include $(MKDIR)/build.mk

# specify SUBDIRS here
SUBDIRS += $(CPU)

C_SRC = exceptions_c.c
S_SRC = vector.S start.S cache-ops.S spinlock.S armv8_mmu_s.S mem_opt.S

include $(MKDIR)/objs_subdir.mk
include $(MKDIR)/rules.mk
