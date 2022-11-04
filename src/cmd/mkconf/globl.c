
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)globl.c	3.0	4/21/86";

/*
 * This file contains all the declarations for the global variables
 * used by mkconf.
 */

#include "mkconf.h"

struct	ovdes	ovdtab [16];

int	sid = 1;
int	ov;
int	ubmap = 1;
int	generic = 0;
int	nfp;
int	dump = 0;
int	rootmaj = -1;
int	rootmin;
int	swapmaj = -1;
int	swapmin;
int	pipemaj = -1;
int	pipemin;
long	swplo	= 4000;
int	nswap = 800;
int	dumpdn = -1;
long	dumplo = -1;
long	dumphi = -1;
int	kfpsim;
int	nldisp = 2;
int	elmaj = -1;
int	elmin;
long	elsb = 4800;
int	elnb = 72;
int	nbuf = 20;
int	ninode = 80;
int	nfile = 75;
int	nmount = 5;
int	nproc = 75;
int	ntext = 25;
int	nclist	= 50;
long	ulimit = 1024;
int	maxuprc = 15;
int	mapsize = -1;
int	elbsiz;
int	timezone = 5;
int	dstflag = 1;
int	ncargs = 5120;
int	hz = 60;
int	msgbufs = 128;
unsigned int maxseg = 61440;	/* (4mb - I/O page) memory limit is default */
int	ncall = 25;
int	first;

char	omn[20];
char	mtsize[20];
int	ipc = 0;
int	sema = 0; 
int	semmap = 10;
int	semmni = 10;
int	semmns = 60;
int	semmnu = 30;
int	semume = 10;
int	semmsl = 25;
int	semopm = 10;
int	semvmx = 32767;
int	semaem = 16384;
int	mesg = 0;
int	msgmax = 8192;
int	msgmnb = 16384;
int	msgtql = 40;
int	msgssz = 8;
int	msgseg = 1024;
int	msgmap = 100;
int	msgmni = 10;
int	flock = 0;
int	flckrec = 100;
int	flckfil = 25;
int	maus = 0;
int	nmaus = 0;
int	mausize[8] = {0,0,0,0,0,0,0,0};
int	shuffle = 0;
int	network = 0;
int	allocs = -1;
int	mbufs = -1;
int	miosize = 0;
int	npty = 2;
/* so can pass vector to attach routine: */
/* char	netattach[8][16]; */
struct netattach netattach[6];
	 
int	nnetattach = 0;
