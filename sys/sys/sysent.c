
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)sysent.c	3.0	4/21/86
 */
/*
 * If you add/delete/change this file, make sure you update
 * /usr/include/sys/systm.h!!!!
 */
/*
 * Added System V Compatability calls:
 *	fcntl maus msgsys semsys ulimit utssys
 * Ohms 4/30/1985
 *
 * The getfp(), ttlocl(), fperr(), errlog(), and bdflush()
 * system calls were added for Unix/v7m.
 *
 * Fred Canter 12/10/81
 */

#include <sys/param.h>
#include <sys/systm.h>
#ifdef	UCB_NET
#include <sys/socket.h>
#endif

/*
 * This table is the switch used to transfer
 * to the appropriate routine for processing a system call.
 * Each row contains the number of arguments expected
 * and a pointer to the routine.
 */
int	alarm();
int	chdir();
int	chmod();
int	chown();
int	chroot();
int	close();
int	creat();
int	dup();
int	exec();
int	exece();
int	fcntl();
int	fork();
int	fstat();
int	getgid();
int	getpid();
int	getuid();
int	gtime();
int	gtty();
int	ioctl();
int	kill();
int	link();
int	maus();
int	msgsys();
int	mknod();
int	nice();
int	nosys();
int	nullsys();
int	open();
int	pause();
int	pipe();
int	profil();
int	ptrace();
int	read();
int	rexit();
int	saccess();
int	sbreak();
int	seek();
int	semsys();
int	setgid();
int	setuid();
int	smount();
int	ssig();
int	stat();
int	stime();
int	stty();
int	sumount();
int	ftime();
int	sync();
int	sysacct();
int	syslock();
int	sysphys();
int	times();
int	ulimit();
int	umask();
int	unlink();
int	utime();
int	utssys();
int	wait();
int	write();
int	getfp();
int	ttlocl();
int	fperr();
int	errlog();
int	bdflush();
int	setpgrp();

int	zaptty();
int	gethostid();
int	sethostid();
#ifdef	SELECT
int	select();
#endif
int	gethostname();
int	sethostname();
int	fpsim();
int	evntflg();
int	renice();
int	nostk();
int	nap();
int	setreuid(), setregid();
int	gethostid(), sethostid();
#ifdef	UCB_NET
int	socket(), bind(), listen(), accept(), connect(), socketpair();
int	setsockopt(), getsockopt(), getsockname(), getpeername();
int	shutdown();
int	sendit(), recvit();
#endif

#ifdef	UCB_SYMLINKS
int	lstat();
int	readlink();
int	symlink();
#endif

#ifdef	NEWLIMITS
int	lim();
#endif	NEWLIMITS

struct sysent sysent[SYSMAX] =
{
	1, 0, nullsys,		/*  0 = indir */
	1, 1, rexit,		/*  1 = exit */
	0, 0, fork,		/*  2 = fork */
	3, 1, read,		/*  3 = read */
	3, 1, write,		/*  4 = write */
	3, 0, open,		/*  5 = open, now 3 args */
	1, 1, close,		/*  6 = close */
	0, 0, wait,		/*  7 = wait */
	2, 0, creat,		/*  8 = creat */
	2, 0, link,		/*  9 = link */
	1, 0, unlink,		/* 10 = unlink */
	2, 0, exec,		/* 11 = exec */
	1, 0, chdir,		/* 12 = chdir */
	0, 0, gtime,		/* 13 = time */
	3, 0, mknod,		/* 14 = mknod */
	2, 0, chmod,		/* 15 = chmod */
	3, 0, chown,		/* 16 = chown; now 3 args */
	1, 0, sbreak,		/* 17 = break */
	2, 0, stat,		/* 18 = stat */
	4, 1, seek,		/* 19 = seek; now 3 args */
	0, 0, getpid,		/* 20 = getpid */
	3, 0, smount,		/* 21 = mount */
	1, 0, sumount,		/* 22 = umount */
	1, 1, setuid,		/* 23 = setuid */
	0, 0, getuid,		/* 24 = getuid */
	2, 2, stime,		/* 25 = stime */
	4, 1, ptrace,		/* 26 = ptrace */
	1, 1, alarm,		/* 27 = alarm */
	2, 1, fstat,		/* 28 = fstat */
	0, 0, pause,		/* 29 = pause */
	2, 0, utime,		/* 30 = utime */
	2, 1, stty,		/* 31 = stty */
	2, 1, gtty,		/* 32 = gtty */
	2, 0, saccess,		/* 33 = access */
	1, 1, nice,		/* 34 = nice */
	1, 0, ftime,		/* 35 = ftime; formerly sleep */
	0, 0, sync,		/* 36 = sync */
	2, 1, kill,		/* 37 = kill */
	0, 0, nullsys,		/* 38 = switch; inoperative */
	2, 1, setpgrp,		/* 39 = setpgrp */
	1, 1, nosys,		/* 40 = tell (obsolete) */
	2, 2, dup,		/* 41 = dup */
	0, 0, pipe,		/* 42 = pipe */
	1, 0, times,		/* 43 = times */
	4, 0, profil,		/* 44 = prof */
	0, 0, nosys,		/* 45 = */
	1, 1, setgid,		/* 46 = setgid */
	0, 0, getgid,		/* 47 = getgid */
	2, 0, ssig,		/* 48 = sig */
	7, 2, msgsys,		/* 49 = IPC messages */
	0, 0, nosys,		/* 50 = reserved for USG */
	1, 0, sysacct,		/* 51 = turn acct off/on */
	3, 0, sysphys,		/* 52 = set user physical addresses */
	1, 0, syslock,		/* 53 = lock user in core */
	3, 0, ioctl,		/* 54 = ioctl */
	0, 0, nosys,		/* 55 = readwrite (in abeyance) */
	0, 0, nosys,		/* 56 = (WAS) creat mpx comm channel */
	3, 2, utssys,		/* 57 = utssys */
/*	0, 0, nosys,	*/	/* 58 = reserved for USG */
	1, 0, nosys,		/* 58 = Berkeley local syscalls */
	3, 0, exece,		/* 59 = exece */
	1, 0, umask,		/* 60 = umask */
	1, 0, chroot,		/* 61 = chroot */
	3, 1, fcntl,		/* 62 = fcntl */
	3, 0, ulimit,		/* 63 = ulimit */
/*
 * Here we end the stock V7 system calls, and
 * start the ULTRIX-11 system calls.
 */
	4, 0, evntflg,		/* 64 = eventflag syscall */
	2, 0, getfp,		/* 65 = for floating point sim. */
	2, 0, ttlocl,		/* 66 = tty local/remote mode */
	2, 0, errlog,		/* 67 = error log status & cntrl */
	1, 0, bdflush,		/* 68 = cancel delay write on buffer */
	0, 0, zaptty,		/* 69 = zap controlling tty */
	1, 0, fpsim,		/* 70 = kfpsim - turn on/off or get status */
	1, 0, nap,		/* 71 = nap system call */
#ifdef	NEWLIMITS
	1, 1, lim,		/* 72 = login limits */
#else	NEWLIMITS
	0, 0, nosys,		/* 72 = unused */
#endif	NEWLIMITS
	0, 0, nosys,		/* 73 = unused */
	0, 0, nosys,		/* 74 = unused */
	0, 0, nosys,		/* 75 = unused */
	0, 0, nosys,		/* 76 = unused */
	0, 0, nosys,		/* 77 = unused */
/* maus is 58 in sys5, semsys is 53 in sys5 */
	1, 0, maus,		/* 78 = maus */
	5, 2, semsys,		/* 79 = semsys */
/*
 * This is where the Berkeley 2.9 compatable 
 * system calls start. Indir's through #58
 * an offset to this point
 */
	0, 0, nosys,		/* 80 (0) = illegal (local) call */
	0, 0, nosys,		/* 81 (1) = login */
#ifdef	UCB_SYMLINKS
	2, 0, lstat,		/* 82 (2) = like stat, don't follow links */
#else	UCB_SYMLINKS
	0, 0, nosys,		/* 82 (2) = like stat, don't follow links */
#endif	UCB_SYMLINKS
	0, 0, nosys,		/* 83 (3) = submit - allow after logout */
	0, 0, nostk,		/* 84 (4) = nostk */
	0, 0, nosys,		/* 85 (5) = killbkg - kill background */
	0, 0, nosys,		/* 86 (6) = killpg-kill process group */
	2, 0, renice,		/* 87 (7) = renice - change a nice value */
	0, 0, nosys,		/* 88 (8) = fetchi */
	0, 0, nosys,		/* 89 (9) = ucall - call sub from user */
	0, 0, nosys,		/* 90 (10) = quota - set quota parameters */
	0, 0, nosys,		/* 91 (11) = qfstat - long fstat for quotas */
	0, 0, nosys,		/* 92 (12) = qstat - long stat for quotas */
	0, 0, setpgrp,		/* 93 (13) = setpgrp - set process group */
	0, 0, nosys,		/* 94 (14) = gldav */
	0, 0, fperr,		/* 95 (15) = fperr - get FP error regs */
	0, 0, nosys,		/* 96 (16) = vhangup - close tty files */
	0, 0, nosys,		/* 97 (17) = unused */
#ifdef	SELECT
	4, 0, select,		/* 98 (18) = select active file descr */
#else	SELECT
	0, 0, nosys,		/* 98 (18) = select active file descr */
#endif	SELECT
	2, 0, gethostname,	/* 99 (19) = get host name */
	2, 0, sethostname,	/* 100 (20) = set host name */
#ifdef	UCB_NET
	3, 0, socket,		/* 101 (21) = get socket fd */
	3, 0, bind,		/* 102 (22) = bind a socket */
	2, 0, listen,		/* 103 (23) = allow connections */
	3, 0, accept,		/* 104 (24) = passive socket connection */
	3, 0, connect,		/* 105 (25) = active socket connection */
	5, 0, socketpair,	/* 106 (26) = make pair of connected sockets */
	2, 2, setreuid,		/* 107 (27) = set real & effective user id */
	2, 2, setregid,		/* 108 (28) = set real & effective group id */
#ifdef	UCB_SYMLINKS
	2, 1, symlink,		/* 109 (29) = symlink */
	3, 1, readlink,		/* 110 (30) = readlink */
#else	UCB_SYMLINKS
	0, 0, nosys,		/* 109 (29) = symlink */
	0, 0, nosys,		/* 110 (30) = readlink */
#endif	UCB_SYMLINKS
	0, 0, gethostid,	/* 111 (31) = gethostid */
	2, 0, sethostid,	/* 112 (32) = sethostid */
	5, 0, setsockopt,	/* 113 (33) = */
	5, 0, getsockopt,	/* 114 (34) = */
	3, 0, getsockname,	/* 115 (35) = */
	3, 0, getpeername,	/* 116 (36) = */
	2, 0, shutdown,		/* 117 (37) = orderly closing of a socket */
	5, 0, sendit,		/* 118 (39) = send data over socket */
	5, 0, recvit,		/* 119 (40) = receive data from socket */
	0, 0, nosys,		/* 120 (41) = */
	0, 0, nosys,		/* 121 (42) = */
#endif	UCB_NET
/*
 * Here is where more system calls can be added if
 * need be, but you have to define UCB_NET to keep
 * the offsets correct. The maximum number of system
 * calls is 127. (bit 0200 in the trap instruction
 * is used to specify arguments on the stack)
 */
};
