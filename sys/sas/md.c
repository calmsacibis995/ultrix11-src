/*
 * SCCSID: @(#)md.c	3.0	4/21/86
 */
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 *
 *  File name:
 *
 *	md.c
 *
 *  Source file description:
 *
 *	This module is virtual disk driver to be used with the boot and
 *	other standalone programs. It allows memory to be accessed as a
 *	disk. The main usage for the memory disk is to speed up installation
 *	on systems with 512KB or more memory. The boot "media" is copied
 *	into memory so sdload can access the standalone programs via
 *	this memory disk driver. The base address for memory accesses is
 *	256Kb (after boot/sdload).
 *
 *  Functions:
 *
 *	mdstrategy	handles all read/write memory accesses.
 *
 *  Usage:
 *
 *	md(unit, off)
 *
 *		unit	unit number, ignored but should be zero.
 *		off	offset from 256Kb memory base (512 byte blocks).
 *
 *  Compile:
 *
 *	make libsa.a (cc -c -O md.c)
 *
 *  Modification history:
 *
 *	01 December 1984
 *		File Created -- Fred Canter
 *
 *	09 May 1985
 *		Version 2.1 -- Fred Canter/Bill burns
 *
 *		Bill - chnages for SYSTEM 5 C compiler.
 *		Fred - move memory disk base address from 192KB to 256KB
 *		       for new boot (boot now at 192KB instead of 128KB).
 *
 */

#include <sys/param.h>
#include <sys/inode.h>
#include "saio.h"

#define	PS	((physadr)0177776)

				/* PSW bit definitions */
#define	CMUSER	0140000		/* current mode user */
#define	CMKERN	0		/* current mode kernel */
#define	PMUSER	030000		/* previous mode user */
#define	PMKERN	0		/* previous mode kernel */
#define	SPL7	0340		/* priority level 7 */

#define	KISA0	0172340		/* Memory management PAR/PDR addresses */
#define	KISD0	0172300
#define	UISA0	0177640
#define	UISD0	0177600
#define	KB256	010000		/* PAR value for 256 KB */
#define	PDRVAL0	03406		/* 512 byte page length + read/write access */
#define	PDRVAL1	04006		/* 1024 byte page length + read/write access */

/*
 * segflag -- location of user's buffer i.e., kernel or user space.
 *
 *  3 -	called from user space, shag a kernal par/pdr for mapping
 *	to memory disk area. User's buffer already mapped in user space.
 *
 *  0 - called from kernel space, shag a user par/pdr for mapping
 *	to memory disk area. Shag a second user par/pdr for mapping
 *	to user's buffer in kernel space.
 */
extern	int	segflag;

/*
 * Function:
 *
 *	mdstrategy
 *
 * Function Description:
 *
 *	Performs read/write functions from/to physical memory. Memory
 *	is accessed as if it were a disk, i.e., in units of 512 byte
 *	logical blocks. The layout of memory is as follows:
 *
 *		+---------------+ 0
 *		| stand-alone	|
 *		| program	|
 *		+---------------+ 64KB
 *		| syscall	|
 *		| segment	|
 *		+---------------+ 192KB
 *		| boot / sdload |
 *		| program	|
 *		+---------------+ 256KB
 *		| memory disk	|
 *		:		:
 *		:		:
 *		+---------------+ end of memory (512KB = 512 blocks)
 *
 *	The driver uses memory management PAR/PDR registers to map to
 *	the memory disk and, if the driver is running in kernel mode,
 *	to the user's buffer. The user's buffer is directly mapped if
 *	the driver is running in user mode. The driver checks the value
 *	in the PAR before each memory access and returns an error if the
 *	access would be beyond the end of physical memory. The amount of
 *	available memory (maxmem) in 64 byte clicks is saved in the
 *	devsw[].dv_csr entry for "md" (see conf.c) by the boot program.
 *	The driver runs in kernel mode when a stand-alone program (mkfs,
 *	icheck, rabads, etc.) is running, and in user mode when boot or
 *	sdload is running.
 *
 * Arguments:
 *
 *	struct iob *io	- pointer to I/O block, see saio.h
 *	int	func	- specifies read or write operation, see saio.h
 *
 * Return Values:
 *
 *	The function returns -1 if the read/write failed.
 *	Otherwise it returns the number of bytes transfered.
 *
 * Side Effects:
 *
 *	None
 */

mdstrategy(io, func)
register struct iob *io;
{
	physadr par, pdr;
	int	a0, d0, a1, d1, sps;
	int	wc, wd;
	char	*adr0, *adr1;
	int	*ubp;
	int	errflag;


	if(segflag == 3) {	/* set pointers to segmentation registers */
		par = KISA0;	/* in user mode, shag kernel par/pdr */
		pdr = KISD0;
	} else {
		par = UISA0;	/* in kernel mode, shag user par/pdr */
		pdr = UISD0;
	}
	a0 = par->r[4];		/* save PAR & PDR contents */
	d0 = pdr->r[4];
	if(segflag != 3) {
		a1 = par->r[5];
		d1 = pdr->r[5];
	}
	sps = PS->r[0];		/* save processor status word */
	if(segflag == 3)	/* set current and previous modes in PSW */
		PS->r[0] = (CMUSER|PMKERN|SPL7);
	else
		PS->r[0] = (CMKERN|PMUSER|SPL7);
	par->r[4] = KB256 + (io->i_bn * 8);
	pdr->r[4] = PDRVAL0;
	if(segflag != 3) {
		par->r[5] = (((int)io->i_ma >> 6) & 01777);
		pdr->r[5] = PDRVAL1;
	}
	wc = 0;
	adr0 = 0100000;				/* address for memory disk */
	adr1 = 0120000 + ((int)io->i_ma & 077);	/* user's buffer address */
	if(segflag == 3)	/* don't use adr1, user's buffer is mapped in */
		ubp = io->i_ma;	/* pointer to user's buffer */
	errflag = 0;
	while(wc < (io->i_cc >> 1)) {
		/*
		 * Make sure xfer does not overflow memory.
		 * devsw[].dv_csr = maxmem (memory size in 64 byte clicks)
		 */
		if(par->r[4] >= (unsigned)devsw[io->i_ino.i_dev].dv_csr) {
			printf("\nFATAL ERROR: memory disk - out of memory!\n");
			errflag = 1;
			break;
		}
		if(func == READ) {
			wd = mfpi(adr0);
			if(segflag == 3)
				*ubp = wd;
			else
				mtpi(wd, adr1);
		} else {
			if(segflag == 3)
				wd = *ubp;
			else
				wd = mfpi(adr1);
			mtpi(wd, adr0);
		}
		adr0 += 2;
		adr1 += 2;
		wc++;
		ubp++;
		if((wc % 256) == 0) {
			par->r[4] += 8;
			if(segflag != 3)
				par->r[5] += 8;
			adr0 = 0100000;
			adr1 = 0120000 + ((int)io->i_ma & 077);
		}
	}
	par->r[4] = a0;		/* restore PAR, PDR, PSW */
	pdr->r[4] = d0;
	if(segflag != 3) {
		par->r[5] = a1;
		pdr->r[5] = d1;
	}
	PS->r[0] = sps;
	if(errflag)
		return(-1);
	else
		return(io->i_cc);
}
