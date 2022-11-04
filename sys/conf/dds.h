
/*
 * Disk driver data structure sizes header file (dds.h).
 * Controls number and size of disk data structures in
 * (dds.c), for MSCP (ra.c) and MASSBUS (hp.c) disk drivers.
 */


#define	NUDA	2
#define	C0_NRA	2
#define	C0_CSR	0172150
#define	C0_VEC	0154
#define	C0_RS	8
#define	C0_RSL2	3
#define	C1_NRA	2
#define	C1_CSR	0172144
#define	C1_VEC	0150
#define	C1_RS	2
#define	C1_RSL2	1
#define	C2_NRA	0
#define	C2_CSR	00
#define	C2_VEC	00
#define	C2_RS	0
#define	C2_RSL2	0
#define	C3_NRA	0
#define	C3_CSR	00
#define	C3_VEC	00
#define	C3_RS	0
#define	C3_RSL2	0


#define	NRH	2
#define	C0_NHP	4
#define	C1_NHP	3
#define	C2_NHP	0
#define	C3_NHP	0

#define	NTS	0

#define	NTK	0


#define	NHK	0
#define	NRP	0

#define	NPROC	135
#define	NTEXT	40
#define	NFILE	150
#define	NBUF	72
#define	ELBSIZ	350
#define	NCALL	40

#define	SHUFFLE	1
#define	MAUS	1
#define	FLOCK	1
#define	FLCKREC	90
#define	FLCKFIL	25
#define	MESG	1
#define	MSGMAP	60
#define	MSGMAX	8192
#define	MSGMNB	16384
#define	MSGMNI	10
#define	MSGSSZ	8
#define	MSGTQL	30
#define	MSGSEG	1024
#define	SEMA	1
#define	SEMMAP	5
#define	SEMMNI	5
#define	SEMMNS	30
#define	SEMMNU	15
#define	SEMMSL	20
#define	SEMOPM	10
#define	SEMUME	5
#define	SEMVMX	32767
#define	SEMAEM	16384
#define	NDE	1
