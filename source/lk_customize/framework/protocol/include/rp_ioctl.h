/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef __SEMIDRIVE_RP_IOCTL_H__
#define __SEMIDRIVE_RP_IOCTL_H__

/* ioctl command encoding: 32 bits total, command in lower 16 bits,
 * size of the parameter structure in the lower 14 bits of the
 * upper 16 bits.
 * Encoding the size of the parameter structure in the ioctl request
 * is useful for catching programs compiled with old versions
 * and to avoid overwriting user space outside the user buffer area.
 * The highest 2 bits are reserved for indicating the ``access mode''.
 * NOTE: This limits the max parameter size to 16kB -1 !
 */

/*
 * The following is for compatibility across the various Linux
 * platforms.  The generic ioctl numbering scheme doesn't really enforce
 * a type field.  De facto, however, the top 8 bits of the lower 16
 * bits are indeed used as a type field, so we might just as well make
 * this explicit here.  Please be sure to use the decoding macros
 * below from now on.
 */
#define RP_IOC_NRBITS	8
#define RP_IOC_TYPEBITS	8

/*
 * Let any architecture override either of the following before
 * including this file.
 */

#ifndef RP_IOC_SIZEBITS
# define RP_IOC_SIZEBITS	14
#endif

#ifndef RP_IOC_DIRBITS
# define RP_IOC_DIRBITS	2
#endif

#define RP_IOC_NRMASK	((1 << RP_IOC_NRBITS)-1)
#define RP_IOC_TYPEMASK	((1 << RP_IOC_TYPEBITS)-1)
#define RP_IOC_SIZEMASK	((1 << RP_IOC_SIZEBITS)-1)
#define RP_IOC_DIRMASK	((1 << RP_IOC_DIRBITS)-1)

#define RP_IOC_NRSHIFT	0
#define RP_IOC_TYPESHIFT	(RP_IOC_NRSHIFT+RP_IOC_NRBITS)
#define RP_IOC_SIZESHIFT	(RP_IOC_TYPESHIFT+RP_IOC_TYPEBITS)
#define RP_IOC_DIRSHIFT	(RP_IOC_SIZESHIFT+RP_IOC_SIZEBITS)

/*
 * Direction bits, which any architecture can choose to override
 * before including this file.
 *
 * NOTE: RP_IOC_WRITE means Linux is writing and RTOS is
 * reading. RP_IOC_READ means Linux is reading and RTOS is writing.
 */

#ifndef RP_IOC_NONE
# define RP_IOC_NONE	0U
#endif

#ifndef RP_IOC_WRITE
# define RP_IOC_WRITE	1U
#endif

#ifndef RP_IOC_READ
# define RP_IOC_READ	2U
#endif

#define RP_IOC(dir,type,nr,size) \
	(((dir)  << RP_IOC_DIRSHIFT) | \
	 ((type) << RP_IOC_TYPESHIFT) | \
	 ((nr)   << RP_IOC_NRSHIFT) | \
	 ((size) << RP_IOC_SIZESHIFT))

/*
 * Used to create numbers.
 *
 * NOTE: RP_IOW means Linux is writing and RTOS is reading. RP_IOR
 * means Linux is reading and RTOS is writing.
 */
#define RP_IO(type,nr)		RP_IOC(RP_IOC_NONE,(type),(nr),0)
#define RP_IOR(type,nr,size)	RP_IOC(RP_IOC_READ,(type),(nr),(sizeof(size)))
#define RP_IOW(type,nr,size)	RP_IOC(RP_IOC_WRITE,(type),(nr),(sizeof(size)))
#define RP_IOWR(type,nr,size)	RP_IOC(RP_IOC_READ|RP_IOC_WRITE,(type),(nr),(sizeof(size)))

/* used to decode ioctl numbers.. */
#define RP_IOC_DIR(nr)		(((nr) >> RP_IOC_DIRSHIFT) & RP_IOC_DIRMASK)
#define RP_IOC_TYPE(nr)		(((nr) >> RP_IOC_TYPESHIFT) & RP_IOC_TYPEMASK)
#define RP_IOC_NR(nr)		(((nr) >> RP_IOC_NRSHIFT) & RP_IOC_NRMASK)
#define RP_IOC_SIZE(nr)		(((nr) >> RP_IOC_SIZESHIFT) & RP_IOC_SIZEMASK)

#endif /* __SEMIDRIVE_RP_IOCTL_H__ */
