
$(C_OBJ): %.o:%.c
	@echo "Compiling $<"
	$(AT)$(SCAN_BUILD) $(CC) $(COMMON_CFLAGS) $(ARCH_CFLAGS) $(MODULE_CFLAGS) $(THUMBCFLAGS) $(CFG_CFLAGS) $(CFLAGS_STK_CHK) $(CINCLUDE) $(CDEFINES) -MMD -o $@ -c $<
	#$(AT)$(CPPCHK) $(CPPCHKFLAGS) $(CINCLUDE) $(CDEFINES) --output-file=$<.chk $<

$(S_OBJ): %.o: %.S
	@echo "Compiling $<"
	$(AT)$(CC) $(COMMON_CFLAGS) $(ARCH_CFLAGS) $(MODULE_CFLAGS) $(CFG_CFLAGS) $(CFLAGS_STK_CHK) $(CINCLUDE) $(CDEFINES) -DASSEMBLY -o $@ -c $<
