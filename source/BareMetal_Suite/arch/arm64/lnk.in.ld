OUTPUT_FORMAT("elf64-littleaarch64", "elf64-bigaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(_start)

MEMORY
{
    ROM (r) :  ORIGIN = %ROM_BASE%, LENGTH = %ROM_SIZE%
    RAM (rw) : ORIGIN = %RAM_BASE%, LENGTH = %RAM_SIZE%
}

SECTIONS
{
    . = %ROM_BASE%;

    .text : {
        KEEP(*(.text.start))
        KEEP(*(.text.vector))
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

    .rodata : ALIGN(8) {
        __rodata_start = .;
        *(.rodata .rodata.*)
    } > ROM

    .dummy_post_rodata : {
        /* end of rodata, start of data area */
        __rodata_end = .;
        __data_start_rom = .;
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

    .data : AT(ALIGN(__data_start_rom, 32)) ALIGN(32) {
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

    /* Strip unnecessary stuff */
    /DISCARD/ : { *(.comment .note .eh_frame) }
}
