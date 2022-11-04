
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985.	      *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/include/COPYRIGHT" for applicable restrictions.  *
 **********************************************************************/

/*
 * SCCSID: @(#)dksizes.c	3.1	3/26/87
 */

/*
 * Following prevents including unneeded code
 * in ra_info.h
 */
#ifdef	KERNEL
#undef	KERNEL
#endif	KERNEL

/*
 * If USEP defined, this file included in one of
 * the USEP disk exercisers, such as, rax1.c, hpx1.c, etc.
 */
#ifndef	USEP
#include "dds.h"
#include <sys/types.h>
#if	NUDA > 0
#include <sys/ra_info.h>
#endif
#endif	USEP



/*
 * HK - RK06/7 Disk Partition Layout
 *
 * GENERAL RULES:
 *
 *  1.	If at all possible, you should use the standard disk partition
 *	layout. If you choose to change the disk partition layout, you
 *	own the responsibility for any file system damage and/or loss of
 *	data caused by an improper disk partition layout.
 *
 *	MAKE A BACKUP COPY OF THIS FILE BEFORE IMPLEMENTING ANY CHANGES
 *
 *  2.	Do not change the HK partition layout if the RK06/7 is the system
 *	disk, that is, the disk where the root, swap, error log, and /usr
 *	file systems are located.
 *
 *  3.	The partition layout is defined, by the sizes table shown below, on
 *	a per controller basis not per drive. The sizes must be the same for
 *	each drive on the controller. However, not all partitions are used
 *	on all drives.
 *
 *  4.	The disk partition layout is not dynamic, that is, the sizes table
 *	becomes an integral part of the kernel during system generation.
 *	Changes to the partition layout do not take effect until you make
 *	and install a new kernel. The recommended procedure is to study your
 *	file system needs carefully, then make any necessary disk partition
 *	layout changes before setting up any user file systems. Subsequent
 *	disk layout changes are strongly discouraged, because of possible
 *	damage to existing file systems and the unpleasant possibility of
 *	accidently booting an old kernel with an invalid sizes table.
 *
 *  5.	Disk partitions must start on a cylinder boundary.
 *
 *  6.	Disk partitions may overlap, but you must make sure that only one
 *	overlapping partition is used on any given drive.
 *
 *  7.	If you use previously unused partitions the setup program will
 *	not automatically make the special files for those partitions.
 *	You should use the -f option of the msf command to make the
 *	special files for these newly used partitions. See msf(1) for
 *	more information about the msf -f option.
 *
 *  8.	The last 44 sectors of the last cylinder are reserved for the bad
 *	sector file and replacement sectors. This reserved area must not be
 *	included in any disk partition.
 *

 *
 * RK06/7 DISK LAYOUT:
 *
 *	First number is the partition number.
 *	Second number is the partition size in 512 byte sectors.
 *	Third is the partition usage.
 *
 *
 *		RK06
 *	+-----------------------+
 *	| 0   7920  root	|
 *	|			|
 *	+-----------------------+
 *	| 1    100  error log	|
 *	|     2936  swap	|
 *	+-----------------------+
 *	| 2  16126  /usr	|
 *	|			|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	| 44 bad sector file	|
 *	+-----------------------+
 *	  6  27082  user
 *
 *
 *		RK07
 *	+-----------------------+
 *	| 0   7920  root	|
 *	|			|
 *	+-----------------------+
 *	| 1    100  error log	|
 *	|     2936  swap	|
 *	+-----------------------+
 *	| 3  42790  /usr	|
 *	|			|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	| 44 bad sector file	|
 *	+-----------------------+
 *	  7  53746  user
 *

 *
 * SIZES STRUCTURE DEFINITION:
 *
 *	struct hksize {
 *		daddr_t	nblocks;
 *		int	cyloff;
 *	};
 *
 *	nblocks - Defines the length of a partition in 512 byte sectors.
 *		  Two sectors make up one 1024 byte file system logical block.
 *
 *	cyloff	- Defines where the partition begins, that is, its starting
 *		  cylinder number relative to the physical beginning of the
 *		  disk.
 *
 * DISK GEOMETRY INFORMATION:
 *
 *	RK06 has 411 cylinders with 66 sectors per cylinder.
 *	RK07 has 815 cylinders with 66 sectors per cylinder.
 *
 *	Note - last 44 sectors of last cylinder reserved for bad sector file.
 *
 */

#if	NHK > 0
struct	hksize {
	daddr_t	nblocks;
	int	cyloff;
} hk_sizes[8] = {
	7920,	0,	/* 0: cyl   0-119, root on rk06/7		*/
	3036,	120,	/* 1: cyl 120-165, swap+errlog on rk06/7	*/
	16126,	166,	/* 2: cyl 166-409(+22 blks), /usr on rk06	*/
	42790,	166,	/* 3: cyl 166-813(+22 blks), /usr on rk07	*/
	0,	0,	/* 4: not used					*/
	0,	0,	/* 5: not used					*/
	27082,	0,	/* 6: cyl   0-409(+22 blks), all of rk06	*/
	53746,	0,	/* 7: cyl   0-813(+22 blks), all of rk07	*/
};
#endif


/*
 * RP - RP02/3 Disk Partition Layout
 *
 * GENERAL RULES:
 *
 *  1.	If at all possible, you should use the standard disk partition
 *	layout. If you choose to change the disk partition layout, you
 *	own the responsibility for any file system damage and/or loss of
 *	data caused by an improper disk partition layout.
 *
 *	MAKE A BACKUP COPY OF THIS FILE BEFORE IMPLEMENTING ANY CHANGES
 *
 *  2.	Do not change the RP partition layout if the RP02/3 is the system
 *	disk, that is, the disk where the root, swap, error log, and /usr
 *	file systems are located.
 *
 *  3.	The partition layout is defined, by the sizes table shown below, on
 *	a per controller basis not per drive. The sizes must be the same for
 *	each drive on the controller. However, not all partitions are used
 *	on all drives.
 *
 *  4.	The disk partition layout is not dynamic, that is, the sizes table
 *	becomes an integral part of the kernel during system generation.
 *	Changes to the partition layout do not take effect until you make
 *	and install a new kernel. The recommended procedure is to study your
 *	file system needs carefully, then make any necessary disk partition
 *	layout changes before setting up any user file systems. Subsequent
 *	disk layout changes are strongly discouraged, because of possible
 *	damage to existing file systems and the unpleasant possibility of
 *	accidently booting an old kernel with an invalid sizes table.
 *
 *  5.	Disk partitions must start on a cylinder boundary.
 *
 *  6.	Disk partitions may overlap, but you must make sure that only one
 *	overlapping partition is used on any given drive.
 *
 *  7.	If you use previously unused partitions the setup program will
 *	not automatically make the special files for those partitions.
 *	You should use the -f option of the msf command to make the
 *	special files for these newly used partitions. See msf(1) for
 *	more information about the msf -f option.
 *

 *
 * RP02/3 DISK LAYOUT:
 *
 *	First number is the partition number.
 *	Second number is the partition size in 512 byte sectors.
 *	Third is the partition usage.
 *
 *		RP02
 *	+-----------------------+
 *	| 0   8400  root	|
 *	|			|
 *	+-----------------------+
 *	| 1    100  error log	|
 *	|     3100  swap	|
 *	+-----------------------+
 *	| 2  28400  /usr	|
 *	|			|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	  6  40000  user
 *
 *
 *		RP03
 *	+-----------------------+
 *	| 0   8400  root	|
 *	|			|
 *	+-----------------------+
 *	| 1    100  error log	|
 *	|     3100  swap	|
 *	+-----------------------+
 *	| 3  68400  /usr	|
 *	|			|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	  7  80000  user
 *

 *
 * SIZES STRUCTURE DEFINITION:
 *
 *	struct rpsize {
 *		daddr_t	nblocks;
 *		int	cyloff;
 *	};
 *
 *	nblocks - Defines the length of a partition in 512 byte sectors.
 *		  Two sectors make up one 1024 byte file system logical block.
 *
 *	cyloff	- Defines where the partition begins, that is, its starting
 *		  cylinder number relative to the physical beginning of the
 *		  disk.
 *
 * DISK GEOMETRY INFORMATION:
 *
 *	RP02 has 200 cylinders with 200 sectors per cylinder.
 *	RP03 has 400 cylinders with 200 sectors per cylinder.
 *
 */

#if	NRP > 0
struct	rpsize {
	daddr_t	nblocks;
	int	cyloff;
} rp_sizes[] = {
	8400,	0,	/* 0: cyl   0- 41, root file system		*/
	3200,	42,	/* 1: cyl  42- 57, swap + error log		*/
	28400,	58,	/* 2: cyl  58-199, /usr for RP02		*/
	68400,	58,	/* 3: cyl  58-399, /usr for RP03		*/
	0,	0,	/* 4: not used					*/
	0,	0,	/* 5: not used					*/
	40000,	0,	/* 6: cyl   0-199, all of RP02 pack		*/
	80000,	0,	/* 7: cyl   0-399, all of RP03 pack		*/
};
#endif


/*
 * HP - RH11/RH70 MASSBUS Disk Partition Layout
 *
 *	HP - First  RH11/RH70 with RM02/3/5, RP04/5/6, ML11 disks
 *	HM - Second RH11/RH70 with RM02/3/5, RP04/5/6, ML11 disks
 *	HJ - Third  RH11/RH70 with RM02/3/5, RP04/5/6, ML11 disks
 *
 * GENERAL RULES:
 *
 *  1.	If at all possible, you should use the standard disk partition
 *	layout. If you choose to change the disk partition layout, you
 *	own the responsibility for any file system damage and/or loss of
 *	data caused by an improper disk partition layout.
 *
 *	MAKE A BACKUP COPY OF THIS FILE BEFORE IMPLEMENTING ANY CHANGES
 *
 *  2.	If the system disk is on an RH11/RH70 controller, do not change any
 *	of the partitions listed below. The system disk is where the root,
 *	swap, error log, and /usr file systems reside. Do not change the
 *	ML11 partitions, they are filled in by the driver.
 *
 *	Disks		Fixed Partitions
 *	-----		----------------
 *	RM02/3/5	0, 1, 2, & 7
 *	RP04/5		0, 1, 2, & 6
 *	RP06		0, 1, 2, 6, & 7
 *
 *  3.	The partition layout is defined, by the sizes table shown below, on
 *	a per controller basis not per drive. The sizes must be the same for
 *	each drive on the controller. However, not all partitions are used
 *	on all drives.
 *
 *  4.	The disk partition layout is not dynamic, that is, the sizes table
 *	becomes an integral part of the kernel during system generation.
 *	Changes to the partition layout do not take effect until you make
 *	and install a new kernel. The recommended procedure is to study your
 *	file system needs carefully, then make any necessary disk partition
 *	layout changes before setting up any user file systems. Subsequent
 *	disk layout changes are strongly discouraged, because of possible
 *	damage to existing file systems and the unpleasant possibility of
 *	accidently booting an old kernel with an invalid sizes table.
 *
 *  5.	Disk partitions must start on a cylinder boundary.
 *
 *  6.	Disk partitions may overlap, but you must make sure that only one
 *	overlapping partition is used on any given drive.
 *
 *  7.	If you use previously unused partitions the setup program will
 *	not automatically make the special files for those partitions.
 *	You should use the -f option of the msf command to make the
 *	special files for these newly used partitions. See msf(1) for
 *	more information about the msf -f option.

 *
 *  8.	The last N sectors of the last cylinder are reserved for the bad
 *	sector file and replacement sectors. This reserved area must not be
 *	included in any disk partition.
 *
 *	N = 44 for RP04/5/6, N = 64 for RM02/3/5
 *
 *  9.	The UEG_LOCAL definition allows Digital's ULTRIX Engineering Group
 *	to implement its own partition layout for RM02/3 disks. You should
 *	make any local changes to the RM02/3 sizes table following the #else
 *	statement.
 *
 *
 * RH11/RH70 MASSBUS DISKS LAYOUT:
 *
 *	First number is the partition number.
 *	Second number is the partition size in 512 byte sectors.
 *	Third is the partition usage.
 *
 *
 *
 *		RM02/3
 *	+-----------------------+
 *	| 0    9120  root	|
 *	|			|
 *	+-----------------------+
 *	| 1   20000  /usr	|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	| 2     200  error log	|
 *	|      5400  swap	|
 *	+-----------------------+
 *	| 3   96896  user	|
 *	|			|
 *	|			|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	| 64 bad block file	|
 *	+-----------------------+
 *	  7 131616  user
 *

 *
 *		RM05
 *	+-----------------------+
 *	| 0   10336  root	|
 *	|			|
 *	+-----------------------+
 *	| 1   21280  /usr	|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	| 2     300  error log	|
 *	|      6388  swap	|
 *	+-----------------------+-----------------------+
 *	| 3  462016  user	| 4  153824  user	|
 *	|			|			|
 *	|			|			|
 *	|			|			|
 *	|			+-----------------------+
 *	|			| 5  153824  user	|
 *	|			|			|
 *	|			|			|
 *	|			|			|
 *	|			+-----------------------+
 *	|			| 6  154368  user	|
 *	|			|			|
 *	|			|			|
 *	|			|			|
 *	+-----------------------+-----------------------+
 *	| 64 bad block file				|
 *	+-----------------------+-----------------------+
 *	  7  500320  user

 *		RP04/5
 *	+-----------------------+
 *	| 0    9614  root	|
 *	|			|
 *	+-----------------------+
 *	| 1   20064  /usr	|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	| 2     200  error log	|
 *	|      6070  swap	|
 *	+-----------------------+
 *	| 3  135806  user	|
 *	|			|
 *	|			|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	| 44 bad block file	|
 *	+-----------------------+
 *	  6  171754  user
 *
 *
 *		RP06
 *	+-----------------------+
 *	| 0    9614  root	|
 *	|			|
 *	+-----------------------+
 *	| 1   20064  /usr	|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	| 2     200  error log	|
 *	|      6070  swap	|
 *	+-----------------------+-----------------------+
 *	| 4  304678  user	| 3  135806  user	|
 *	|			|			|
 *	|			|			|
 *	|			+-----------------------+
 *	|			| 44 unused		|
 *	|			+-----------------------+
 *	|			| 5  168828  user	|
 *	|			|			|
 *	|			|			|
 *	|			|			|
 *	+-----------------------+-----------------------+
 *	| 44 bad block file				|
 *	+-----------------------+-----------------------+
 *	  6  171754  user
 *	  7  340626  user
 *
 *	Note - the RP06 can be split into two large sections
 *	       by using partitions 5 and 6.
 *

 *
 * SIZES STRUCTURE DEFINITION:
 *
 *	struct hpsize {
 *		daddr_t	nblocks;
 *		int	cyloff;
 *	};
 *
 *	nblocks - Defines the length of a partition in 512 byte sectors.
 *		  Two sectors make up one 1024 byte file system logical block.
 *
 *	cyloff	- Defines where the partition begins, that is, its starting
 *		  cylinder number relative to the physical beginning of the
 *		  disk.
 *
 * DISK GEOMETRY INFORMATION:
 *
 *	RM02/3 have 823 cylinders with 160 sectors per cylinder.
 *	RM05 has 823 cylinders with 608 sectors per cylinder.
 *	RP04/5 have 411 cylinders with 418 sectors per cylinder.
 *	RP06 has 815 cylinders with 418 sectors per cylinder.
 *
 *	Note - last 44 sectors (RP04/5/6) or last 64 sectors (RM02/3/5)
 *	       of the last cylinder are reserved for the bad sector file.
 *
 */


#if	NRH > 0
struct	hpsize {
	daddr_t	nblocks;
	int	cyloff;
} hp_sizes[32] = {	/* size of 32 also hardwired into HPX */
 			/* RP04/5/6 disk sizes table */
	9614,	0,	/* 0: cyl   0- 22, root file system		*/
	20064,	23,	/* 1: cyl  23- 70, /usr file system		*/
	6270,	71,	/* 2: cyl  71- 85, swap + error log		*/
	135806,	86,	/* 3: cyl  86-409(+374 blks), user		*/
	304678,	86,	/* 4: cyl  86-813(+374 blks), user		*/
	168828,	411,	/* 5: cyl 411-813(+374 blks), user		*/
	171754,	0,	/* 6: cyl   0-409(+374 blks), user - all rp04/5	*/
	340626,	0,	/* 7: cyl   0-813(+374 blks), user - all rp06	*/

			/* RM02/3 disk sizes table */
	9120,	0,	/* 0: cyl   0- 56, root file system		*/
	20000,	57,	/* 1: cyl  57-181, swap + error log		*/
	5600,	182,	/* 2: cyl 182-216, /usr file system		*/
	96896,	217,	/* 3: cyl 217-821(+96 blks), user		*/
#ifdef	UEG_LOCAL
	32160,	217,	/* 4: cyl 217-417, UEG kit building		*/
	32160,	418,	/* 5: cyl 418-618, UEG kit building		*/
	32576,	619,	/* 6: cyl 619-821(+96 blks), UEG kit building	*/
#else
	0,	0,	/* 4: not used					*/
	0,	0,	/* 5: not used					*/
	0,	0,	/* 6: not used					*/
#endif	UEG_LOCAL
	131616,	0,	/* 7: cyl   0-821(+96 blks), user - entire disk	*/

			/* RM05 disk sizes table */
	10336,	0,	/* 0: cyl   0- 16, root file system		*/
	21280,	17,	/* 1: cyl  17- 51, /usr file system		*/
	6688,	52,	/* 2: cyl  52- 62, swap + error log		*/
	462016,	63,	/* 3: cyl  63-821(+544 blks), user		*/
	153824,	63,	/* 4: cyl  63-315, user				*/
	153824,	316,	/* 5: cyl 316-568, user				*/
	154368, 569,	/* 6: cyl 569-821(+544 blks), user		*/
	500320,	0,	/* 7: cyl   0-821(+544 blks), user - whole pack	*/

			/* ML11 solid state disk sizes table */
	0,	0,	/* ML11  unit	0	*/
	0,	0,	/*		1	*/
	0,	0,	/*		2	*/
	0,	0,	/*		3	*/
	0,	0,	/*		4	*/
	0,	0,	/*		5	*/
	0,	0,	/*		6	*/
	0,	0,	/*		7	*/
};
#endif


/*
 * RA - MSCP Disk Partition Layout
 *
 *	UDA50 - RA60/RA80/RA81
 *	KLESI - RC25
 *	RQDX1/2/3 - RD31/RD32/RD51/RD52/RD53/RD54/RX50/RX33
 *	RUX1  - RX50
 *
 * GENERAL RULES:
 *
 *  1.	If at all possible, you should use the standard disk partition
 *	layout. If you choose to change the disk partition layout, you
 *	own the responsibility for any file system damage and/or loss of
 *	data caused by an improper disk partition layout.
 *
 *	MAKE A BACKUP COPY OF THIS FILE BEFORE IMPLEMENTING ANY CHANGES
 *
 *  2.	The system disk is where the root, swap, error log, and /usr file
 *	systems reside. If the system disk is an RA60/RA80/RA81, do not
 *	change partitions 0, 1, 2, and 7. If the system disk is an RC25,
 *	do not change partitions 0, 1, 2, and 7. RC25 partitions 4, 5, and
 *	6 may be used on non system disks.
 *
 *	Do not change any partitions for RD31/RD32/RD51/RD52/RD53/RD54 disks.
 *	Never changes partition 7 for any MSCP disk.
 *
 *  3.	The partition layout is defined, by the sizes table shown below, on
 *	a per controller basis not per drive. The sizes must be the same for
 *	each drive on the controller. However, not all partitions are used
 *	on all drives.
 *
 *  4.	The disk partition layout is not dynamic, that is, the sizes table
 *	becomes an integral part of the kernel during system generation.
 *	Changes to the partition layout do not take effect until you make
 *	and install a new kernel. The recommended procedure is to study your
 *	file system needs carefully, then make any necessary disk partition
 *	layout changes before setting up any user file systems. Subsequent
 *	disk layout changes are strongly discouraged, because of possible
 *	damage to existing file systems and the unpleasant possibility of
 *	accidently booting an old kernel with an invalid sizes table.
 *
 *  5.	Disk partitions may overlap, but you must make sure that only one
 *	overlapping partition is used on any given drive.
 *
 *  6.	If you use previously unused partitions the setup program will
 *	not automatically make the special files for those partitions.
 *	You should use the -f option of the msf command to make the
 *	special files for these newly used partitions. See msf(1) for
 *	more information about the msf -f option.
 *
 *  7.	The MSCP disks have a maintenance area at the end of the disk. This
 *	area is reserved for use by the MSCP disk exerciser (RAX). No disk
 *	partition may overlap the maintenance area.
 *

 *
 * MSCP DISKS LAYOUT:
 *
 *	First number is the partition number.
 *	Second number is the partition size in 512 byte sectors.
 *	Third is the partition usage.
 *
 *
 *
 *		RD31
 *	+-----------------------+
 *	| 0   9700  root	|
 *	|			|
 *	+-----------------------+
 *	| 5    100  error log	|
 *	|     3000  swap	|
 *	+-----------------------+
 *	| 6  28728  /usr	|
 *	|			|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	| 32 maintenance area	|
 *	+-----------------------+
 *	  7  41560  (user = 41528)
 *
 *
 *		RD32
 *	+-----------------------+
 *	| 0   9700  root	|
 *	|			|
 *	+-----------------------+
 *	| 1  17300  /usr	|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	| 2    100  error log	|
 *	|     3000  swap	|
 *	+-----------------------+
 *	| 3  53072  user	|
 *	|			|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	| 32 maintenance area	|
 *	+-----------------------+
 *	  7  83204  (user = 83172)

 *
 *		RD51
 *	+-----------------------+
 *	| 0   7460  root	| (9260 total)
 *	|       40  error log	|
 *	|     2200  swap	|
 *	+-----------------------+
 *	| 4  11868  /usr	|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	|  32 maintenance area	|
 *	+-----------------------+
 *	  7  21600  (user = 21568)
 *
 *
 *		RD52
 *	+-----------------------+
 *	| 0   9700  root	|
 *	|			|
 *	+-----------------------+
 *	| 1  17300  /usr	|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	| 2    100  error log	|
 *	|     3000  swap	|
 *	+-----------------------+
 *	| 3  30348  user	|
 *	|			|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	| 32 maintenance area	|
 *	+-----------------------+
 *	  7  60480  (user = 60448)

 *
 *		RD53
 *	+-----------------------+
 *	| 0   9700  root	|
 *	|			|
 *	+-----------------------+
 *	| 1  17300  /usr	|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	| 2    100  error log	|
 *	|     3000  swap	|
 *	+-----------------------+
 *	| 3 108540  user	|
 *	|			|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	| 32 maintenance area	|
 *	+-----------------------+
 *	  7  138672  (user = 138640)
 *
 *
 *		RD54
 *	+-----------------------+
 *	| 0   9700  root	|
 *	|			|
 *	+-----------------------+
 *	| 1  17300  /usr	|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	| 2    100  error log	|
 *	|     3000  swap	|
 *	+-----------------------+
 *	| 3 281068  user	|
 *	|			|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	| 32 maintenance area	|
 *	+-----------------------+
 *	  7  311200  (user = 311168)
 *
 *
 *		RC25
 *	+-----------------------+
 *	| 0   9000  root	|
 *	|			|
 *	+-----------------------+
 *	| 1    200  error log	|
 *	|     4000  swap	|
 *	+-----------------------+
 *	| 2  37600  user	|
 *	|			|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	| 102 maintenance area	|
 *	+-----------------------+
 *	  7  50902  (user = 50800)

 *
 *		RA80
 *	+-----------------------+
 *	| 0    9600  root	|
 *	|			|
 *	+-----------------------+
 *	| 1   20000  /usr	|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	| 2     200  error log	|
 *	|      6000  swap	|
 *	+-----------------------+
 *	| 3  200412  user	|
 *	|			|
 *	|			|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	| 1000 maintenance area	|
 *	+-----------------------+
 *	  7 = 237212  (user = 236212)
 *
 *
 *		RA60
 *	+-----------------------+
 *	| 0    9600  root	|
 *	|			|
 *	+-----------------------+
 *	| 1   20000  /usr	|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	| 2     200  error log	|
 *	|      6000  swap	|
 *	+-----------------------+-----------------------+
 *	| 3  363376  user	| 4  181688  user	|
 *	|			|			|
 *	|			|			|
 *	|			|			|
 *	|			+-----------------------+
 *	|			| 5  181688  user	|
 *	|			|			|
 *	|			|			|
 *	|			|			|
 *	+-----------------------+-----------------------+
 *	| 1000 maintenance area				|
 *	+-----------------------+-----------------------+
 *	  7 = 400176  (user = 399176)
 *
 *
 *		RA81
 *	+-----------------------+
 *	| 0    9600  root	|
 *	|			|
 *	+-----------------------+
 *	| 1   20000  /usr	|
 *	|			|
 *	|			|
 *	+-----------------------+
 *	| 2     200  error log	|
 *	|      6000  swap	|
 *	+-----------------------+-----------------------+
 *	| 3  854272  user	| 4  181688  user	|
 *	|			|			|
 *	|			|			|
 *	|			|			|
 *	|			+-----------------------+
 *	|			| 5  181688  user	|
 *	|			|			|
 *	|			|			|
 *	|			|			|
 *	|			+-----------------------+
 *	|			| 6  490896  user	|
 *	|			|			|
 *	|			|			|
 *	|			|			|
 *	|			|			|
 *	|			|			|
 *	|			|			|
 *	+-----------------------+-----------------------+
 *	| 1000 maintenance area				|
 *	+-----------------------+-----------------------+
 *	  7 = 891072  (user = 890072)

 *
 * SIZES STRUCTURE DEFINITION:
 *
 *	struct rasize {
 *		daddr_t	nblocks;
 *		daddr_t	blkoff;
 *	};
 *
 *	nblocks - Defines the length of a partition in 512 byte blocks.
 *		  Two blocks make up one 1024 byte file system logical block.
 *
 *	Note	- nblocks has two values with special meaning:
 *
 *		  An nblocks value of -1 specifies a partition length of
 *		  the size of the entire disk minus the partition's starting
 *		  block number. In other words, the partition ends at the
 *		  end of the disk.
 *
 *		  An nblocks value of -2 specifies a partition length of
 *		  the size of the disk minus the size of the maintenance
 *		  area minus the starting block number of the partition.
 *		  In other words, the partition ends at the start of the
 *		  maintenance area.
 *
 *	blkoff	- Defines where the partition begins, that is, its starting
 *		  block number relative to the physical beginning of the disk.
 *
 * DISK GEOMETRY INFORMATION:
 *
 *	For the MSCP disks, the sizes table is defined in terms of block
 *	numbers. The concept of cylinders is not used.
 *
 *
 */


#if	NUDA > 0

/*
 * The rasize structure is defined in /usr/include/sys/ra_info.h.
 */

struct	rasize	ud_sizes[8] = {	/* UDA50-RA60/RA80/RA81 */
	9600,	0,	/* 0: root 			*/
	20000,	9600,	/* 1: /usr			*/
	6200,	29600,	/* 2: swap + error log		*/
	-2,	35800,	/* 3: user			*/
	181688,	35800,	/* 4: user			*/
	181688,	217488,	/* 5: user			*/
	-2,	399176,	/* 6: user			*/
	-1,	0,	/* 7: user			*/
};

/*
 * CAUTION:
 *	If your distribution or system disk is an RC25,
 *	do not change partitions 3 or 4. The sdload and
 *	setup programs depend on partitions 3 and 4
 *	being known locations.
 */
struct	rasize	rc_sizes[8] = {	/* KLESI-RC25 */
	9000,	0,	/* 0: root			*/
	4200,	9000,	/* 1: swap + error log		*/
	-2,	13200,	/* 2: /usr			*/
	12000,	13200,	/* 3: DISTRIBUTION DISK ONLY	*/
	-2,	25200,	/* 4: DISTRIBUTION DISK ONLY	*/
	0,	0,	/* 5: not used			*/
	0,	0,	/* 6: not used			*/
	-1,	0,	/* 7: user			*/
};

/*
 * Sizes for RD31/RD32/RD51/RD52/RD53/RD54/RX50/RX33 disks.
 */
struct	rasize	rq_sizes[8] = {	/* RQDX1/RQDX2/RQDX3/RUX1 */
	9700,	0,		/* RD31-32,51-54 root (swap+error log - rd51) */
	17300,	9700,		/* RD32/RD52/RD53/RD54 /usr */
	3100,	27000,		/* RD32/RD52/RD53/RD54 swap + error log */
	-2,	30100,		/* RD32/RD52/RD53/RD54 user files */
	-2,	9700,		/* RD51 /usr */
	3100,	9700,		/* RD31 swap + error log */
	-2,	12800,		/* RD31 /usr */
	-1,	0,		/* RD31-32, RD51-54 entire disk */
};
#endif
