OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

SECTIONS
{
    . = 0x1C0000;
    __rom_start = . ;

    .text : {
        KEEP(*(.text.vector))
        KEEP(*(.text.test_section))
        *(.text*)
    }

    TEST_SECTION : ALIGN(8) {
        __start_TEST_SECTION = .;
        KEEP(*(TEST_SECTION*))
        __stop_TEST_SECTION = .;
    }

    shell_cmd : ALIGN(8) {
        __start_shell_cmd = .;
        KEEP(*(shell_cmd*))
        __stop_shell_cmd = .;
    }

    .rodata : ALIGN(4) {
        __rodata_start = .;
        *(.rodata .rodata.*)
    }

    .ddr_fw_sec : ALIGN(4) {
        __ddr_fw_start = .;
        KEEP(*(ddr_fw_sec*))
        . = ALIGN(8);
        __ddr_fw_end = .;
    }

    .dummy_post_rodata : {
        /* end of rodata, start of data area */
        __rodata_end = .;
    }

    .data : ALIGN(32){
        __data_start = .;
        *(.data .data.*)
        . = ALIGN(32);
        __data_end = .;
    }
    __load_data_start = LOADADDR(.data);

    .app_para_section (0x1C0000 + 0x10000 - 0x4000 - 0x20) : ALIGN(4) {
        KEEP(*(.app_para_section))
    }

    .ddr_init_seq_sec (0x1C0000 + 0x10000 - 0x4000) : ALIGN(4) {
        __ddr_init_seq_start = .;
        KEEP(*(ddr_init_seq_sec*))
        . = ALIGN(8);
        __ddr_init_seq_end = .;
    }

    .checksum (0x1C0000 + 0x10000 - 32) : ALIGN(4) {
        KEEP(*(.checksum))
        __rom_end = . ;
    }

    .bss : ALIGN(32) {
        __bss_start = .;
        KEEP(*(.bss.prebss.*))
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(32);
        __bss_end = .;
    }
    .bss_2 : ALIGN(32) {
        __bss_2_start = .;
        *(.bss_2)
        . = ALIGN(32);
        __bss_2_end = .;
    }

    .stack  : ALIGN(32) {
        __stack_start = .;
        . += 0x6000;
        . = ALIGN(32);
        __stack_end = .;
    }

    .tb_trigger : ALIGN(32) {
        __tb_trigger_start = .;
        . += 0x40;
        __tb_trigger_end = .;
    }

    __heap_start = .;
    _end = .;
    . = 0x1C0000 + 0x40000;
    _end_of_ram = .;


    /* Strip unnecessary stuff */
    /DISCARD/ : { *(.comment .note .eh_frame) }
}
