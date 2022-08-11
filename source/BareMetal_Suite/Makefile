
TOPDIR := $(shell pwd)
BUILDDIR := $(TOPDIR)/build
MKDIR := $(TOPDIR)/mk
CFGDIR := $(TOPDIR)/mk/cfg
BIN2HEX := $(TOPDIR)/tools/bin2hex
SIGN_CMD=$(TOPDIR)/tools/atb_signer
INJ_DS_CMD=$(TOPDIR)/tools/script/inj_ddr_seq.sh
ifneq ($(DIAG),)
DDR_INIT_SEQ_OFF=736
else
DDR_INIT_SEQ_OFF=416
endif

LINK_SCRIPT := $(BINDIR)/lnk.ld

export TOPDIR
export BUILDDIR
export MKDIR
export CFGDIR

include $(MKDIR)/build.mk
BINDIR := $(BUILDDIR)/$(SOC)/$(TGT)
ELF := $(BINDIR)/$(BINARY_NAME)

SUBDIRS := arch lib service driver soc app/$(APP)
ifneq ($(BOARD),)
SUBDIRS += board/$(BOARD)
endif

ifneq ($(SOC),host)
LINKER_SCRIPT=$(BINDIR)/lnk.ld
ifneq ($(SOC),qemu)
LINKER_SCRTIP_IN ?= arch/$(ARCH)/lnk.in.ld
else
LINKER_SCRTIP_IN=arch/$(ARCH)/lnk_qemu.in.ld
endif
ifneq ($(TCH),armcc)
LDFLAGS += -T $(LINKER_SCRIPT) -Wl,-Map=$(ELF).map
else
LINKER_SCRTIP_IN=arch/$(ARCH)/lnk.in.scat
LDFLAGS += --entry=_start --scatter=$(LINKER_SCRIPT) --verbose --callgraph --callgraph_file=$(ELF).callgraph
LDFLAGS += --info=sizes,totals,stack,unused --map --list $(ELF).list
endif
endif

ifneq ($(BT_DEV_LIST),)
SUBDIRS += driver/$(BT_DEV_LIST)
endif

ALL_OBJS = $(sort $(shell find -name "*.o"))

define gcc_elf2bin
	$(AT)$(OBJCOPY) -O binary $(1) -j .checksum -j .text -j .rodata -j .data\
		-j TEST_SECTION -j shell_cmd -j .ram_func -j .ddr_init_seq_sec -j .ddr_fw_sec -j .ddr_diag_sec -j .app_para_section\
        --gap-fill 0xFF $(1).bin
endef

define armcc_elf2bin
	fromelf --bin --bincombined --output $(1).bin $(1)
endef

define pack_peerloaded_img
    echo "packing ap image..."
    dd of=$(1) if=$(2) seek=448 bs=1024
endef

ifneq (${TCH},armcc)
ELF2BIN = gcc_elf2bin
else
ELF2BIN = armcc_elf2bin
endif

define bin2hex_cmdstring
	$(BIN2HEX) rom if=$(ELF).bin width=8 col=1 fmt=hex split=2 part=$(ROM_PART)
endef
define bin2hexbin_cmdstring
	$(AT) dd if=$(ELF).bin of=$(ELF).bin.tmp bs=1024 seek=32
	$(AT) $(BIN2HEX) rom if=$(ELF).bin.tmp width=16 col=1 fmt=bin split=4 part=1 of=$(ELF).bin.hex
	$(AT) rm -rf $(ELF).bin.tmp
endef

ifneq (${CFG},kunlun_fpga)
BIN2HEX_CMD = bin2hex_cmdstring
else
BIN2HEX_CMD = bin2hexbin_cmdstring
endif

#below function is for ddr seq
define remove_line
	$(subst _, ,$1)
endef

define split_addprefix
	$(subst :, _,$(addprefix _,$1))
endef

define join_ddr_params
	$(join $1, $(join $2,$3))
endef

define join_ddr_suffix
	$(if $(2),$(foreach n,$(2), $(if $(filter-out  _,$(word $(n), $(3))),$(join $(word  $(n),$1),$(word $(n), $(3))),$(word  $(n),$1))),$1)
endef

DDR_TP := $(call split_addprefix, $(DDR_TYPE))
DDR_SZ := $(call split_addprefix, $(DDR_SIZE))
DDR_FQ := $(call split_addprefix, $(DDR_FREQ))
DDR_SF := $(call split_addprefix, $(DDR_SUFFIX))
DDR_SURFIX_INDEX := $(shell for x in `seq 1 $(words $(DDR_SF))`; do echo $$x; done)

NULLSTR :=
DDR_SEQ_FILES := $(call join_ddr_params,$(DDR_TP), $(DDR_SZ), $(DDR_FQ))
DDR_SEQ_FILES := $(call join_ddr_suffix,$(DDR_SEQ_FILES),$(DDR_SURFIX_INDEX),$(DDR_SF))
DDR_SEQ_FILES := $(addprefix ddr_init, $(DDR_SEQ_FILES))
DDR_SEQ_FILES := $(join  $(addsuffix /,$(call remove_line,$(DDR_TP))), $(DDR_SEQ_FILES))
DDR_SEQ_FILES := $(addprefix $(TOPDIR)/board/$(BOARD)/ddr_init_script/, $(DDR_SEQ_FILES))
DDR_SEQ_FILES := $(addsuffix .c, $(DDR_SEQ_FILES))
DDR_SEQ_FILES := $(subst $(NULLSTR) $(NULLSTR),:,$(DDR_SEQ_FILES))

DDR_SEQ_BIN   := $(TOPDIR)/build/ddr_fw/ddr_init_seq.bin

ifneq ($(DDR_MASK),)
	DDR_SEQ_MASK  := msk=$(DDR_MASK)
else
	DDR_SEQ_MASK  := msk=0x0
endif

ifneq ($(DDR_ID),)
	DDR_SEQ_ID    := id=$(DDR_ID)
else
	DDR_SEQ_ID    := id=0x0
endif

LIBGCC := $(shell $(CC) $(CFG_CFLAGS) $(ARCH_CFLAGS) $(THUMBCFLAGS) -print-libgcc-file-name)
LIBS += $(LIBGCC)

.PHONY: rom msg_start mk_folders clean_all cscope help clean $(SUBDIRS) $(LINKER_SCRIPT) unified_boot ddr_fw

bin: elf
	@echo "To generate bin..."
	$(AT) $(call ${ELF2BIN},${ELF})
ifneq (${DS},)
	@echo "To inject ddr init seq..."
	DDR_INIT_SEQ_OFF=${DDR_INIT_SEQ_OFF} ${INJ_DS_CMD} ${DS} ${ELF}.bin
endif
ifneq (${PL},)
	@echo "To pack ap image ${LOAD_BIN_NAME}... "
	$(AT) $(call pack_peerloaded_img,${ELF}.bin,build/kunlun/ap/${LOAD_BIN_NAME})
endif
	@echo "To sign bin..."
	${SIGN_CMD} sign --rcp key=test/keys/TestRSA1024_ossl.pem rot=1 --iib img=${ELF}.bin to=$(ROM_BASE) entry=$(ROM_BASE) --dgst sha256 --of ${ELF}.bin.signed.rsa1024

elf: msg_start mk_folders $(SUBDIRS) $(LINKER_SCRIPT)
	@echo "$(LINKER_SCRIPT)"
	@echo "To link objs..."
	$(LD) $(LDFLAGS) $(ALL_OBJS) $(LIBS) $(LIB_CFILES) $(LIB_HDRs) -o $(ELF)
	$(AT) find -name "*.c.chk" | xargs cat >$(ELF).cppcheck.chk
	$(AT) find -name "*.c.chk" | xargs rm -rf

run: elf
	@echo "To run $(ELF)"
	@$(ELF)

msg_start:
	@echo "To build ${ELF}"

mk_folders:
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(BUILDDIR)/$(SOC)
	@mkdir -p $(BUILDDIR)/$(SOC)/$(TGT)

$(SUBDIRS):
	$(AT)echo "Enter $(CURDIR)/$@"
	$(AT)make $(SILENT) -C $@

$(LINKER_SCRIPT): $(LINKER_SCRTIP_IN)
	@echo "To generate link script..."
	@echo "source link script:"$(LINKER_SCRTIP_IN)
	$(AT) -rm $@ -rf
	$(AT)sed "s/%ROM_BASE%/$(ROM_BASE)/g; s/%ROM_SIZE%/$(ROM_SIZE)/g; s/%ROMPAD_BASE%/$(ROMPAD_BASE)/g; s/%ROMPAD_SIZE%/$(ROMPAD_SIZE)/g; s/%RAM_BASE%/$(RAM_BASE)/g; s/%RAM_SIZE%/$(RAM_SIZE)/g; s/%NMEM_BASE%/$(NMEM_BASE)/g; s/%RAM2_BASE%/$(RAM2_BASE)/g; s/%STACK_SZ%/$(STACK_SZ)/g; s/%MEMBASE%/$(MEMBASE)/g; s/%MEMSIZE%/$(MEMSIZE)/g;" < $< > $@

cscope:
	@echo "To build CSCOPE..."
	@find ${TOPDIR} -name "*.[chsS]" > cscope.files
	@cscope -b -q -k

clean:
	find -L -name "*.[od]" | xargs rm -rf
	find -L -name "*.gc*" | xargs rm -rf
	find -L -name "*.c.orig" | xargs rm -rf

clean_all: clean
	-rm $(BUILDDIR) -rf

unified_boot: bin
	@echo "Build unified boot binary files done"

ddr_fw:
	@echo "To build DDR FW"
	$(AT)make -C tools/bin_packer
	#clear ld flags for compile error when build for yocto
	LDFLAGS=;\
	tools/script/prepare_ddr_fw.sh
	cp build/ddr_fw/$(DDR_FW)_training_fw.pac  build/ddr_fw/ddr_fw.bin
	$(AT)make -C tools/ddr_seq_parser
	tools/ddr_seq_parser/ddr_seq_parser if=$(DDR_SEQ_FILES) $(DDR_SEQ_ID) $(DDR_SEQ_MASK) of=$(DDR_SEQ_BIN)

help:
	@echo "Usage: "
	@echo " make CFG=[cfg] [D=1] [T=xxx] AP=[1/2] DS=ddr_init_file mk_tgt"
	@echo "     CFG        kunlun_sec/kunlun_ap, etc"
	@echo "     D          if provisioned, atb debug version will be built"
	@echo "     AP         1 for AP1 and 2 for AP2, valid when CFG=kunlun_sec"
	@echo "     mk_tgt     the target to be 'make'"
	@echo "                    elf/bin/hex   to specify the output file desired"
	@echo " To clean the build, run: make clean_all"
	@echo " To create cscope files, run: make cscope"
