/*
 * dw_i2c_test.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: I2C driver header for test.
 *
 * Revision History:
 * -----------------
 */
#ifndef __DW_I2C_TEST_H__
#define __DW_I2C_TEST_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

#include <dev/i2c.h>
#include "dw_i2c.h"


bool dw_i2c_dump_all_reg_test(i2c_reg_type_t *base);
bool dw_i2c_read_only_reg_test(vaddr_t base);
bool dw_i2c_rw_reg_test(vaddr_t base);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif  //__DW_I2C__TEST_H__
