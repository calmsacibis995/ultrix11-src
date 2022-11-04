# SCCSID: @(#)makefile	3.0	(ULTRIX-11)	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

SOURCES = access.s acct.s alarm.s bdflush.s chdir.s chroot.s \
	chmod.s chown.s close.s creat.s dup.s errlog.s \
	evntflg.s execl.s execle.s execv.s execve.s exit.s \
	fcntl.s fork.s fperr.s fpsim.s fstat.s getgid.s getpid.s \
	getppid.s getuid.s ghostid.s ghostname.s ioctl.s kill.s \
	link.s lock.s lstat.s lseek.s maus.s mknod.s mount.s \
	msgsys.c nap.s nice.s nostk.s open.s pause.s phys.s pipe.s \
	profil.s ptrace.s read.s readlink.s renice.s sbrk.s semsys.c \
	setgid.s setuid.s sgpgrp.c shostid.s shostname.s signal.s \
	spgrp.s stat.s syscall.s stime.s symlink.s sync.s time.s times.s \
	ttlocl.s ulimit.s umask.s umount.s uname.s unlink.s ustat.s utime.s \
	wait.s write.s zaptty.s \
	accept.s bind.s connect.s getpeername.s getsocketopt.s \
	getsockname.s listen.s recvit.s select.s sendit.s setregid.s \
	setreuid.s setsocketopt.s shutdown.s socket.s socketpair.s

OBJECTS = access.o acct.o alarm.o bdflush.o chdir.o chroot.o\
	chmod.o chown.o close.o creat.o dup.o errlog.o\
	evntflg.o execl.o execle.o execv.o execve.o exit.o\
	fcntl.o fork.o fperr.o fpsim.o fstat.o getgid.o getpid.o\
	getppid.o getuid.o ghostid.o ghostname.o ioctl.o kill.o link.o\
	lock.o lstat.o lseek.o maus.o mknod.o mount.o \
	msgsys.o nap.o nice.o nostk.o open.o pause.o phys.o pipe.o \
	profil.o ptrace.o read.o readlink.o renice.o sbrk.o semsys.o \
	setgid.o setuid.o sgpgrp.o shostid.o shostname.o signal.o spgrp.o \
	stat.o stime.o syscall.o symlink.o sync.o time.o times.o \
	ttlocl.o ulimit.o umask.o umount.o uname.o unlink.o ustat.o utime.o \
	wait.o write.o zaptty.o \
	accept.o bind.o connect.o getpeername.o getsocketopt.o \
	getsockname.o listen.o recvit.o select.o sendit.o setregid.o \
	setreuid.o setsocketopt.o shutdown.o socket.o socketpair.o

CFLAGS = -O

all: ${OBJECTS}

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

archive: ${OBJECTS}
	ar r ../${LIB} ${OBJECTS}

clean:
	rm -f *.o

clobber:
	rm -f *.o; sccs clean

tags:	forceit
	@(echo "not making tags for assembler files")

sccsinfo:
	sccs info

forceit:
