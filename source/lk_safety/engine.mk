LOCAL_MAKEFILE:=$(MAKEFILE_LIST)

BUILDROOT ?= .

#export for iar
CFLAGS_IAR :=
IAR_GLB_DEF :=
IAR_MODULE_INCLUDE := $(IAR_FreeRTOS_INC)
IAR_MODULE_SRCS :=
IAR_EXECUTION_PLACE = $(EXECUTION_PLACE)
export CFLAGS_IAR
export IAR_GLB_DEF
export IAR_MODULE_INCLUDE
export IAR_MODULE_SRCS
export IAR_EXECUTION_PLACE


ifeq ($(MAKECMDGOALS),spotless)
spotless:
	rm -rf -- "$(BUILDROOT)"/build-*
else

ifndef LKROOT
$(error please define LKROOT to the root of the lk build system)
endif

-include local.mk
include make/macros.mk

# If one of our goals (from the commandline) happens to have a
# matching project/goal.mk, then we should re-invoke make with
# that project name specified...

project-name := $(firstword $(MAKECMDGOALS))

ifneq ($(project-name),)
ifneq ($(strip $(foreach d,$(LKINC),$(wildcard $(d)/project/$(project-name).mk))),)
do-nothing := 1
$(MAKECMDGOALS) _all: make-make
	@:
make-make:
	@PROJECT=$(project-name) $(MAKE) -rR -f $(LOCAL_MAKEFILE) $(filter-out $(project-name), $(MAKECMDGOALS))

.PHONY: make-make
endif
endif

# some additional rules to print some help
include make/help.mk

ifeq ($(do-nothing),)

ifeq ($(PROJECT),)

ifneq ($(DEFAULT_PROJECT),)
PROJECT := $(DEFAULT_PROJECT)
else
$(error No project specified. Use 'make list' for a list of projects or 'make help' for additional help)
endif
endif

DEBUG ?= 2

IAR_PRJ := $(PROJECT)

BUILDDIR := $(BUILDROOT)/build-$(PROJECT)$(PROJECT_BUILDDIR_POSTFIX)
OUTBIN := $(BUILDDIR)/$(PROJECT).bin
SIGNEDBIN := $(BUILDDIR)/$(PROJECT).bin.signed.rsa1024
OUTELF := $(BUILDDIR)/$(PROJECT).elf
CONFIGHEADER := $(BUILDDIR)/config.h


GLOBAL_KERNEL_INCLUDES := $(addsuffix /include,$(LKINC))
# For backwards compatibility.
GLOBAL_KERNEL_INCLUDES += $(addsuffix /include/uapi/uapi,$(LKINC)) $(addsuffix /include/shared/lk,$(LKINC))
GLOBAL_UAPI_INCLUDES := $(addsuffix /include/uapi,$(LKINC))
GLOBAL_SHARED_INCLUDES := $(addsuffix /include/shared,$(LKINC))
GLOBAL_USER_INCLUDES := $(addsuffix /include/user,$(LKINC))
GLOBAL_INCLUDES := $(BUILDDIR) $(GLOBAL_UAPI_INCLUDES) $(GLOBAL_SHARED_INCLUDES) $(GLOBAL_KERNEL_INCLUDES)
GLOBAL_OPTFLAGS ?= $(ARCH_OPTFLAGS)
GLOBAL_COMPILEFLAGS := -g -finline -include $(CONFIGHEADER)
GLOBAL_COMPILEFLAGS += -Werror -Wall -Wsign-compare -Wno-multichar -Wno-unused-function -Wno-unused-label -Wno-tautological-compare
GLOBAL_COMPILEFLAGS += -fno-short-enums -fno-common
GLOBAL_CFLAGS := --std=c11 -Wstrict-prototypes -Wwrite-strings
GLOBAL_CPPFLAGS := --std=c++11 -fno-exceptions -fno-rtti -fno-threadsafe-statics
#GLOBAL_CPPFLAGS += -Weffc++
GLOBAL_ASMFLAGS := -DASSEMBLY
GLOBAL_LDFLAGS :=

GLOBAL_LDFLAGS += $(addprefix -L,$(LKINC))

# Architecture specific compile flags
ARCH_COMPILEFLAGS :=
ARCH_CFLAGS :=
ARCH_CPPFLAGS :=
ARCH_ASMFLAGS :=

# top level rule
all:: $(OUTBIN) $(OUTELF).lst $(OUTELF).debug.lst $(OUTELF).sym $(OUTELF).sym.sorted $(OUTELF).size $(OUTELF).dump $(BUILDDIR)/srcfiles.txt $(BUILDDIR)/include_paths.txt

# master module object list
ALLOBJS_MODULE :=

# master object list (for dep generation)
ALLOBJS :=

# master source file list
ALLSRCS :=

# a linker script needs to be declared in one of the project/target/platform files
LINKER_SCRIPT :=

# anything you add here will be deleted in make clean
GENERATED := $(CONFIGHEADER)

# anything added to GLOBAL_DEFINES will be put into $(BUILDDIR)/config.h
GLOBAL_DEFINES := LK=1 __TRUSTY__=1

# Anything added to GLOBAL_SRCDEPS will become a dependency of every source file in the system.
# Useful for header files that may be included by one or more source files.
GLOBAL_SRCDEPS := $(CONFIGHEADER)

# these need to be filled out by the project/target/platform rules.mk files
TARGET :=
PLATFORM :=
ARCH :=
ALLMODULES :=
export TARGET
# add any external module dependencies
MODULES := $(EXTERNAL_MODULES)

# any .mk specified here will be included before build.mk
EXTRA_BUILDRULES :=

# any rules you put here will also be built by the system before considered being complete
EXTRA_BUILDDEPS :=

# any rules you put here will be depended on in clean builds
EXTRA_CLEANDEPS :=

# any objects you put here get linked with the final image
EXTRA_OBJS :=

# any extra linker scripts to be put on the command line
EXTRA_LINKER_SCRIPTS :=

# if someone defines this, the build id will be pulled into lib/version
BUILDID ?=

# comment out or override if you want to see the full output of each command
NOECHO ?= @

# default to building with clang
#CLANGBUILD ?= true
#override CLANGBUILD := $(call TOBOOL,$(CLANGBUILD))

#ifeq ($(call TOBOOL,$(CLANGBUILD)), true)
#GLOBAL_COMPILEFLAGS += -Wimplicit-fallthrough
#endif

# try to include the project file
-include project/$(PROJECT).mk
ifndef TARGET
$(error couldn't find project or project doesn't define target)
endif
include target/$(TARGET)/rules.mk
ifndef PLATFORM
$(error couldn't find target or target doesn't define platform)
endif
include platform/$(PLATFORM)/rules.mk

$(info PROJECT = $(PROJECT))
$(info PLATFORM = $(PLATFORM))
$(info TARGET = $(TARGET))

# default to building with clang for google prebuild gcc
#
ifneq ($(findstring android, $(ARCH_$(ARCH)_TOOLCHAIN_PREFIX)), )
CLANGBUILD ?= true
override CLANGBUILD := $(call TOBOOL,$(CLANGBUILD))
endif

ifeq ($(call TOBOOL,$(CLANGBUILD)), true)
GLOBAL_COMPILEFLAGS += -Wimplicit-fallthrough
endif

ifeq ($(findstring android, $(ARCH_$(ARCH)_TOOLCHAIN_PREFIX)), )
GLOBAL_COMPILEFLAGS += -Wno-nonnull-compare
endif

include arch/$(ARCH)/rules.mk
include top/rules.mk

# recursively include any modules in the MODULE variable, leaving a trail of included
# modules in the ALLMODULES list
include make/recurse.mk

# any extra top level build dependencies that someone declared
all:: $(EXTRA_BUILDDEPS)

IAR_GLB_DEF += $(GLOBAL_DEFINES)
# add some automatic configuration defines
GLOBAL_DEFINES += \
	PROJECT_$(PROJECT)=1 \
	PROJECT=\"$(PROJECT)\" \
	TARGET_$(TARGET)=1 \
	TARGET=\"$(TARGET)\" \
	PLATFORM_$(PLATFORM)=1 \
	PLATFORM=\"$(PLATFORM)\" \
	ARCH_$(ARCH)=1 \
	ARCH=\"$(ARCH)\" \
	$(addsuffix =1,$(addprefix WITH_,$(ALLMODULES)))

# debug build?
ifneq ($(DEBUG),)
GLOBAL_DEFINES += \
	LK_DEBUGLEVEL=$(DEBUG)
endif

# allow additional defines from outside the build system
ifneq ($(EXTERNAL_DEFINES),)
GLOBAL_DEFINES += $(EXTERNAL_DEFINES)
$(info EXTERNAL_DEFINES = $(EXTERNAL_DEFINES))
endif


# prefix all of the paths in GLOBAL_INCLUDES with -I
GLOBAL_INCLUDES := $(addprefix -I,$(GLOBAL_INCLUDES))
# prefix all of the paths in FreeRTOS_INC with -I
FreeRTOS_INC := $(sort $(addprefix -I,$(FreeRTOS_INC)))


# test for some old variables
ifneq ($(INCLUDES),)
$(error INCLUDES variable set, please move to GLOBAL_INCLUDES: $(INCLUDES))
endif
ifneq ($(DEFINES),)
$(error DEFINES variable set, please move to GLOBAL_DEFINES: $(DEFINES))
endif

# default to no ccache
CCACHE ?=
ifeq ($(call TOBOOL,$(CLANGBUILD)), true)
ifeq ($(CLANG_BINDIR),)
$(error clang directory not specified, please set CLANG_BINDIR)
endif
CC := $(CCACHE) $(CLANG_BINDIR)/clang
GLOBAL_COMPILEFLAGS += -no-integrated-as
else
CC := $(CCACHE) $(TOOLCHAIN_PREFIX)gcc
endif
LD := $(TOOLCHAIN_PREFIX)ld.bfd
AR := $(TOOLCHAIN_PREFIX)ar
OBJDUMP := $(TOOLCHAIN_PREFIX)objdump
OBJCOPY := $(TOOLCHAIN_PREFIX)objcopy
CPPFILT := $(TOOLCHAIN_PREFIX)c++filt
SIZE := $(TOOLCHAIN_PREFIX)size
NM := $(TOOLCHAIN_PREFIX)nm
STRIP := $(TOOLCHAIN_PREFIX)strip

LIBGCC := $(shell $(CC) $(GLOBAL_COMPILEFLAGS) $(ARCH_COMPILEFLAGS) $(THUMBCFLAGS) -print-libgcc-file-name)

# try to have the compiler output colorized error messages if available
export GCC_COLORS ?= 1

# the logic to compile and link stuff is in here
include make/build.mk

DEPS := $(ALLOBJS:%o=%d)

# put all of the global build flags in config.h to force a rebuild if any change
GLOBAL_DEFINES += GLOBAL_INCLUDES=\"$(subst $(SPACE),_,$(GLOBAL_INCLUDES))\"
GLOBAL_DEFINES += GLOBAL_COMPILEFLAGS=\"$(subst $(SPACE),_,$(GLOBAL_COMPILEFLAGS))\"
GLOBAL_DEFINES += GLOBAL_OPTFLAGS=\"$(subst $(SPACE),_,$(GLOBAL_OPTFLAGS))\"
GLOBAL_DEFINES += GLOBAL_CFLAGS=\"$(subst $(SPACE),_,$(GLOBAL_CFLAGS))\"
GLOBAL_DEFINES += GLOBAL_CPPFLAGS=\"$(subst $(SPACE),_,$(GLOBAL_CPPFLAGS))\"
GLOBAL_DEFINES += GLOBAL_ASMFLAGS=\"$(subst $(SPACE),_,$(GLOBAL_ASMFLAGS))\"
GLOBAL_DEFINES += GLOBAL_LDFLAGS=\"$(subst $(SPACE),_,$(GLOBAL_LDFLAGS))\"
GLOBAL_DEFINES += ARCH_COMPILEFLAGS=\"$(subst $(SPACE),_,$(ARCH_COMPILEFLAGS))\"
GLOBAL_DEFINES += ARCH_CFLAGS=\"$(subst $(SPACE),_,$(ARCH_CFLAGS))\"
GLOBAL_DEFINES += ARCH_CPPFLAGS=\"$(subst $(SPACE),_,$(ARCH_CPPFLAGS))\"
GLOBAL_DEFINES += ARCH_ASMFLAGS=\"$(subst $(SPACE),_,$(ARCH_ASMFLAGS))\"

ifneq ($(OBJS),)
$(warning OBJS=$(OBJS))
$(error OBJS is not empty, please convert to new module format)
endif
ifneq ($(OPTFLAGS),)
$(warning OPTFLAGS=$(OPTFLAGS))
$(error OPTFLAGS is not empty, please use GLOBAL_OPTFLAGS or MODULE_OPTFLAGS)
endif
ifneq ($(CFLAGS),)
$(warning CFLAGS=$(CFLAGS))
$(error CFLAGS is not empty, please use GLOBAL_CFLAGS or MODULE_CFLAGS)
endif
ifneq ($(CPPFLAGS),)
$(warning CPPFLAGS=$(CPPFLAGS))
$(error CPPFLAGS is not empty, please use GLOBAL_CPPFLAGS or MODULE_CPPFLAGS)
endif

$(info LIBGCC = $(LIBGCC))
$(info GLOBAL_COMPILEFLAGS = $(GLOBAL_COMPILEFLAGS))
$(info GLOBAL_OPTFLAGS = $(GLOBAL_OPTFLAGS))

# make all object files depend on any targets in GLOBAL_SRCDEPS
$(ALLOBJS): $(GLOBAL_SRCDEPS)

clean: $(EXTRA_CLEANDEPS)
	rm -f $(ALLOBJS) $(DEPS) $(GENERATED) $(OUTBIN) $(SIGNEDBIN) $(OUTELF) $(OUTELF).lst $(OUTELF).debug.lst $(OUTELF).sym $(OUTELF).sym.sorted $(OUTELF).size $(OUTELF).hex $(OUTELF).dump

install: all
	scp $(OUTBIN) 192.168.0.4:/tftproot

$(info ${ARM_ARCH_LEVEL})
include ide/IAR/scripts/Makefile.iar
# generate a config.h file with all of the GLOBAL_DEFINES laid out in #define format
configheader:

$(CONFIGHEADER): configheader
	@$(call MAKECONFIGHEADER,$@,GLOBAL_DEFINES)

# Empty rule for the .d files. The above rules will build .d files as a side
# effect. Only works on gcc 3.x and above, however.
%.d:

ifeq ($(filter $(MAKECMDGOALS), clean), )
-include $(DEPS)
endif

.PHONY: configheader
endif

endif # make spotless