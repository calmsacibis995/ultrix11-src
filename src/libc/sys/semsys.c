/* SCCSID: @(#)semsys.c	3.0	4/22/86	*/

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* Based on:	@(#)semsys.c	1.1	(SYSTEM V)	*/
#include	"sys/types.h"
#include	"sys/ipc.h"
#include	"sys/sem.h"

#define	SEMSYS	79

#define	SEMCTL	0
#define	SEMGET	1
#define	SEMOP	2

semctl(semid, semnum, cmd, arg)
int semid, cmd;
int semnum;
union semun {
	int val;
	struct semid_ds *buf;
	ushort array[1];
} arg;
{
	return(syscall(SEMSYS, SEMCTL, semid, semnum, cmd, arg));
}

semget(key, nsems, semflg)
key_t key;
int nsems, semflg;
{
	return(syscall(SEMSYS, SEMGET, key, nsems, semflg));
}

semop(semid, sops, nsops)
int semid;
struct sembuf (*sops)[];
int nsops;
{
	return(syscall(SEMSYS, SEMOP, semid, sops, nsops));
}
