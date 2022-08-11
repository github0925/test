SECTIONS {
	.slt_module_test : {
        __slt_module_test = .;
        KEEP (*(.slt_module_test))
        __slt_module_test_end = .;
    }
}
INSERT AFTER .rodata;
