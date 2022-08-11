########################################################################### ###
#@File    Linux.mk
#@Copyright     Copyright (c) Imagination Technologies Ltd. All Rights Reserved
#@License       Strictly Confidential.
### ###########################################################################

modules := ocl_unit_test

ocl_unit_test_type := executable
ocl_unit_test_src := ocltest.c
ocl_unit_test_includes := include/$(PVR_ARCH) include include/khronos $(OUT)/include
ocl_unit_test_extlibs := m
ocl_unit_test_cflags :=

#Allow deprecated APIs. Avoids compilation failures with -Wdeprecated-declarations
ocl_unit_test_cflags += \
-DCL_USE_DEPRECATED_OPENCL_1_0_APIS \
-DCL_USE_DEPRECATED_OPENCL_1_1_APIS \
-DCL_USE_DEPRECATED_OPENCL_1_2_APIS

ifneq ($(SUPPORT_ANDROID_PLATFORM),1)
 ifneq ($(CROSS_COMPILE),)
  ocl_unit_test_extlibs += stdc++
 endif
endif

cl_includes_dir += CL

# We need to set compiler option -mfloat-abi=softfp to enable
# fesetround works on android
ifeq ($(SUPPORT_ANDROID_PLATFORM),1)
 ifeq ($(ARCH),arm)
  ocl_unit_test_cflags += -mfloat-abi=softfp
 endif
endif

ifneq (,$(filter $(PVR_BUILD_DIR),vp_linux emu_linux tc_linux fpga_linux))
 ocl_unit_test_cflags += -DCUT_DOWN_UNIT_TEST
endif

ocl_unit_test_libs = $(call module-library,opencl)
