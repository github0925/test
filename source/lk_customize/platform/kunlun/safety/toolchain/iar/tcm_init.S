#include <asm.h>
    SECTION .text:CODE:ROOT(2)
    PUBLIC tcm_init

tcm_init
    push {r4,r5,r6,r7}
    mov r6,r0
    mov r7,r1
    ; mrc p15, 0, r0, c0, c0, 2   // Read TCMTR (TCM Type Register)
    ; // r0: ATCM & BTCM availability
    ; mrc p15, 0, r0, c9, c1, 1   // Read ATCM Region Register
    ; // r0 now contains ATCM size in bits[6:2]
    ; mrc p15, 0, r0, c9, c1, 0   // Read BTCM Region Register
    ; // r0 now contains BTCM size in bits[6:2]
    ; // Write to TCMTR.size will be ignored
    orr r0, r0, #1  // Enable ATCM
    mcr p15, 0, r0, c9, c1, 1
    orr r1, r1, #1  // Enable BTCM
    mcr p15, 0, r1, c9, c1, 0

    ; r2 = tcmA_base + tcmA_size = tcmA_end
    add r2,r6,r2
    ; r3 = tcmB_base + tcmB_size = tcmB_end
    add r3,r7,r3
    ;combine to 64 bit
    mov r4,#0
    mov r5,#0

_Ltcm_clrA:
    cmp r6,r2
    bhs _Ltcm_clrB
    stmia r6!, {r4-r5}
    b _Ltcm_clrA

_Ltcm_clrB:
    cmp r7,r3
    bhs .Ltcm_clr_done
    stmia r7!, {r4-r5}
    b _Ltcm_clrB

.Ltcm_clr_done:

    pop {r4,r5,r6,r7}
    bx lr

    END
