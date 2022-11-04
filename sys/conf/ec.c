
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985.	      *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/include/COPYRIGHT" for applicable restrictions.  *
 **********************************************************************/

/*
 * SCCSID: @(#)ec.c	3.0	4/21/86
 */

#include "dds.h"
#include <sys/param.h>
#include <sys/file.h>
#include <sys/proc.h>
#include <sys/text.h>
#include <sys/callo.h>
#include <sys/map.h>
#include <sys/maus.h>
#include <sys/flock.h>

int		mb_end;
char		hostname[32];
struct {
	int	nbufr;
	long	nread;
	long	nreada;
	long	ncache;
	long	nwrite;
	long	bufcount[NBUF];
}	io_info;

int		el_buf[ELBSIZ+1];
struct	callo	callout[NCALL];
struct	file	file[NFILE];
struct	proc	proc[NPROC];
struct	text	text[NTEXT];

#ifdef MAUS
int	mauscore;		/* Start of MAUS segments */
int	mausend;		/* Last core address of MAUS regions */
int	nmausent;		/* Number of entries in mausmap */

#endif

#ifdef MESG
#include	<sys/ipc.h>
#include	<sys/msg.h>

struct map	msgmap[MSGMAP];
struct msqid_ds	msgque[MSGMNI];
struct msg	msgh[MSGTQL];
struct msginfo	msginfo = {
	MSGMAP,
	MSGMAX,
	MSGMNB,
	MSGMNI,
	MSGSSZ,
	MSGTQL,
	MSGSEG
};
#endif

#ifdef SEMA
#ifndef IPC_ALLOC
#include <sys/ipc.h>
#endif
#include	<sys/sem.h>
struct semid_ds	sema[SEMMNI];
struct sem	sem[SEMMNS];
struct map	semmap[SEMMAP];
struct	sem_undo	*sem_undo[NPROC];
#define	SEMUSZ	(sizeof(struct sem_undo)+sizeof(struct undo)*SEMUME)
int	semu[((SEMUSZ*SEMMNU)+NBPW-1)/NBPW];
union {
	short		semvals[SEMMSL];
	struct semid_ds	ds;
	struct sembuf	semops[SEMOPM];
}	semtmp;

struct	seminfo seminfo = {
	SEMMAP,
	SEMMNI,
	SEMMNS,
	SEMMNU,
	SEMMSL,
	SEMOPM,
	SEMUME,
	SEMUSZ,
	SEMVMX,
	SEMAEM
};

#endif

#ifdef FLOCK
struct flckinfo flckinfo = {
	FLCKREC,
	FLCKFIL,
	0,
	0,
	0,
	0
};

struct filock flox[FLCKREC];
struct flino flinotab[FLCKFIL];
#endif
