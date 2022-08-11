#include <assert.h>
#include <stdlib.h>
#include <kernel/thread.h>
#include "diskio.h"
#include "ff.h"

#include "storage_cfg.h"

#define SECTOR_SIZE 512

#define SDRV_DISK_TYPE_MMC1 0
#define SDRV_DISK_TYPE_MMC2 1
#define SDRV_DISK_TYPE_MMC3 2
#define SDRV_DISK_TYPE_MMC4 3
#define SDRV_DISK_TYPE_OSPI1 4
#define SDRV_DISK_TYPE_OSPI2 5
#define SDRV_DISK_TYPE_MEM1 6
#define SDRV_DISK_TYPE_MEM2 7
#define SDRV_DISK_NUM_MAX 8

static struct storage_device_cfg* Disk[SDRV_DISK_NUM_MAX] = {
    &mmc1,
    &mmc2,
    &mmc3,
    &mmc4,
    &ospi1,
    &ospi2,
    &memdisk1, /* for ospi1 direct access */
    &memdisk2, /* for ram access */
};

/*--------------------------------------------------------------------------

	Public Functions

---------------------------------------------------------------------------*/

/*
 * Global variables
 */
static DSTATUS Stat[SDRV_DISK_NUM_MAX] = {
    STA_NOINIT,
    STA_NOINIT,
    STA_NOINIT,
    STA_NOINIT,
    STA_NOINIT,
    STA_NOINIT,
    STA_NOINIT,
    STA_NOINIT,
};	/* Disk status */

static storage_device_t *Storage[SDRV_DISK_NUM_MAX] = {0};

/*-----------------------------------------------------------------------*/
/* Get Disk Status							*/
/*-----------------------------------------------------------------------*/

/*****************************************************************************/
/**
*
* Gets the status of the disk.
* In case of SD, it checks whether card is present or not.
*
* @param	pdrv - Drive number
*
* @return
*		0		Status ok
*		STA_NOINIT	Drive not initialized
*		STA_NODISK	No medium in the drive
*		STA_PROTECT	Write protected
*
* @note		In case Card detect signal is not connected,
*		this function will not be able to check if card is present.
*
******************************************************************************/
DSTATUS disk_status (
		BYTE pdrv	/* Drive number (0) */
)
{
	DSTATUS s = Stat[pdrv];
	return s;
}

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive						 */
/*-----------------------------------------------------------------------*/
/*****************************************************************************/
/**
*
* Initializes the drive.
* In case of SD, it initializes the host controller and the card.
* This function also selects additional settings such as bus width,
* speed and block size.
*
* @param	pdrv - Drive number
*
* @return	s - which contains an OR of the following information
*		STA_NODISK	Disk is not present
*		STA_NOINIT	Drive not initialized
*		STA_PROTECT	Drive is write protected
*		0 or only STA_PROTECT both indicate successful initialization.
*
* @note
*
******************************************************************************/
DSTATUS disk_initialize (
		BYTE pdrv	/* Physical drive number (0) */
)
{
	DSTATUS s;

	s = disk_status(pdrv);
	if ((s & STA_NODISK) != 0U) {
		return s;
	}

	/* If disk is already initialized */
	if ((s & STA_NOINIT) == 0U) {
		return s;
	}

	if (Disk[pdrv]) {
		Storage[pdrv] = setup_storage_dev(Disk[pdrv]->device_type,
						Disk[pdrv]->res_idex, (void *)&Disk[pdrv]->cfg);
		ASSERT(Storage[pdrv]);
		s &= (~STA_NOINIT);
		Stat[pdrv] = s;
	}
	else {
		s |= STA_NODISK;
		Stat[pdrv] = s;
	}

	return s;
}

/*-----------------------------------------------------------------------*/
/* Set global offset							 */
/*-----------------------------------------------------------------------*/
/*****************************************************************************/
/**
*
* Set global offset of sector number
*
* @param	pdrv - Drive number
* @param	offset - global offset sector number

* @return
*		RES_OK		Successful
*		RES_ERROR	Successful
*
* @note
*
******************************************************************************/
DRESULT disk_set_offset (
		BYTE pdrv,	/* Physical drive number (0) */
		LBA_t offset	/* Start sector number (LBA) */
)
{
	Disk[pdrv]->offset = offset;
	return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)							 */
/*-----------------------------------------------------------------------*/
/*****************************************************************************/
/**
*
* Reads the drive
* In case of SD, it reads the SD card using ADMA2 in polled mode.
*
* @param	pdrv - Drive number
* @param	*buff - Pointer to the data buffer to store read data
* @param	sector - Start sector number
* @param	count - Sector count
*
* @return
*		RES_OK		Read successful
*		STA_NOINIT	Drive not initialized
*		RES_ERROR	Read not successful
*
* @note
*
******************************************************************************/
DRESULT disk_read (
		BYTE pdrv,	/* Physical drive number (0) */
		BYTE *buff,	/* Pointer to the data buffer to store read data */
		LBA_t sector,	/* Start sector number (LBA) */
		UINT count	/* Sector count (1..128) */
)
{
	DSTATUS s;
	storage_device_t *storage = Storage[pdrv];
	LBA_t src_addr;
	ASSERT(storage);

	s = disk_status(pdrv);

	if ((s & STA_NOINIT) != 0U) {
		return RES_NOTRDY;
	}
	if (count == 0U) {
		return RES_PARERR;
	}

	src_addr = (sector + Disk[pdrv]->offset) * SECTOR_SIZE;
	if (storage->read(storage, src_addr, buff, count * SECTOR_SIZE))
		return RES_ERROR;

	return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions						*/
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,				/* Physical drive number (0) */
	BYTE cmd,				/* Control code */
	void *buff				/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_OK;
	storage_device_t *storage = Storage[pdrv];
	ASSERT(storage);

	void *LocBuff = buff;
	if ((disk_status(pdrv) & STA_NOINIT) != 0U) {	/* Check if card is in the socket */
		return RES_NOTRDY;
	}

	res = RES_ERROR;
	switch (cmd) {
		case (BYTE)CTRL_SYNC :	/* Make sure that no pending write process */
			res = RES_OK;
			break;

		case (BYTE)GET_SECTOR_COUNT : /* Get number of sectors on the disk (DWORD) */
			(*((DWORD *)(void *)LocBuff)) = storage->get_capacity(storage) / SECTOR_SIZE;
			res = RES_OK;
			break;

		case (BYTE)GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
			(*((DWORD *)((void *)LocBuff))) = ((DWORD)128);
			res = RES_OK;
			break;

		default:
			res = RES_PARERR;
			break;
	}

	return res;
}

/******************************************************************************/
/**
*
* This function is User Provided Timer Function for FatFs module
*
* @return	DWORD
*
* @note		None
*
****************************************************************************/

DWORD get_fattime (void)
{
	return	((DWORD)(2010U - 1980U) << 25U)	/* Fixed to Jan. 1, 2010 */
		| ((DWORD)1 << 21)
		| ((DWORD)1 << 16)
		| ((DWORD)0 << 11)
		| ((DWORD)0 << 5)
		| ((DWORD)0 >> 1);
}

/*****************************************************************************/
/**
*
* Reads the drive
* In case of SD, it reads the SD card using ADMA2 in polled mode.
*
* @param	pdrv - Drive number
* @param	*buff - Pointer to the data to be written
* @param	sector - Sector address
* @param	count - Sector count
*
* @return
*		RES_OK		Read successful
*		STA_NOINIT	Drive not initialized
*		RES_ERROR	Read not successful
*
* @note
*
******************************************************************************/
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Sector address (LBA) */
	UINT count			/* Number of sectors to write (1..128) */
)
{
	DSTATUS s;
	storage_device_t *storage = Storage[pdrv];
	LBA_t dst_addr;
	ASSERT(storage);

	s = disk_status(pdrv);

	if ((s & STA_NOINIT) != 0U) {
		return RES_NOTRDY;
	}
	if (count == 0U) {
		return RES_PARERR;
	}

	dst_addr = (sector + Disk[pdrv]->offset) * SECTOR_SIZE;
	if (storage->cached_write) {
		if (storage->cached_write(storage, dst_addr, buff, count * SECTOR_SIZE))
			return RES_ERROR;
	} else {
		if (storage->write(storage, dst_addr, buff, count * SECTOR_SIZE))
			return RES_ERROR;
	}

	return RES_OK;
}
