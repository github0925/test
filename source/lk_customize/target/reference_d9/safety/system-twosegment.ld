OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)


ENTRY(_start)
SECTIONS
{
    . = %ROMBASE%;
    __rom_start = .;

    .boot.vect : {
        *(.text.boot.vect)
        KEEP(*(.text.boot.vect))
    }

    .text : {
        KEEP(*(.text.boot.vectab1))
        KEEP(*(.text.boot.vectab2))
        KEEP(*(.text.boot))
        EXCLUDE_FILE(%RELOCATED_OBJS%)
        *(.text* .sram.text.glue_7* .gnu.linkonce.t.*)

    }

    .interp : { *(.interp) }
    .hash : { *(.hash) }
    .dynsym : { *(.dynsym) }
    .dynstr : { *(.dynstr) }
    .rel.text : { *(.rel.text) *(.rel.gnu.linkonce.t*) }
    .rela.text : { *(.rela.text) *(.rela.gnu.linkonce.t*) }
    .rel.data : { *(.rel.data) *(.rel.gnu.linkonce.d*) }
    .rela.data : { *(.rela.data) *(.rela.gnu.linkonce.d*) }
    .rel.rodata : { *(.rel.rodata) *(.rel.gnu.linkonce.r*) }
    .rela.rodata : { *(.rela.rodata) *(.rela.gnu.linkonce.r*) }
    .rel.got : { *(.rel.got) }
    .rela.got : { *(.rela.got) }
    .rel.ctors : { *(.rel.ctors) }
    .rela.ctors : { *(.rela.ctors) }
    .rel.dtors : { *(.rel.dtors) }
    .rela.dtors : { *(.rela.dtors) }
    .rel.init : { *(.rel.init) }
    .rela.init : { *(.rela.init) }
    .rel.fini : { *(.rel.fini) }
    .rela.fini : { *(.rela.fini) }
    .rel.bss : { *(.rel.bss) }
    .rela.bss : { *(.rela.bss) }
    .rel.plt : { *(.rel.plt) }
    .rela.plt : { *(.rela.plt) }
    .init : { *(.init) } =0x9090
    .plt : { *(.plt) }

    /* .ARM.exidx is sorted, so has to go in its own output section.  */
    __exidx_start = .;
    .ARM.exidx : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) }
    __exidx_end = .;

    .rodata : ALIGN(4) {
        __rodata_start = .;
        __fault_handler_table_start = .;
        KEEP(*(.rodata.fault_handler_table))
        __fault_handler_table_end = .;
        *(.rodata .rodata.* .gnu.linkonce.r.*)
    }

    /* fake section for .data to anchor off of
     * needed because extra linker scripts tend to insert sections
     * just after .rodata
     */
    .dummy_post_rodata : {
        /* end of rodata, start of data area */
        __rodata_end = . ;
        __rom_end = . ;
        __data_start_rom = .;
    }

    /* in two segment binaries, the data starts at the bottom of ram (MEMBASE)
     * bump us forward to the start of ram
     */
    . = %MEMBASE%;

    .sram.boot.vect : AT( ADDR(.boot.vect) ) {
        . = . + SIZEOF(.boot.vect);
    }

    __sram_boot_vector_start = LOADADDR(.sram.boot.vect);
    __sram_boot_vector_to = ADDR(.sram.boot.vect);
    __sram_boot_vector_to_end = __sram_boot_vector_to + SIZEOF(.sram.boot.vect);



    /* start .data segment, force the physical address to be AT() __data_start_rom */
    .data : AT ( ADDR (.dummy_post_rodata) + SIZEOF (.dummy_post_rodata) ) ALIGN(4) {
        __data_start = .;
        *(.data .data.* .gnu.linkonce.d.*)

    }

    /* code that is located in ram */
    .sram.text : ALIGN(4) {
        %RELOCATED_OBJS_LINK_CMD%

        KEEP (*(.sram.text*))
    }

    __sram_text_end = .;
    __sram_text_start = ADDR(.sram.text);
    __sram_text_load_addr = LOADADDR(.sram.text);
    __sram_text_size = SIZEOF(.sram.text);

    .ctors : ALIGN(4) {
        __ctor_list = .;
        KEEP(*(.ctors .init_array))
        __ctor_end = .;
    }
    __ctor_load_addr = LOADADDR(.ctors);
    .dtors  : ALIGN(4) {
        __dtor_list = .;
        KEEP(*(.dtors .fini_array))
        __dtor_end = .;
    }
    __dtor_load_addr = LOADADDR(.dtors);

    .got : { *(.got.plt) *(.got) }
    .dynamic : { *(.dynamic) }

    /*
     * extra linker scripts tend to insert sections just after .data,
     * so we want to make sure this symbol comes after anything inserted above,
     * but not aligned to the next section necessarily.
     */
    .dummy_post_data : {
        __data_end = .;
    }

    .nocache : ALIGN(1024) {
        _nocacheable_start = .;
        *(.nocache)
        . = ALIGN(1024);
        _nocacheable_end = .;
    }

    /* uninitialized data (in same segment as writable data) */
    .bss : ALIGN(4) {
        KEEP(*(.bss.prebss.*))
        . = ALIGN(4);
        __bss_start = .;
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    }

    _end = .;


    __internal_heap = __tcm_end;

    __internal_heap_size = TCM_SIZE -( __tcm_end - __tcm_start);

    _end_of_ram = %MEMBASE% + %MEMSIZE%;



    /* Add a labelize section of ext memory start addr, which could
     * be locate to extra linker script */
    .ext.mem %EXT_MEM_BASE% (NOLOAD) : {
        . = ALIGN(4);
        __ext_mem_start = .;
        *(.ext.mem .ext.mem.*)
        . = ALIGN(4);
        __ext_mem_end = .;

    }

    __external_heap = ADDR(.ext.mem)+SIZEOF(.ext.mem);

    __external_heap_size = (%EXT_MEM_BASE% + %EXT_MEM_SIZE%) - __external_heap;

    /* Strip unnecessary stuff */
    /DISCARD/ : { *(.comment .note .eh_frame) }
}

ASSERT(_end_of_ram >= _end, "No enough RAM for specified Heap & Stack")
ASSERT(__external_heap_size > 0, "No enough external memory for static definition")
