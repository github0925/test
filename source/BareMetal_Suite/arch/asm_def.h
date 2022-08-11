/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

#ifndef __ASM_INC_H__
#define __ASM_INC_H__

#define FUNCTION(x) .global x; .type x,"function"; x:
#define DATA(x) .global x; .type x,"data"; x:

#define LOCAL_FUNCTION(x) .type x,"function"; x:
#define LOCAL_DATA(x) .type x,"data"; x:

#endif

