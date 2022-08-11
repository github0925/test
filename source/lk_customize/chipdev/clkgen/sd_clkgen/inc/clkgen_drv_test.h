
#ifndef __CLKGEN_DRV_TEST_H__
#define __CLKGEN_DRV_TEST_H__
#include "clkgen_drv.h"
bool clkgen_dump_reg_for_test(vaddr_t base,
                              clkgen_debug_module_type module_type, uint32_t slice_idx,
                              uint16_t gating_idx);
bool clkgen_ipslice_readonlyreg_check_test(vaddr_t base,
        uint32_t slice_idx);
bool clkgen_busslice_readonlyreg_check_test(vaddr_t base,
        uint32_t slice_idx);
bool clkgen_coreslice_readonlyreg_check_test(vaddr_t base,
        uint32_t slice_idx);
bool clkgen_other_readonlyreg_check_test(vaddr_t base);
bool clkgen_ipslice_rw_reg_check_test(vaddr_t base, uint32_t slice_idx);
bool clkgen_busslice_rw_reg_check_test(vaddr_t base, uint32_t slice_idx);
bool clkgen_coreslice_rw_reg_check_test(vaddr_t base, uint32_t slice_idx);
bool clkgen_lpgating_rw_reg_check_test(vaddr_t base, uint32_t gating_idx);
bool clkgen_uuuslice_rw_reg_check_test(vaddr_t base,
                                       uint16_t uuu_clock_wrapper_idx);
bool clkgen_other_rw_reg_check_test(vaddr_t base);
#endif
