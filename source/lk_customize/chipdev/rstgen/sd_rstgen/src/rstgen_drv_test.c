//*****************************************************************************
//
// rstgen_program.c - Driver for the rstgen Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup rstgen_program api
//! @{
//
//*****************************************************************************
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <debug.h>
#include <trace.h>
#include <reg.h>
#include "__regs_base.h"
#include "target_res.h"
#include "rstgen_drv.h"
#include "rstgen_drv_test.h"
#include <platform/gic.h>
#if WITH_SMP
#include <kernel/mp.h>
#endif
#define RSTGEN_GICBASE(n)  (n)
#define GICREG(gic, reg) (*REG32(RSTGEN_GICBASE(gic) + (reg)))
#define GICD_IPRIORITYR(n)      (GICD_OFFSET + 0x400 + (n) * 4)

#ifndef GIC1_BASE
#define GIC1_BASE (0xf5400000u)
#endif
#ifndef GIC2_BASE
#define GIC2_BASE (0xf5410000u)
#endif
#ifndef GIC3_BASE
#define GIC3_BASE (0xf5420000u)
#endif
#ifndef GIC4_BASE
#define GIC4_BASE (0)
#endif
#ifndef GIC5_BASE
#define GIC5_BASE (0)
#endif

uint8_t rstgen_gic_get_priority(vaddr_t gic_addr,u_int irq)
{
    u_int reg = irq / 4;
    u_int shift = 8 * (irq % 4);
    return (GICREG(gic_addr, GICD_IPRIORITYR(reg)) >> shift) & 0xff;
}

uint8_t rstgen_gic_set_priority_locked(vaddr_t gic_addr,u_int irq, uint8_t priority)
{
    u_int reg = irq / 4;
    u_int shift = 8 * (irq % 4);
    u_int mask = 0xff << shift;
    uint32_t regval;

    regval = GICREG(gic_addr, GICD_IPRIORITYR(reg));
    LTRACEF("irq %i, old GICD_IPRIORITYR%d = %x\n", irq, reg, regval);
    regval = (regval & ~mask) | ((uint32_t)priority << shift);
    GICREG(gic_addr, GICD_IPRIORITYR(reg)) = regval;
    LTRACEF("irq %i, new GICD_IPRIORITYR%d = %x, req %x\n",
            irq, reg, GICREG(gic_addr, GICD_IPRIORITYR(reg)), regval);

    return 0;
}

#if WITH_SMP
int rstgen_mp_is_cpu_active(uint cpu)
{
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "MODULE_READ_DATA cpu test active_cpus:0x%x\n",mp.active_cpus);
    return mp.active_cpus & (1 << cpu);
}
#endif
//*****************************************************************************
//
//! rstgen_rstgen_dump_all_reg_for_test
//!
//! \param ulBase is the base address of the rstgen timer module.
//! \core_idx is core index value
//! \module_idx is module index
//! \iso_idx is iso index
//!
//! This function is dump all register value
//!
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool rstgen_dump_all_reg_for_test(vaddr_t base,uint32_t core_idx,uint32_t module_idx,uint32_t iso_idx)
{
    uint32_t reg_value = 0;
    vaddr_t core_rst_en_addr;
    vaddr_t core_sw_rst_en_addr;
    vaddr_t sw_self_rst_addr;
    vaddr_t sw_oth_rst_addr;
    vaddr_t rst_sta_addr;
    vaddr_t general_rst_addr;
    vaddr_t iso_en_addr;
    vaddr_t module_rst_addr;

    core_rst_en_addr = base+SOC_RSTGEN_REG_MAP(RSTGEN_CORE_RST_EN_OFF(core_idx));;
    core_sw_rst_en_addr = base+SOC_RSTGEN_REG_MAP(RSTGEN_CORE_SW_RST_OFF(core_idx));;
    sw_self_rst_addr = base+SOC_RSTGEN_REG_MAP(RSTGEN_SW_SELF_RST_OFF);
    sw_oth_rst_addr = base+SOC_RSTGEN_REG_MAP(RSTGEN_SW_OTH_RST_OFF);
    rst_sta_addr = base+SOC_RSTGEN_REG_MAP(RSTGEN_RST_STA_OFF);
    general_rst_addr = base+SOC_RSTGEN_REG_MAP(RSTGEN_GENERAL_RST_OFF);
    iso_en_addr  = base+SOC_RSTGEN_REG_MAP(RSTGEN_ISO_EN_OFF(iso_idx));
    module_rst_addr = base+SOC_RSTGEN_REG_MAP(RSTGEN_MODULE_RST_OFF(module_idx));

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "#######################################\n");
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "dump_all_reg:\n");
    reg_value = readl(base);
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "glb_rst_en:0x%x\n",reg_value);
    reg_value = readl(core_rst_en_addr);
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "core_rst_en:0x%x\n",reg_value);
    reg_value = readl(core_sw_rst_en_addr);
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "core_sw_rst_en:0x%x\n",reg_value);
    reg_value = readl(sw_self_rst_addr);
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "sw_self_rst:0x%x\n",reg_value);
    reg_value = readl(sw_oth_rst_addr);
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "sw_oth_rst:0x%x\n",reg_value);
    reg_value = readl(rst_sta_addr);
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rst_sta:0x%x\n",reg_value);
    reg_value = readl(general_rst_addr);
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "general_rst:0x%x\n",reg_value);
    reg_value = readl(iso_en_addr);
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "iso_en:0x%x\n",reg_value);
    reg_value = readl(module_rst_addr);
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "module_rst:0x%x\n",reg_value);
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "#######################################\n");
    return true;
}

//*****************************************************************************
//
//! crstgen_clear_rst_sta .
//!\param base   rstgen peripheral base address
//! This function is used to clear rstgen reset status register
//!
//! \return reset status
//
//*****************************************************************************
bool rstgen_clear_rst_sta(vaddr_t base)
{
    vaddr_t rst_sta_addr;
    uint32_t rstgen_rst_sta = 0;

    rst_sta_addr = base+SOC_RSTGEN_REG_MAP(RSTGEN_RST_STA_OFF);

    rstgen_rst_sta = readl(rst_sta_addr);
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_get_rst_sta rst_sta before:0x%x\n",rstgen_rst_sta);
    writel(0x0,rst_sta_addr);
    spin(1);
    rstgen_rst_sta = readl(rst_sta_addr);
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_get_rst_sta rst_sta after:0x%x\n",rstgen_rst_sta);

    return true;
}

//*****************************************************************************
//
//! rstgenReadonlyRegCheckTest  test1
//!
//! \param ulBase is the base address of the rstgen timer module.
//! \core_idx is core index value
//!
//! This function is test watchdog read only register read it must match with expected value with the reset value
//!
//! \return true is check ok,else return false.
//
//Need modify .tcl force  emu_top.dut.safe_ss.efusec.fuse_reg_ctrl.fuse_ctrl_apb_reg.q_fuse_word_data[5263] = 1'b1; close romcode run
//*****************************************************************************
bool rstgen_core_readonlyreg_check_test(vaddr_t base,uint32_t core_idx)
{
    bool ret = true;
    uint32_t core_slice_idx = 0;
    vaddr_t core_rst_addr;
    vaddr_t core_sw_rst_addr;
    uint32_t reg_read = 0;

    core_slice_idx = (core_idx/2) - (CORE_RST_EN/4);
    // Check the arguments.
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_core_readonlyreg_check_test start core_idx:%d\n",core_slice_idx);
    core_rst_addr = base+SOC_RSTGEN_REG_MAP(RSTGEN_CORE_RST_EN_OFF(core_slice_idx));
    core_sw_rst_addr = base+SOC_RSTGEN_REG_MAP(RSTGEN_CORE_SW_RST_OFF(core_slice_idx));
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_core_readonlyreg_check_test core_rst_addr:0x%llx,core_sw_rst_addr:0x%llx \n",(uint64_t)core_rst_addr,(uint64_t)core_sw_rst_addr);

    if(((base == RSTGEN_SEC_BASE) && ((core_slice_idx == drv_core_cpu1_core_all) || (core_slice_idx == drv_core_cpu2_core)))
        || ((base == RSTGEN_SAF_BASE) && (core_slice_idx == drv_core_cr5_mp))){
        //If it is runing then core_rst=0x40000001,core_sw_rst = 0x60000001
         reg_read =readl(core_rst_addr);
         LTRACEF("rstgen core rst sw rst enable status value21:0x%x\n",reg_read);
         if(reg_read != 0x40000001){
             LTRACEF("rstgen core rst sw rst enable status value is error:0x%x\n",reg_read);
             ret = false;
         }

         reg_read =readl(core_sw_rst_addr);
         LTRACEF("rstgen core rst sw rst enable status value22:0x%x\n",reg_read);
         if(reg_read != 0x60000001){
             LTRACEF("rstgen core rst sw rst static rst status value is error:0x%x\n",reg_read);
             ret = false;
         }
    }else{
        //core_cr5_se/core_cr5_mp:if fuse PARALLEL_BOOT_DISABLE = 0 then core_rst=0x0,core_sw_rst = 0x40000000,else core_rst=00,core_sw_rst = 0x0
        reg_read =readl(core_rst_addr);
        LTRACEF("rstgen core rst sw rst enable status value11:0x%x\n",reg_read);
        if(reg_read != 0x0){
            LTRACEF("rstgen core rst sw rst enable status value is error:0x%x\n",reg_read);
            ret = false;
        }

        reg_read =readl(core_sw_rst_addr);
        LTRACEF("rstgen core rst sw rst enable status value12:0x%x\n",reg_read);
        if(reg_read != 0x0){
            LTRACEF("rstgen core rst sw rst static rst status value is error:0x%x\n",reg_read);
            ret = false;
        }
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_core_readonlyreg_check_test end ret:%d\n",ret);
    rstgen_dump_all_reg_for_test((vaddr_t)base,core_slice_idx,0,0);
    return ret;
}

//*****************************************************************************
//
//! rstgenReadonlyRegCheckTest  test2
//!
//! \param ulBase is the base address of the rstgen timer module.
//! \module_idx is module index
//!
//! This function is test watchdog read only register read it must match with expected value with the reset value
//!
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool rstgen_module_readonlyreg_check_test(vaddr_t base,uint32_t module_idx)
{
    bool ret = true;
    vaddr_t module_rst_addr;
    uint32_t reg_read = 0;
    uint32_t module_slice_idx = 0;

    module_slice_idx = module_idx - (MODULE_RST/4);
    module_rst_addr = base+SOC_RSTGEN_REG_MAP(RSTGEN_MODULE_RST_OFF(module_slice_idx));
    // Check the arguments.
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_module_readonlyreg_check_test start.....\n");
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_module_readonlyreg_check_test module_rst_addr:0x%llx \n",(uint64_t)module_rst_addr);
    reg_read =readl(module_rst_addr);

    if((reg_read & RSTGEN_MODULE_RST_STA_MASK) != RSTGEN_MODULE_RST_STA_MASK){
        LTRACEF("rstgen module reset status value is error:0x%x\n",reg_read);
        ret = false;
    }

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_module_readonlyreg_check_test end ret:%d\n",ret);
    rstgen_dump_all_reg_for_test((vaddr_t)base,0,module_slice_idx,0);
    return ret;
}
//*****************************************************************************
//! rstgen_global_rw_reg_check_test test3
//!
//! \param ulBase is the base address of the rstgen timer module.
//!
//! This function is test rstgen global regiter test
//!	use 0xffffffff or 0x00000000 write and immediately read it should be equal with the write data
//!
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool rstgen_global_rw_reg_check_test(vaddr_t base)
{
    bool ret = true;
    vaddr_t global_rst_addr;
    uint32_t reg_read = 0;
    uint32_t reg_read_old = 0;
    uint32_t reg_write = 0;

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_rw_reg_check_test start................\n");

    /*global reg test begin*/
    //global rst enable register test
    global_rst_addr = base;
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_rw_reg_check_test global_rst_addr:0x%llx \n",(uint64_t)global_rst_addr);
    reg_read =readl(global_rst_addr);
    reg_read_old = reg_read;
    if((reg_read & 0xffffffff) != 0x00){
        LTRACEF("rstgen global rst en reg value is error:0x%x\n",reg_read);
        rstgen_dump_all_reg_for_test((vaddr_t)base,0,0,0);
        ret = false;
    }
    reg_write = 0x000007f8;
    writel(reg_write,global_rst_addr);
    spin(10);
    reg_read =readl(global_rst_addr);
    if((reg_read & 0xffffffff) != 0x000007f8 ){
        LTRACEF("rstgen global rst en reg write is error:0x%x\n",reg_read);
        rstgen_dump_all_reg_for_test((vaddr_t)base,0,0,0);
        ret = false;
    }
    writel(reg_read_old,global_rst_addr);

    //sw self rst test
    global_rst_addr = base + SOC_RSTGEN_REG_MAP(RSTGEN_SW_SELF_RST_OFF);
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_rw_reg_check_test global_rst_addr:0x%llx \n",(uint64_t)global_rst_addr);
    reg_read =readl(global_rst_addr);
    reg_read_old = reg_read;
    if(((reg_read & 0x1) != 0x0)|| ((reg_read & 0x80000000) != 0)){
        LTRACEF("rstgen sw self rst test reg value is error:0x%x\n",reg_read);
        rstgen_dump_all_reg_for_test((vaddr_t)base,0,0,0);
        ret = false;
    }
    reg_write = 0x7fffffff;
    writel(reg_write,global_rst_addr);
    spin(10);
    reg_read =readl(global_rst_addr);
    if((reg_read & 0x1) != 0x1){
        LTRACEF("rstgen sw self rst test reg write is error:0x%x\n",reg_read);
        rstgen_dump_all_reg_for_test((vaddr_t)base,0,0,0);
        ret = false;
    }
    writel(reg_read_old,global_rst_addr);

    //sw other rst test
    global_rst_addr = base + SOC_RSTGEN_REG_MAP(RSTGEN_SW_OTH_RST_OFF);
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_rw_reg_check_test global_rst_addr:0x%llx \n",(uint64_t)global_rst_addr);
    reg_read =readl(global_rst_addr);
    reg_read_old = reg_read;
    if(((reg_read & 0x1) != 0x0)|| ((reg_read & 0x80000000) != 0)){
        LTRACEF("rstgen sw other rst test reg value is error:0x%x\n",reg_read);
        rstgen_dump_all_reg_for_test((vaddr_t)base,0,0,0);
        ret = false;
    }
    reg_write = 0x7fffffff;
    writel(reg_write,global_rst_addr);
    spin(10);
    reg_read =readl(global_rst_addr);
    if((reg_read & 0x1) != 0x1){
        LTRACEF("rstgen sw other rst test reg write is error:0x%x\n",reg_read);
        rstgen_dump_all_reg_for_test((vaddr_t)base,0,0,0);
        ret = false;
    }
    writel(reg_read_old,global_rst_addr);

    //rst status test
    global_rst_addr = base + SOC_RSTGEN_REG_MAP(RSTGEN_RST_STA_OFF);
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_rw_reg_check_test global_rst_addr:0x%llx \n",(uint64_t)global_rst_addr);
    reg_read =readl(global_rst_addr);
    reg_read_old = reg_read;
    if((reg_read & 0xffffffff) != 0x20002){
        LTRACEF("rstgen rst status test reg value is error:0x%x\n",reg_read);
        rstgen_dump_all_reg_for_test((vaddr_t)base,0,0,0);
        ret = false;
    }
    reg_write = 0xffffffff;
    writel(reg_write,global_rst_addr);
    spin(10);
    reg_read =readl(global_rst_addr);
    if((reg_read & 0xffffffff) != 0xffffffff){
        LTRACEF("rstgen rst status test reg write is error:0x%x\n",reg_read);
        rstgen_dump_all_reg_for_test((vaddr_t)base,0,0,0);
        ret = false;
    }
    writel(reg_read_old,global_rst_addr);
    /*global reg test end*/

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_rw_reg_check_test end................ret:%d\n",ret);
    return ret;
}

//*****************************************************************************
//! rstgen_core_rw_reg_check_test test4
//!
//! \param ulBase is the base address of the rstgen timer module.
//! \core_idx core index
//!
//! This function is test watchdog rw register read it must read data  should be equal with the write data
//!	use 0xffffffff or 0x00000000 write and immediately read it should be equal with the write data
//!
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool rstgen_core_rw_reg_check_test(vaddr_t base,uint32_t core_idx)
{
    bool ret = true;
    vaddr_t core_rst_addr;
    //vaddr_t core_sw_rst_addr;
    uint32_t reg_read = 0;
    uint32_t reg_read_old = 0;
    uint32_t reg_write = 0;
    uint32_t core_slice_idx = 0;

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_core_rw_reg_check_test start................\n");

    core_slice_idx = (core_idx/2) - (CORE_RST_EN/4);
    //core_cr5_se/core_cr5_mp:if fuse PARALLEL_BOOT_DISABLE = 0 then core_rst=0x0,core_sw_rst = 0x40000000,else core_rst=0x40000001,core_sw_rst = 0x60000001
    if(((base == RSTGEN_SEC_BASE) && ((core_slice_idx == drv_core_cpu1_core_all) || (core_slice_idx == drv_core_cpu2_core)))
        || ((base == RSTGEN_SAF_BASE) && (core_slice_idx == drv_core_cr5_mp))){
        /*core reg test begin*/
        core_rst_addr = base+SOC_RSTGEN_REG_MAP(RSTGEN_CORE_RST_EN_OFF(core_slice_idx));
        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_core_rw_reg_check_test core_rst_addr:0x%llx \n",(uint64_t)core_rst_addr);
        reg_read =readl(core_rst_addr);
        reg_read_old = reg_read;
        if(reg_read != 0x40000001){
            LTRACEF("rstgen core rst en test reg value is error:0x%x\n",reg_read);
            rstgen_dump_all_reg_for_test((vaddr_t)base,core_slice_idx,0,0);
            ret = false;
        }
        reg_write = 0x7fffffff;
        writel(reg_write,core_rst_addr);
        spin(10);
        reg_read =readl(core_rst_addr);
        if((reg_read & 0x7) != 0x7){
            LTRACEF("rstgen core rst en test reg write is error:0x%x\n",reg_read);
            rstgen_dump_all_reg_for_test((vaddr_t)base,core_slice_idx,0,0);
            ret = false;
        }
        writel(reg_read_old,core_rst_addr);
    }else{
        /*core reg test begin*/
        core_rst_addr = base+SOC_RSTGEN_REG_MAP(RSTGEN_CORE_RST_EN_OFF(core_slice_idx));
        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_core_rw_reg_check_test core_rst_addr:0x%llx \n",(uint64_t)core_rst_addr);
        reg_read =readl(core_rst_addr);
        reg_read_old = reg_read;
        if(reg_read != 0x00){
            LTRACEF("rstgen core rst en test reg value is error:0x%x\n",reg_read);
            rstgen_dump_all_reg_for_test((vaddr_t)base,core_slice_idx,0,0);
            ret = false;
        }
        reg_write = 0x7fffffff;
        writel(reg_write,core_rst_addr);
        spin(10);
        reg_read =readl(core_rst_addr);
        if((reg_read & 0x7) != 0x7){
            LTRACEF("rstgen core rst en test reg write is error:0x%x\n",reg_read);
            rstgen_dump_all_reg_for_test((vaddr_t)base,core_slice_idx,0,0);
            ret = false;
        }
        writel(reg_read_old,core_rst_addr);

    }
    /*core reg test end*/
    /*#############################################*/
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_core_rw_reg_check_test end................ret:%d\n",ret);

    return ret;
}

//*****************************************************************************
//! rstgen_module_rw_reg_check_test test5
//!
//! \param ulBase is the base address of the rstgen timer module.
//! \module_idx module index
//!
//! This function is test
//!	use 0xffffffff or 0x00000000 write and immediately read it should be equal with the write data
//!
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool rstgen_module_rw_reg_check_test(vaddr_t base,uint32_t module_idx)
{
    bool ret = true;
    vaddr_t module_rst_addr;
    uint32_t reg_read = 0;
    uint32_t reg_read_old = 0;
    uint32_t reg_write = 0;
    uint32_t module_slice_idx = 0;

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_module_rw_reg_check_test start................\n");

    module_slice_idx = module_idx - (MODULE_RST/4);
    /*module reg test begin*/
    module_rst_addr = base+SOC_RSTGEN_REG_MAP(RSTGEN_MODULE_RST_OFF(module_slice_idx));
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_module_rw_reg_check_test module_rst_addr:0x%llx \n",(uint64_t)module_rst_addr);
    reg_read =readl(module_rst_addr);
    reg_read_old = reg_read;
    if(reg_read != 0x40000003){
        LTRACEF("rstgen module reg test reg value is error:0x%x\n",reg_read);
        rstgen_dump_all_reg_for_test((vaddr_t)base,0,module_slice_idx,0);
        ret = false;
    }
    reg_write = 0x7fffffff;
    writel(reg_write,module_rst_addr);
    spin(10);
    reg_read =readl(module_rst_addr);
    if((reg_read & 0x3) != 0x3){
        LTRACEF("rstgen module reg test reg write is error:0x%x\n",reg_read);
        rstgen_dump_all_reg_for_test((vaddr_t)base,0,module_slice_idx,0);
        ret = false;
    }
    writel(reg_read_old,module_rst_addr);
    /*module reg test end*/
    /*#############################################*/
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_module_rw_reg_check_test end................ret:%d\n",ret);
    return ret;
}

//*****************************************************************************
//! rstgen_iso_rw_reg_check_test test6
//!
//! \param ulBase is the base address of the rstgen timer module.
//! \iso_idx isolation index
//!
//! This function is test
//!	use 0xffffffff or 0x00000000 write and immediately read it should be equal with the write data
//!
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool rstgen_iso_rw_reg_check_test(vaddr_t base,uint32_t iso_idx)
{
    bool ret = true;
    vaddr_t iso_rst_addr;
    uint32_t reg_read = 0;
    uint32_t reg_read_old = 0;
    uint32_t reg_write = 0;
    uint32_t iso_slice_idx = 0;

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_iso_rw_reg_check_test start................\n");

    iso_slice_idx = iso_idx - (ISO_EN/4);
    /*module reg test begin*/
    iso_rst_addr = base+SOC_RSTGEN_REG_MAP(ISO_EN+0x04*iso_slice_idx);
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_iso_rw_reg_check_test iso_rst_addr:0x%llx \n",(uint64_t)iso_rst_addr);
    reg_read =readl(iso_rst_addr);
    reg_read_old = reg_read;
    LTRACEF("rstgen iso reg test reg value is:0x%x\n",reg_read);
    if((reg_read & 0x1) == 0x0){
        LTRACEF("rstgen iso reg test reg value is error:0x%x\n",reg_read);
        reg_write = 0x00000001;
        writel(reg_write,iso_rst_addr);
        spin(10);
        reg_read =readl(iso_rst_addr);
        if((reg_read & 0x1) != 0x1){
            LTRACEF("rstgen iso reg test reg write is error:0x%x\n",reg_read);
            rstgen_dump_all_reg_for_test((vaddr_t)base,0,0,iso_slice_idx);
            ret = false;
        }
        writel(reg_read_old,iso_rst_addr);
    }else{
        LTRACEF("rstgen iso reg test reg value is error:0x%x\n",reg_read);
        reg_write = 0x00000000;
        writel(reg_write,iso_rst_addr);
        spin(10);
        reg_read =readl(iso_rst_addr);
        if((reg_read & 0x1) != 0x0){
            LTRACEF("rstgen iso reg test reg write is error:0x%x\n",reg_read);
            rstgen_dump_all_reg_for_test((vaddr_t)base,0,0,iso_slice_idx);
            ret = false;
        }
        writel(reg_read_old,iso_rst_addr);
    }

    /*module reg test end*/
    /*#############################################*/
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_iso_rw_reg_check_test end................ret:%d\n",ret);
    return ret;
}

//*****************************************************************************
//! rstgen_general_rw_reg_check_test test7
//!
//! \param ulBase is the base address of the rstgen timer module.
//! \general_idx general index
//!
//! This function is test
//!	use 0xffffffff or 0x00000000 write and immediately read it should be equal with the write data
//!
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool rstgen_general_rw_reg_check_test(vaddr_t base,uint32_t general_idx)
{
    bool ret = true;
    vaddr_t general_rst_addr;
    uint32_t reg_read = 0;
    uint32_t reg_read_old = 0;
    uint32_t reg_write = 0;
    uint32_t general_slice_idx = 0;

    //case1.4
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_general_rw_reg_check_test start................\n");

    //first power on test
    /*module reg test begin*/
    general_slice_idx = general_idx - (GENERAL_REG/4);
    general_rst_addr = base+SOC_RSTGEN_REG_MAP(GENERAL_REG+0x04*general_slice_idx);
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_general_rw_reg_check_test general_rst_addr:0x%llx \n",(uint64_t)general_rst_addr);
    reg_read =readl(general_rst_addr);
    reg_read_old = reg_read;
    if((reg_read & 0xffffffff) != 0x0){
        LTRACEF("rstgen general reg test reg value is error:0x%x\n",reg_read);
        rstgen_dump_all_reg_for_test((vaddr_t)base,0,0,0);
        ret = false;
    }
    reg_write = 0xffffffff;
    writel(reg_write,general_rst_addr);
    spin(10);
    reg_read =readl(general_rst_addr);
    if((reg_read & 0xffffffff) != 0xffffffff){
        LTRACEF("rstgen general reg test reg write is error:0x%x\n",reg_read);
        rstgen_dump_all_reg_for_test((vaddr_t)base,0,0,0);
        ret = false;
    }
    writel(reg_read_old,general_rst_addr);
    /*module reg test end*/

    /*#############################################*/
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_general_rw_reg_check_test end................ret:%d\n",ret);
    return ret;
}

//*****************************************************************************
//! rstgen_general_rw_reg_check_test test8
//!
//! \param ulBase is the base address of the rstgen timer module.
//! \mask globale reset mask value
//!
//! This function is test need hal poduct
//!	use 0xffffffff or 0x00000000 write and immediately read it should be equal with the write data
//!
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool rstgen_global_reset_test(vaddr_t base,uint32_t mask)
{
    bool ret = true;
    uint32_t rst_sta = 0;

    //case1.5
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test start................\n");

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test base:0x%llx \n",(uint64_t)base);
    rstgen_global_rst_enable( base,mask);
    rst_sta = rstgen_get_rst_sta(base);
    if(mask == 0){
        if((rst_sta & RSTGEN_GLB_RST_SELF_RST_EN_MASK) == 0){
            uint32_t tmp_mask = 0;
            // 1.enable pre rstgen software reset
            // 2.trigger pre rstgen software reset
            //enable safety rstgen software global reset
            tmp_mask = RSTGEN_GLB_RST_SELF_RST_EN_MASK;
            rstgen_global_rst_enable(RSTGEN_SAF_BASE,tmp_mask);
            //trigger safety rstgen software reset
            rstgen_sw_self_rst(RSTGEN_SAF_BASE,false);
            spin(100);
            rstgen_sw_self_rst(RSTGEN_SAF_BASE,true);
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test pre reset test failed\n");
            ret = false;
        }else{
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test pre reset test success\n");
            ret = true;
        }
    }else if(mask & RSTGEN_GLB_RST_SELF_RST_EN_MASK){
        if((rst_sta & RSTGEN_RST_STA_SELF_SW_RST_MASK) == 0){
            rstgen_global_rst_enable(base,(mask & RSTGEN_GLB_RST_SELF_RST_EN_MASK));
            rstgen_sw_self_rst(base,false);
            spin(100);
            rstgen_sw_self_rst(base,true);
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test self reset test failed\n");
            ret = false;
        }else{
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test self reset test success\n");
            ret = true;
        }
    }else if(mask & RSTGEN_GLB_RST_SEM_RST_EN_MASK){
        if((rst_sta & RSTGEN_RST_STA_SEM_RST_MASK) == 0){
            rstgen_global_rst_enable(base,(mask & RSTGEN_GLB_RST_SEM_RST_EN_MASK));
            //trigger sem reset single

            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test sem reset test failed\n");
            ret = false;
        }else{
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test sem reset test success\n");
            ret = true;
        }
    }else if(mask & RSTGEN_GLB_RST_DBG_RST_EN_MASK){
        if((rst_sta & RSTGEN_RST_STA_DBG_RST_MASK) == 0){
            rstgen_global_rst_enable(base,(mask & RSTGEN_GLB_RST_DBG_RST_EN_MASK));
            //trigger debug reset single

            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test debug reset test failed\n");
            ret = false;
        }else{
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test debug reset test success\n");
            ret = true;
        }
    }else if(mask & RSTGEN_GLB_RST_WDG1_RST_EN_MASK){
        if((rst_sta & RSTGEN_RST_STA_WDG1_RST_MASK) == 0){
            rstgen_global_rst_enable(base,(mask & RSTGEN_GLB_RST_WDG1_RST_EN_MASK));
            //trigger watchdog reset single

            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test wdg1 reset test failed\n");
            ret = false;
        }else{

            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test wdg1 reset test success\n");
            ret = true;
        }
    }else if(mask & RSTGEN_GLB_RST_WDG2_RST_EN_MASK){
        if((rst_sta & RSTGEN_RST_STA_WDG2_RST_MASK) == 0){
            rstgen_global_rst_enable(base,(mask & RSTGEN_GLB_RST_WDG2_RST_EN_MASK));
            //trigger watchdog reset single
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test wdg2 reset test failed\n");
            ret = false;
        }else{

            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test wdg2 reset test success\n");
            ret = true;
        }
    }else if(mask & RSTGEN_GLB_RST_WDG3_RST_EN_MASK){
        if((rst_sta & RSTGEN_RST_STA_WDG3_RST_MASK) == 0){
            rstgen_global_rst_enable(base,(mask & RSTGEN_GLB_RST_WDG3_RST_EN_MASK));
            //trigger watchdog reset single
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test wdg3 reset test failed\n");
            ret = false;
        }else{

            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test wdg3 reset test success\n");
            ret = true;
        }
    }else if(mask & RSTGEN_GLB_RST_WDG4_RST_EN_MASK){
        if((rst_sta & RSTGEN_RST_STA_WDG4_RST_MASK) == 0){
            rstgen_global_rst_enable(base,(mask & RSTGEN_GLB_RST_WDG4_RST_EN_MASK));
            //trigger watchdog reset single
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test wdg4 reset test failed\n");
            ret = false;
        }else{

            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test wdg4 reset test success\n");
            ret = true;
        }
    }else if(mask & RSTGEN_GLB_RST_WDG5_RST_EN_MASK){
        if((rst_sta & RSTGEN_RST_STA_WDG5_RST_MASK) == 0){
            rstgen_global_rst_enable(base,(mask & RSTGEN_GLB_RST_WDG5_RST_EN_MASK));
            //trigger watchdog reset single
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test wdg5 reset test failed\n");
            ret = false;
        }else{

            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test wdg5 reset test success\n");
            ret = true;
        }
    }else if(mask & RSTGEN_GLB_RST_WDG6_RST_EN_MASK){
        if((rst_sta & RSTGEN_RST_STA_WDG6_RST_MASK) == 0){
            rstgen_global_rst_enable(base,(mask & RSTGEN_GLB_RST_WDG6_RST_EN_MASK));
            //trigger watchdog reset single
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test wdg6 reset test failed\n");
            ret = false;
        }else{

            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test wdg6 reset test success\n");
            ret = true;
        }
    }else if(mask & RSTGEN_GLB_RST_WDG7_RST_EN_MASK){
        if((rst_sta & RSTGEN_RST_STA_WDG7_RST_MASK) == 0){
            rstgen_global_rst_enable(base,(mask & RSTGEN_GLB_RST_WDG7_RST_EN_MASK));
            //trigger watchdog reset single
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test wdg7 reset test failed\n");
            ret = false;
        }else{

            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test wdg7 reset test success\n");
            ret = true;
        }
    }else if(mask & RSTGEN_GLB_RST_WDG8_RST_EN_MASK){
        if((rst_sta & RSTGEN_RST_STA_WDG8_RST_MASK) == 0){
            rstgen_global_rst_enable(base,(mask & RSTGEN_GLB_RST_WDG8_RST_EN_MASK));
            //trigger watchdog reset single
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test wdg8 reset test failed\n");
            ret = false;
        }else{

            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test wdg8 reset test success\n");
            ret = true;
        }
    }else{

    }
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_global_reset_test end................ret:%d\n",ret);
    return ret;
}

//*****************************************************************************
//case2.5
//! hal_rstgen_module_rst_test. test9
//!
//! \handle rstgen handle for rstgen func.
//! \module_idx module index
//!
//! This function is for rstgen module reset test
//!
//! \return bool status
//
//*****************************************************************************
bool rstgen_module_rst_test(vaddr_t base,uint32_t module_idx)
{
    bool ret = false;
    vaddr_t module_base_addr = 0;
    uint32_t reg_write = 0x0000ffff;
    uint32_t module_slice_idx = 0;
    rstgen_rst_type l_rst_type = drv_rst_type_oth;

    module_slice_idx = module_idx -(MODULE_RST/4);
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_module_rst_test start,module_idx:%d\n",module_slice_idx);

    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_module_rst_test base:0x%llx \n",(uint64_t)base);
    //set module register value
    if(base == RSTGEN_SAF_BASE){
        //set safety module register value
        switch (module_slice_idx) {
            case drv_saf_module_gic1:
            {
                //set register value not equal reset value and read it for check had modify
                module_base_addr = (vaddr_t)GIC1_BASE;
                break;
            }
            case drv_saf_module_ospi1:
            {
                //set register value not equal reset value and read it for check had modify
                module_base_addr = (vaddr_t)APB_OSPI1_BASE;
                break;
            }
            case drv_saf_module_enet1:
            {
                //set register value not equal reset value and read it for check had modify
                module_base_addr = (vaddr_t)APB_BIPC_ENET1_BASE;
                break;
            }
            case drv_saf_module_i2s_sc1:
            {
                //set register value not equal reset value and read it for check had modify
                module_base_addr = (vaddr_t)APB_I2S_SC1_BASE;
                break;
            }
            case drv_saf_module_i2s_sc2:
            {
                //set register value not equal reset value and read it for check had modify
                module_base_addr = (vaddr_t)APB_I2S_SC2_BASE;
                break;
            }
            case drv_saf_module_canfd1:
            {
                //set register value not equal reset value and read it for check had modify
                module_base_addr = (vaddr_t)APB_CAN1_BASE;
                break;
            }
            case drv_saf_module_canfd2:
            {
                //set register value not equal reset value and read it for check had modify
                module_base_addr = (vaddr_t)APB_CAN2_BASE;
                break;
            }
            case drv_saf_module_canfd3:
            {
                //set register value not equal reset value and read it for check had modify
                module_base_addr = (vaddr_t)APB_CAN3_BASE;
                break;
            }
            case drv_saf_module_canfd4:
            {
                //set register value not equal reset value and read it for check had modify
                module_base_addr = (vaddr_t)APB_CAN4_BASE;
                break;
            }
            case drv_saf_module_sem1:
            {
                module_base_addr = (vaddr_t)APB_SEM1_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_saf_module_sem2:
            {
                module_base_addr = (vaddr_t)APB_SEM2_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            default:
            {
                ret = false;
                break;
            }
        }
    }else if(base == RSTGEN_SEC_BASE){
        //set sec module register value
        switch (module_slice_idx) {
            case drv_ap_module_ospi2:
            {
                module_base_addr = (vaddr_t)APB_OSPI2_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_i2s_sc3:
            {
                module_base_addr = (vaddr_t)APB_I2S_SC3_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_i2s_sc4:
            {
                module_base_addr = (vaddr_t)APB_I2S_SC4_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_i2s_sc5:
            {
                module_base_addr = (vaddr_t)APB_I2S_SC5_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_i2s_sc6:
            {
                module_base_addr = (vaddr_t)APB_I2S_SC6_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_i2s_sc7:
            {
                module_base_addr = (vaddr_t)APB_I2S_SC7_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_i2s_sc8:
            {
                module_base_addr = (vaddr_t)APB_I2S_SC8_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_i2s_mc1:
            {
                module_base_addr = (vaddr_t)APB_I2S_MC1_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_i2s_mc2:
            {
                module_base_addr = (vaddr_t)APB_I2S_MC2_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_canfd5:
            {
                module_base_addr = (vaddr_t)APB_CAN5_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_canfd6:
            {
                module_base_addr = (vaddr_t)APB_CAN6_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_canfd7:
            {
                module_base_addr = (vaddr_t)APB_CAN7_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_canfd8:
            {
                module_base_addr = (vaddr_t)APB_CAN8_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_enet2:
            {
                module_base_addr = (vaddr_t)APB_ENET_QOS2_BASE;//APB_BIPC_ENET2_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_mshc1:
            {
                module_base_addr = (vaddr_t)SD1_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_mshc2:
            {
                module_base_addr = (vaddr_t)SD2_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_mshc3:
            {
                module_base_addr = (vaddr_t)SD3_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_mshc4:
            {
                module_base_addr = (vaddr_t)SD4_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_adsp:
            {
                //module_base_addr = (vaddr_t)ADSP_BASE;//address error
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_gic2:
            {
                module_base_addr = (vaddr_t)GIC2_BASE;
                l_rst_type = drv_rst_type_gic;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_gic3:
            {
                module_base_addr = (vaddr_t)GIC3_BASE;
                l_rst_type = drv_rst_type_gic;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_cpu1_core0_warm:
            {
                l_rst_type = drv_rst_type_cpu;
                //module_base_addr = (vaddr_t)drv_ap_module_cpu1_core0_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_cpu1_core1_warm:
            {
                l_rst_type = drv_rst_type_cpu;
                //module_base_addr = (vaddr_t)drv_ap_module_cpu1_core1_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_cpu1_core2_warm:
            {
                l_rst_type = drv_rst_type_cpu;
                //module_base_addr = (vaddr_t)drv_ap_module_cpu1_core2_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_cpu1_core3_warm:
            {
                l_rst_type = drv_rst_type_cpu;
                //module_base_addr = (vaddr_t)drv_ap_module_cpu1_core3_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_cpu1_core4_warm:
            {
                l_rst_type = drv_rst_type_cpu;
                //module_base_addr = (vaddr_t)drv_ap_module_cpu1_core4_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_cpu1_core5_warm:
            {
                l_rst_type = drv_rst_type_cpu;
                //module_base_addr = (vaddr_t)drv_ap_module_cpu1_core5_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_cpu1_scu_warm:
            {
                l_rst_type = drv_rst_type_cpu;
                //module_base_addr = (vaddr_t)drv_ap_module_cpu1_core0_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_ddr_ss:
            {
                //module_base_addr = (vaddr_t)APB_DDRCTRL_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_ddr_sw0:
            {
                //module_base_addr = (vaddr_t)APB_DDRCTRL_BASE+0X0080; address error
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_ddr_sw1:
            {
                //module_base_addr = (vaddr_t)APB_DDRCTRL_BASE+0X00b0;  address error
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_ddr_sw2:
            {
                //module_base_addr = (vaddr_t)drv_ap_module_ddr_sw2;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_gic4:
            {
                module_base_addr = (vaddr_t)GIC4_BASE;
                l_rst_type = drv_rst_type_gic;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_gic5:
            {
                module_base_addr = (vaddr_t)GIC5_BASE;
                l_rst_type = drv_rst_type_gic;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_cssys_treset_n:
            {
                //module_base_addr = (vaddr_t)0x30bf6000;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_nna:
            {
                //module_base_addr = (vaddr_t)drv_ap_module_nna;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_vdsp_DReset:
            {
                //module_base_addr = (vaddr_t)VDSP_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_vpu1:
            {
                module_base_addr = (vaddr_t)APB_VPU1_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_vpu2:
            {
                module_base_addr = (vaddr_t)APB_VPU2_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_mjpeg:
            {
                module_base_addr = (vaddr_t)APB_MJPEG_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_cpu1_ss:
            {
                module_base_addr = (vaddr_t)APB_PLL_CPU1A_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_cpu2_ss:
            {
                module_base_addr = (vaddr_t)APB_PLL_CPU2_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_mipi_csi1:
            {
                module_base_addr = (vaddr_t)APB_MIPI_CSI1_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_mipi_csi2:
            {
                module_base_addr = (vaddr_t)APB_MIPI_CSI2_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_mipi_csi3:
            {
                //module_base_addr = (vaddr_t)APB_MIPI_CSI3_BASE; //no module
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_mipi_dsi1:
            {
                module_base_addr = (vaddr_t)APB_MIPI_DSI1_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_mipi_dsi2:
            {
                module_base_addr = (vaddr_t)APB_MIPI_DSI2_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_dc1:
            {
                module_base_addr = (vaddr_t)APB_DC1_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_dc2:
            {
                module_base_addr = (vaddr_t)APB_DC2_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_dc3:
            {
                module_base_addr = (vaddr_t)APB_DC3_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_dc4:
            {
                module_base_addr = (vaddr_t)APB_DC4_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_dc5:
            {
                module_base_addr = (vaddr_t)APB_DC5_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_dp1:
            {
                module_base_addr = (vaddr_t)APB_DP1_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_dp2:
            {
                module_base_addr = (vaddr_t)APB_DP2_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_dp3:
            {
                module_base_addr = (vaddr_t)APB_DP3_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_lvds_ss:
            {
                module_base_addr = (vaddr_t)APB_LVDS1_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_csi1:
            {
                module_base_addr = (vaddr_t)APB_CSI1_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_csi2:
            {
                module_base_addr = (vaddr_t)APB_CSI2_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_csi3:
            {
                module_base_addr = (vaddr_t)APB_CSI3_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_disp_mux:
            {
                module_base_addr = (vaddr_t)APB_DISP_MUX_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_g2d1:
            {
                module_base_addr = (vaddr_t)APB_G2D1_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_g2d2:
            {
                module_base_addr = (vaddr_t)APB_G2D2_BASE;
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_gpu1_core:
            {
                //module_base_addr = (vaddr_t)GPU1_BASE;//address error
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_gpu1_ss:
            {
                //module_base_addr = (vaddr_t)GPU1_BASE;//address error
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_gpu2_core:
            {
                //module_base_addr = (vaddr_t)GPU2_BASE;//address error
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_gpu2_ss:
            {
                //module_base_addr = (vaddr_t)GPU2_BASE;//address error
                //set register value not equal reset value and read it for check had modify
                break;
            }
            case drv_ap_module_dbg_req:
            {
                //set register value not equal reset value and read it for check had modify
                break;
            }
            default:
            {
                break;
            }
        }
    }else{
        ret = false;
    }

    if(module_base_addr != 0){
        //just module register value is rset value
        vaddr_t module_base_vaddr = 0;
        uint32_t tmp_reg_write = 0;
        uint32_t tmp_reg_read = 0;

        module_base_vaddr=(vaddr_t)_ioaddr(module_base_addr);
        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "## module_base_addr:0x%x module_base_vaddr:0x%llx\n",(uint32_t)module_base_addr,(uint64_t)module_base_vaddr);

        if(l_rst_type == drv_rst_type_gic){
            tmp_reg_write = 2;
            rstgen_gic_set_priority_locked(module_base_vaddr,1,tmp_reg_write);
            tmp_reg_write = rstgen_gic_get_priority(module_base_vaddr,1);
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "MODULE_READ_DATA gic write:%d\n",tmp_reg_write);
        }else if(l_rst_type == drv_rst_type_cpu){
            //Please active cpu1 mp core then read core is active
#if WITH_SMP
            uint8_t cpu_num = 0;
            cpu_num = module_slice_idx - drv_ap_module_cpu1_core0_warm;
            if(cpu_num > SMP_MAX_CPUS){
                return false;
            }

            if(rstgen_mp_is_cpu_active(cpu_num)){
                tmp_reg_write = 10;
                tmp_reg_read = tmp_reg_write;
            }else{
                tmp_reg_write = 10;
                tmp_reg_read = 0;
            }
#else
            tmp_reg_write = 10;
            tmp_reg_read = 0;
#endif
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "MODULE_READ_DATA cpu test\n");
        }else{
            tmp_reg_write = readl(module_base_vaddr);
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "MODULE_WRITE_DATA tmp_reg_write111:0x%x\n",tmp_reg_write);
            writel(reg_write,module_base_vaddr);
            tmp_reg_write = readl(module_base_vaddr);
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "MODULE_WRITE_DATA tmp_reg_write222:0x%x\n",tmp_reg_write);
        }

        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "MODULE_WRITE_DATA g_module_reg_read:0x%x\n",tmp_reg_write);

        rstgen_module_ctl(base,module_slice_idx,false);
        spin(1);
        rstgen_module_ctl(base,module_slice_idx,true);
        spin(1);

        if(l_rst_type == drv_rst_type_gic){
            tmp_reg_read = rstgen_gic_get_priority(module_base_vaddr,1);
        }else if(l_rst_type == drv_rst_type_cpu){
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "MODULE_READ_DATA cpu test\n");
        }else{
            tmp_reg_read = readl(module_base_vaddr);
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "MODULE_WRITE_DATA tmp_reg_write333:0x%x\n",tmp_reg_write);
        }

        LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "MODULE_READ_DATA tmp_reg_read:0x%x,tmp_reg_write:0x%x\n",tmp_reg_read,tmp_reg_write);

        if(tmp_reg_read != tmp_reg_write){
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_module_rst_test reset test success\n");
            ret = true;
        }else{
            LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_module_rst_test reset test failed tmp_reg_read:0x%x,tmp_reg_write:0x%x\n",tmp_reg_read,tmp_reg_write);
            ret = false;
        }
    }else{
        ret = true;
    }
    LTRACEF_LEVEL(DEFAULT_RSTGEN_LOG_LEVEL, "rstgen_module_rst_test end ret:%d\n",ret);
    return ret;
}

