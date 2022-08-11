
-include $(CFGDIR)/$(CFG).cfg
-include $(TOPDIR)/arch/$(ARCH)/compiler.mk
-include $(TOPDIR)/arch/$(ARCH)/arch.mk
-include $(TOPDIR)/arch/$(ARCH)/$(CPU)/cpu.mk

CPPCHK = $(TOPDIR)/../rom_tools/cppcheck-1.87/cppcheck
CPPCHKFLAGS = --inconclusive --enable=warning,style --std=c99 -q
#CPPCHKFLAGS = --enable=all --std=c99 -q

ARCH_def = $(subst -,_,$(ARCH))
CPU_def = $(subst -,_,$(CPU))
CDEFINES += -DARCH_$(ARCH_def) -DCPU_$(CPU_def) -DSOC_$(SOC) -DTGT_$(TGT) -DTC_$(TC)
CDEFINES += -DATB_ROMCODE

ifdef D
CDEFINES += -DDEBUG_ENABLE
endif

ifdef V
AT :=
SILENT :=
else
AT := @
SILENT := -s
endif

ifdef T
TEST=$(T)
CDEFINES += -DTEST_ITEM=$(TEST)
endif
ifdef BOARD
CDEFINES += -DBOARD -DBOARD_$(BOARD)
endif

ifeq ($(SDCARD_BOOT),1)
CDEFINES += -DSDCARD_BOOT
endif

ifeq ($(DDR_FW),lpddr4)


CDEFINES += -DLPDDR4_USED
endif
ifeq ($(DDR_FW),ddr4)

CDEFINES += -DDDR4_USED
endif

CINCLUDE += -I$(TOPDIR)\
			-I$(TOPDIR)/include/\
			-I$(TOPDIR)/include/common\
			-I$(TOPDIR)/include/crypto\
			-I$(TOPDIR)/arch/\
			-I$(TOPDIR)/arch/$(ARCH)/\
			-I$(TOPDIR)/arch/$(ARCH)/include/\
			-I$(TOPDIR)/atb/\
			-I$(TOPDIR)/lib/\
			-I$(TOPDIR)/lib/mini_libc/\
			-I$(TOPDIR)/lib/mem_image/\
			-I$(TOPDIR)/lib/str/\
			-I$(TOPDIR)/lib/virt_uart/\
			-I$(TOPDIR)/driver/\
			-I$(TOPDIR)/bt_dev/\
			-I$(TOPDIR)/service/\
			-I$(TOPDIR)/soc/\
			-I$(TOPDIR)/crypto/\
			-I$(TOPDIR)/crypto/mbedtls/include\
			-I$(TOPDIR)/soc/$(SOC)\
			-I$(TOPDIR)/soc/$(SOC)/$(TGT)\
			-I$(TOPDIR)/driver/ddr/dw_umctl2/inc\
			-I$(TOPDIR)/driver/fuse_ctrl\
			-I$(TOPDIR)/board/$(BOARD) \
			-I$(TOPDIR)/board/$(BOARD)/$(TGT) \
			-I$(TOPDIR)/lib/cksum/include/\
			-I$(TOPDIR)/lib/partition/include/\
			-I$(TOPDIR)/lib/storage_device/include/\
			-I$(TOPDIR)/driver/mailbox/\
			-I$(TOPDIR)/driver/scr/\
			-I$(TOPDIR)/lib/boardinfo/\
			-I$(TOPDIR)/driver/gic\
			-I$(TOPDIR)/driver/gic/common\
			-I$(TOPDIR)/driver/bt_dev/usb/inc\
			-I$(TOPDIR)/driver/bt_dev/dw_usb/include\
			-I$(TOPDIR)/driver/bt_dev/usb/class/ezusb/\
