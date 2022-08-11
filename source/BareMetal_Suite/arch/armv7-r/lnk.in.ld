OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY
{
    ROM (r) :  ORIGIN = %ROM_BASE%, LENGTH = %ROM_SIZE%
    RAM (rw) : ORIGIN = %RAM_BASE%, LENGTH = %RAM_SIZE%
    RAM2 (rw) : ORIGIN = %RAM2_BASE%, LENGTH = 0x80
}

SECTIONS
{
    . = %ROM_BASE%;
    __rom_start = . ;

    .text : {
        KEEP(*(.text.vector))
        KEEP(*(.text.test_section))
        *(.text*)
    } > ROM

    TEST_SECTION : ALIGN(8) {
        __start_TEST_SECTION = .;
        KEEP(*(TEST_SECTION*))
        __stop_TEST_SECTION = .;
    } > ROM

    shell_cmd : ALIGN(8) {
        __start_shell_cmd = .;
        KEEP(*(shell_cmd*))
        __stop_shell_cmd = .;
    } > ROM

    .rodata : ALIGN(4) {
        __rodata_start = .;
        *(.rodata .rodata.*)
    } > ROM

    .ddr_fw_sec : ALIGN(4) {
        __ddr_fw_start = .;
        KEEP(*(ddr_fw_sec*))
        . = ALIGN(8);
        __ddr_fw_end = .;
     } > ROM

    .dummy_post_rodata : {
        /* end of rodata, start of data area */
        __rodata_end = .;
        __data_start_rom = .;
    } > ROM

    .app_para_section (%ROM_BASE% + %ROM_SIZE% - 0x8000 - 0x20) : ALIGN(4) {
        KEEP(*(.app_para_section))
    } > ROM 

    .ddr_init_seq_sec (%ROM_BASE% + %ROM_SIZE% - 0x8000) : ALIGN(4) {
        __ddr_init_seq_start = .;
        KEEP(*(ddr_init_seq_sec*))
        . = ALIGN(8);
        __ddr_init_seq_end = .;
    } > ROM

    .checksum (%ROM_BASE% + %ROM_SIZE% - 32) : ALIGN(4) {
        KEEP(*(.checksum))
        __rom_end = . ;
    } > ROM

    . = %RAM_BASE%;

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

    .data : AT(ALIGN(__data_start_rom, 8)) ALIGN(32) {
        __data_start = .;
        *(.data .data.*)
        . = ALIGN(32);
        __data_end = .;
    }
    __load_data_start = LOADADDR(.data);

    __heap_start = .;

    .stack (%RAM_BASE% + %RAM_SIZE% - %STACK_SZ%) : AT(%RAM_BASE% + %RAM_SIZE% - %STACK_SZ%) ALIGN(32) {
        __stack_start = .;
        . += %STACK_SZ%;
        . = ALIGN(32);
        __stack_end = .;
    } > RAM

    .tb_trigger %RAM2_BASE% + 0x40:  AT(%RAM2_BASE% + 0x40) ALIGN(32) {
        __tb_trigger_start = .;
        . += 0x40;
        __tb_trigger_end = .;
    } > RAM2

    /* Strip unnecessary stuff */
    /DISCARD/ : { *(.comment .note .eh_frame) }
}
