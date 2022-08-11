1、put this i2c_demo folder into "SemiDrive_BSP/application/sample"

2、change SemiDrive_BSP/application/rules.mk add "MODULE_DEPS += application/sample/i2c"
--- a/application/rules.mk
+++ b/application/rules.mk
@@ -1,6 +1,8 @@
 LOCAL_DIR := $(GET_LOCAL_DIR)
 MODULE := $(LOCAL_DIR)

+MODULE_DEPS += application/sample/i2c
+
ifeq ($(SUPPORT_WDG_SYSTEM_INIT),true)
MODULE_DEPS += application/system/wdg
endif

3、run test cmd "i2c_demo"
