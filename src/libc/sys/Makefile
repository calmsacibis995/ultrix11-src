# SCCSID: @(#)Makefile	3.0	(ULTRIX-11)	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# This is just a dummy makefile. The real one, "makefile", is created
# automatically from the make in '..'.

SOURCES = access.s acct.s alarm.s bdflush.s chdir.s chmod.s \
	chown.s chroot.s close.s creat.s dup.s errlog.s evntflg.s execl.s \
	execle.s execv.s execve.s exit.s fork.s fperr.s fpsim.s \
	fstat.s getgid.s getpid.s getppid.s getuid.s ghostid.s \
	ghostname.s ioctl.s kill.s link.s lock.s lseek.s mknod.s \
	mount.s mpxcall.s nice.s open.s pause.s phys.s pipe.s \
	profil.s ptrace.s read.s sbrk.s setgid.s setuid.s shostid.s \
	shostname.s signal.s stat.s stime.s sync.s time.s times.s \
	ttlocl.s umask.s umount.s unlink.s utime.s wait.s write.s zaptty.s

sources: $(SOURCES)

.DEFAULT:
	sccs get $<
