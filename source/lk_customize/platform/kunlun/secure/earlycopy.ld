/*
 * Link script patch for early copy sections.
 */
SECTIONS {
    /* TCM code and data sections. */
    TCM_BASE = 0x4b0000;
    TCM_SIZE = 0x20000;

    . = TCM_BASE;
    __LMA_tcm = ALIGN(LOADADDR(.bss) + SIZEOF(.bss), 4);

    .tcm : AT (__LMA_tcm) {
        __tcm_start = .;
        *(.tcmcode)

        . = ALIGN(4);
        *(.tcmdata)

        __tcm_end = ALIGN(4);
        ASSERT(__tcm_end - __tcm_start <= TCM_SIZE, "TCM memory too large!");
    }

    /* Add other early copy sections here.
     *
     *   . = XXX_MEMORY_BASE;
     *   __LMA_XXX_MEMORY = ALIGN(LOADADDR(.tcm) + SIZEOF(.tcm), 4);
     *   .xxx : AT (__LMA_XXX_MEMORY) {
     *        ...
     *   }
     */
}
INSERT BEFORE .text;

SECTIONS {
    .early_copy : ALIGN(4) {
        __earlycopy_start = .;

        /* VMA, size and LMA of all early copy sections. */
        LONG(ADDR(.tcm))
        LONG(SIZEOF(.tcm))
        LONG(LOADADDR(.tcm))

        __earlycopy_end = .;
    }
}
INSERT AFTER .rodata;
