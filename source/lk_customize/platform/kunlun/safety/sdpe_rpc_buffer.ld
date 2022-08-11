/*
 * Link script patch for sdpe rpc buffer sections.
 */
SECTIONS {
    SDPE_BUFFER_OFFSET = %SAF_SDPE_RPC_MEMSIZE% / 8;
    SDPE_BUFFER_BASE = %SAF_SDPE_RPC_MEMBASE% + SDPE_BUFFER_OFFSET;
    SDPE_BUFFER_SIZE = %SAF_SDPE_RPC_MEMSIZE% - SDPE_BUFFER_OFFSET;

    .sdpe_rpc_buf SDPE_BUFFER_BASE (NOLOAD) : {
        . = ALIGN(4);
        __sdpe_rpc_buf_start = .;
        *(.sdpe_rpc_buf)
        __sdpe_rpc_buf_end = .;
    }

    ASSERT(. < SDPE_BUFFER_BASE + SDPE_BUFFER_SIZE, "RPC buffer too large")
}
INSERT BEFORE .tcm;
