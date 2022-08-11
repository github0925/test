/*************************************************************************/ /*!
@File           img_elf.h
@Title          IMG ELF file definitions
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Platform       RGX
@Description    Definitions for ELF file structures used in the DDK.
@License        Strictly Confidential.
*/ /**************************************************************************/

#if !defined(IMG_ELF_H)
#define IMG_ELF_H

#include "img_types.h"

/* ELF format defines */
#define ELF_PT_LOAD     (0x1U)   /* Program header identifier as Load */
#define ELF_SHT_SYMTAB  (0x2U)   /* Section identifier as Symbol Table */
#define ELF_SHT_STRTAB  (0x3U)   /* Section identifier as String Table */
#define MAX_STRTAB_NUM  (0x8U)   /* Maximum number of string table in the ELF file */

/* Redefined structs of ELF format */
typedef struct
{
	IMG_UINT8    ui32Eident[16];
	IMG_UINT16   ui32Etype;
	IMG_UINT16   ui32Emachine;
	IMG_UINT32   ui32Eversion;
	IMG_UINT32   ui32Eentry;
	IMG_UINT32   ui32Ephoff;
	IMG_UINT32   ui32Eshoff;
	IMG_UINT32   ui32Eflags;
	IMG_UINT16   ui32Eehsize;
	IMG_UINT16   ui32Ephentsize;
	IMG_UINT16   ui32Ephnum;
	IMG_UINT16   ui32Eshentsize;
	IMG_UINT16   ui32Eshnum;
	IMG_UINT16   ui32Eshtrndx;
} IMG_ELF_HDR;

typedef struct
{
	IMG_UINT32   ui32Stname;
	IMG_UINT32   ui32Stvalue;
	IMG_UINT32   ui32Stsize;
	IMG_UINT8    ui32Stinfo;
	IMG_UINT8    ui32Stother;
	IMG_UINT16   ui32Stshndx;
} IMG_ELF_SYM;

typedef struct
{
	IMG_UINT32   ui32Shname;
	IMG_UINT32   ui32Shtype;
	IMG_UINT32   ui32Shflags;
	IMG_UINT32   ui32Shaddr;
	IMG_UINT32   ui32Shoffset;
	IMG_UINT32   ui32Shsize;
	IMG_UINT32   ui32Shlink;
	IMG_UINT32   ui32Shinfo;
	IMG_UINT32   ui32Shaddralign;
	IMG_UINT32   ui32Shentsize;
} IMG_ELF_SHDR;

typedef struct
{
	IMG_UINT32   ui32Ptype;
	IMG_UINT32   ui32Poffset;
	IMG_UINT32   ui32Pvaddr;
	IMG_UINT32   ui32Ppaddr;
	IMG_UINT32   ui32Pfilesz;
	IMG_UINT32   ui32Pmemsz;
	IMG_UINT32   ui32Pflags;
	IMG_UINT32   ui32Palign;
} IMG_ELF_PROGRAM_HDR;

#endif /* IMG_ELF_H */
