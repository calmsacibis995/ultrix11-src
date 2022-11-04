
SCCSID="@(#)makefile	2.4	3/4/86"

CFILES=	../sys/acct.c ../sys/alloc.c ../sys/clock.c ../sys/errlog.c \
	../sys/fio.c ../sys/flock.c ../sys/iget.c ../sys/ioctl.c \
	../sys/ipc.c ../sys/machdep.c ../sys/main.c ../sys/malloc.c \
	../sys/maus.c ../sys/msg.c ../sys/nami.c ../sys/pipe.c \
	../sys/prf.c ../sys/prim.c ../sys/rdwri.c ../sys/sem.c ../sys/sig.c \
	../sys/shuffle.c ../sys/slp.c ../sys/subr.c ../sys/sys1.c \
	../sys/sys2.c ../sys/sys3.c ../sys/sys4.c ../sys/sys_berk.c \
	../sys/sys_v7m.c ../sys/sysent.c ../sys/syslocal.c ../sys/text.c \
	../sys/trap.c ../sys/ubmap.c ../sys/ureg.c ../sys/fakenet.s \
	../sys/select.c ../sys/mbuf.c ../sys/subr_net.c \
	../sys/mbuf11.c ../sys/sys_socket.c ../sys/uipc_domain.c \
	../sys/uipc_socket.c ../sys/uipc_socket2.c ../sys/uipc_syscall.c \
	../sys/mkuchar.c

# LIBOBJ (OBJ1 & OBJ2) are the objects to go into the library.
# OTHERS (OBJ3 & OBJ4) are all the other objects.
# OBJ1 & OBJ3 are objects that do not have explicit entries in the makefile
# OBJ2 & OBJ4 are objects that do have entries in the makefile.
#
#	The library (LIB1_ov) is no longer used.  Thus make all
#	does not create the library.
#
OBJ1 =	trap.o iget.o slp.o rdwri.o clock.o \
	alloc.o sysent.o prim.o
OBJ2 =	prf.o
LIBOBJ = ${OBJ1} ${OBJ2}
OBJ3 =	acct.o errlog.o fio.o flock.o ioctl.o machdep.o main.o malloc.o maus.o \
	ipc.o msg.o pipe.o sem.o shuffle.o \
	sig.o subr.o sys1.o sys2.o sys3.o sys4.o \
	sys_berk.o sys_v7m.o syslocal.o text.o ubmap.o ureg.o \
	mbuf.o mbuf11.o sys_socket.o uipc_domain.o uipc_socket.o \
	uipc_socket2.o uipc_syscall.o subr_net.o
OBJ4 =	nami.o fpsim.o select.o fakenet.o
OTHERS = ${OBJ3} ${OBJ4}

CFLAGS = -O -N -DK_OV -DKERNEL
CPP = /lib/cpp -P

all: ${OTHERS} ${LIBOBJ}

LIB1_ov: ${LIBOBJ}
	ar r LIB1_ov ${LIBOBJ}

${OBJ1} ${OBJ3}:
	cc -E ${CFLAGS} ../sys/`basename $@ .o`.c | xstr -
	cc -S xs.c
	ed - xs.s < asmfix1
	cc -S ${CFLAGS} x.c
	ed - x.s < asmfix2
	as - -V -o $@ x.s xs.s
	rm -f x.c x.s xs.c xs.s
	@echo

fpsim.o: ../sys/fpsim.s
	${CPP} -DKERNEL -DK_OV ../sys/fpsim.s >fpsim.i
	as - -V -o fpsim.o fpsim.i
	rm -f fpsim.i

nami.o: ../sys/nami.c mkuchar
	cc -S ${CFLAGS} ../sys/nami.c
	mkuchar >>nami.s
	cc -c nami.s
	rm -f nami.s

mkuchar.o:
	cc -c -O ../sys/mkuchar.c
mkuchar: mkuchar.o
	cc -O -o mkuchar mkuchar.o

fakenet.o: ../sys/fakenet.s
	as - -V -o fakenet.o ../sys/fakenet.s

prf.o: ../sys/prf.c
	cc -c ${CFLAGS} ../sys/prf.c

select.o:
	cc -c ${CFLAGS} ../sys/select.c

depend:
	(grep '^#include' ${CFILES} | grep -v devmaj | \
	sed -e 's/:[^<]*<\([^>]*\)>.*/: \/usr\/include\/\1/' \
	    -e 's/:[^"]*"\([^"]*\)".*/: \1/' \
	    -e 's/\.c/.o/' \
	    -e 's,../[a-zA-Z]*/,,'; \
	for i in ${CFILES}; do echo `basename $$i .c`.o: $$i; done) | \
	sort | \
	awk ' { if ($$1 != prev) { print rec; rec = $$0; prev = $$1; } \
		else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
		       else rec = rec " " $$2 } } \
	      END { print rec } ' > makedep
	echo '/^# DO NOT DELETE THIS LINE/+1,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp makefile makefile.bak
	ed - makefile < eddep
	rm eddep makedep

# DO NOT DELETE THIS LINE -- make depend uses it

acct.o: ../sys/acct.c /usr/include/sys/acct.h /usr/include/sys/dir.h
acct.o: /usr/include/sys/inode.h /usr/include/sys/lock.h
acct.o: /usr/include/sys/param.h /usr/include/sys/proc.h /usr/include/sys/seg.h
acct.o: /usr/include/sys/systm.h /usr/include/sys/text.h
acct.o: /usr/include/sys/user.h
alloc.o: ../sys/alloc.c /usr/include/sys/buf.h /usr/include/sys/conf.h
alloc.o: /usr/include/sys/dir.h /usr/include/sys/fblk.h
alloc.o: /usr/include/sys/filsys.h /usr/include/sys/ino.h
alloc.o: /usr/include/sys/inode.h /usr/include/sys/mount.h
alloc.o: /usr/include/sys/param.h /usr/include/sys/seg.h
alloc.o: /usr/include/sys/systm.h /usr/include/sys/user.h
clock.o: ../sys/clock.c /usr/include/sys/callo.h /usr/include/sys/dir.h
clock.o: /usr/include/sys/param.h /usr/include/sys/proc.h
clock.o: /usr/include/sys/reg.h /usr/include/sys/seg.h /usr/include/sys/systm.h
clock.o: /usr/include/sys/user.h
errlog.o: ../sys/errlog.c /usr/include/sys/buf.h /usr/include/sys/dir.h
errlog.o: /usr/include/sys/errlog.h /usr/include/sys/param.h
errlog.o: /usr/include/sys/proc.h /usr/include/sys/reg.h /usr/include/sys/seg.h
errlog.o: /usr/include/sys/systm.h /usr/include/sys/tmscp.h
errlog.o: /usr/include/sys/uba.h /usr/include/sys/user.h
fakenet.s.o: ../sys/fakenet.s
fio.o: ../sys/fio.c /usr/include/sys/acct.h /usr/include/sys/conf.h
fio.o: /usr/include/sys/dir.h /usr/include/sys/file.h /usr/include/sys/filsys.h
fio.o: /usr/include/sys/inode.h /usr/include/sys/param.h /usr/include/sys/reg.h
fio.o: /usr/include/sys/seg.h /usr/include/sys/systm.h /usr/include/sys/user.h
flock.o: ../sys/flock.c /usr/include/errno.h /usr/include/signal.h
flock.o: /usr/include/sys/dir.h /usr/include/sys/file.h
flock.o: /usr/include/sys/flock.h /usr/include/sys/inode.h
flock.o: /usr/include/sys/param.h /usr/include/sys/proc.h
flock.o: /usr/include/sys/types.h /usr/include/sys/user.h
iget.o: ../sys/iget.c /usr/include/sys/buf.h /usr/include/sys/conf.h
iget.o: /usr/include/sys/dir.h /usr/include/sys/filsys.h
iget.o: /usr/include/sys/inline.h /usr/include/sys/ino.h
iget.o: /usr/include/sys/inode.h /usr/include/sys/mount.h
iget.o: /usr/include/sys/param.h /usr/include/sys/seg.h
iget.o: /usr/include/sys/systm.h /usr/include/sys/user.h
ioctl.o: ../sys/ioctl.c /usr/include/sys/conf.h /usr/include/sys/dir.h
ioctl.o: /usr/include/sys/file.h /usr/include/sys/inode.h
ioctl.o: /usr/include/sys/param.h /usr/include/sys/proc.h
ioctl.o: /usr/include/sys/reg.h /usr/include/sys/systm.h /usr/include/sys/tty.h
ioctl.o: /usr/include/sys/user.h
ipc.o: ../sys/ipc.c /usr/include/errno.h /usr/include/signal.h
ipc.o: /usr/include/sys/dir.h /usr/include/sys/ipc.h /usr/include/sys/param.h
ipc.o: /usr/include/sys/proc.h /usr/include/sys/seg.h /usr/include/sys/types.h
ipc.o: /usr/include/sys/user.h
machdep.o: ../sys/machdep.c /usr/include/sys/acct.h /usr/include/sys/buf.h
machdep.o: /usr/include/sys/dir.h /usr/include/sys/inode.h
machdep.o: /usr/include/sys/map.h /usr/include/sys/param.h
machdep.o: /usr/include/sys/proc.h /usr/include/sys/reg.h
machdep.o: /usr/include/sys/seg.h /usr/include/sys/systm.h
machdep.o: /usr/include/sys/uba.h /usr/include/sys/user.h
main.o: ../sys/main.c /usr/include/sys/buf.h /usr/include/sys/conf.h
main.o: /usr/include/sys/dir.h /usr/include/sys/filsys.h
main.o: /usr/include/sys/inode.h /usr/include/sys/map.h
main.o: /usr/include/sys/mount.h /usr/include/sys/param.h
main.o: /usr/include/sys/proc.h /usr/include/sys/seg.h /usr/include/sys/systm.h
main.o: /usr/include/sys/user.h
malloc.o: ../sys/malloc.c /usr/include/sys/map.h /usr/include/sys/param.h
malloc.o: /usr/include/sys/systm.h
maus.o: ../sys/maus.c /usr/include/errno.h /usr/include/signal.h
maus.o: /usr/include/sys/conf.h /usr/include/sys/dir.h /usr/include/sys/inode.h
maus.o: /usr/include/sys/map.h /usr/include/sys/maus.h /usr/include/sys/param.h
maus.o: /usr/include/sys/reg.h /usr/include/sys/seg.h /usr/include/sys/systm.h
maus.o: /usr/include/sys/types.h /usr/include/sys/user.h
mbuf.o: ../sys/mbuf.c /usr/include/netinet/in_systm.h /usr/include/sys/dir.h
mbuf.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
mbuf.o: /usr/include/sys/proc.h /usr/include/sys/user.h
mbuf11.o: ../sys/mbuf11.c /usr/include/netinet/in_systm.h
mbuf11.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
mbuf11.o: /usr/include/sys/seg.h
msg.o: ../sys/msg.c /usr/include/errno.h /usr/include/signal.h
msg.o: /usr/include/sys/buf.h /usr/include/sys/dir.h /usr/include/sys/ipc.h
msg.o: /usr/include/sys/map.h /usr/include/sys/msg.h /usr/include/sys/param.h
msg.o: /usr/include/sys/proc.h /usr/include/sys/seg.h /usr/include/sys/systm.h
msg.o: /usr/include/sys/types.h /usr/include/sys/user.h
nami.o: ../sys/nami.c /usr/include/sys/buf.h /usr/include/sys/dir.h
nami.o: /usr/include/sys/filsys.h /usr/include/sys/inode.h
nami.o: /usr/include/sys/mount.h /usr/include/sys/param.h
nami.o: /usr/include/sys/systm.h /usr/include/sys/user.h
pipe.o: ../sys/pipe.c /usr/include/sys/dir.h /usr/include/sys/file.h
pipe.o: /usr/include/sys/inode.h /usr/include/sys/param.h
pipe.o: /usr/include/sys/reg.h /usr/include/sys/systm.h /usr/include/sys/user.h
prf.o: ../sys/prf.c /usr/include/sys/buf.h /usr/include/sys/conf.h
prf.o: /usr/include/sys/param.h /usr/include/sys/seg.h /usr/include/sys/systm.h
prim.o: ../sys/prim.c /usr/include/sys/buf.h /usr/include/sys/conf.h
prim.o: /usr/include/sys/param.h /usr/include/sys/seg.h
prim.o: /usr/include/sys/systm.h /usr/include/sys/tty.h
rdwri.o: ../sys/rdwri.c /usr/include/sys/buf.h /usr/include/sys/conf.h
rdwri.o: /usr/include/sys/dir.h /usr/include/sys/file.h
rdwri.o: /usr/include/sys/inode.h /usr/include/sys/param.h
rdwri.o: /usr/include/sys/systm.h /usr/include/sys/user.h
select.o: ../sys/select.c /usr/include/sys/conf.h /usr/include/sys/dir.h
select.o: /usr/include/sys/file.h /usr/include/sys/inode.h
select.o: /usr/include/sys/param.h /usr/include/sys/proc.h
select.o: /usr/include/sys/seg.h /usr/include/sys/stat.h
select.o: /usr/include/sys/systm.h /usr/include/sys/user.h
sem.o: ../sys/sem.c /usr/include/errno.h /usr/include/signal.h
sem.o: /usr/include/sys/buf.h /usr/include/sys/dir.h /usr/include/sys/ipc.h
sem.o: /usr/include/sys/map.h /usr/include/sys/param.h /usr/include/sys/proc.h
sem.o: /usr/include/sys/seg.h /usr/include/sys/sem.h /usr/include/sys/types.h
sem.o: /usr/include/sys/user.h
shuffle.o: ../sys/shuffle.c /usr/include/sys/acct.h /usr/include/sys/dir.h
shuffle.o: /usr/include/sys/inode.h /usr/include/sys/lock.h
shuffle.o: /usr/include/sys/param.h /usr/include/sys/proc.h
shuffle.o: /usr/include/sys/seg.h /usr/include/sys/systm.h
shuffle.o: /usr/include/sys/text.h /usr/include/sys/user.h
sig.o: ../sys/sig.c /usr/include/sys/dir.h /usr/include/sys/inode.h
sig.o: /usr/include/sys/param.h /usr/include/sys/proc.h /usr/include/sys/reg.h
sig.o: /usr/include/sys/seg.h /usr/include/sys/systm.h /usr/include/sys/text.h
sig.o: /usr/include/sys/user.h
slp.o: ../sys/slp.c /usr/include/sys/buf.h /usr/include/sys/dir.h
slp.o: /usr/include/sys/file.h /usr/include/sys/inline.h
slp.o: /usr/include/sys/inode.h /usr/include/sys/map.h /usr/include/sys/param.h
slp.o: /usr/include/sys/proc.h /usr/include/sys/seg.h /usr/include/sys/systm.h
slp.o: /usr/include/sys/text.h /usr/include/sys/user.h
subr.o: ../sys/subr.c /usr/include/sys/buf.h /usr/include/sys/conf.h
subr.o: /usr/include/sys/dir.h /usr/include/sys/inode.h
subr.o: /usr/include/sys/param.h /usr/include/sys/systm.h
subr.o: /usr/include/sys/user.h
subr_net.o: ../sys/subr_net.c /usr/include/net/netisr.h
subr_net.o: /usr/include/netinet/in_systm.h /usr/include/sys/map.h
subr_net.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
sys1.o: ../sys/sys1.c /usr/include/sys/acct.h /usr/include/sys/buf.h
sys1.o: /usr/include/sys/dir.h /usr/include/sys/inode.h /usr/include/sys/map.h
sys1.o: /usr/include/sys/param.h /usr/include/sys/proc.h /usr/include/sys/reg.h
sys1.o: /usr/include/sys/seg.h /usr/include/sys/systm.h /usr/include/sys/tty.h
sys1.o: /usr/include/sys/user.h
sys2.o: ../sys/sys2.c /usr/include/sys/dir.h /usr/include/sys/file.h
sys2.o: /usr/include/sys/inode.h /usr/include/sys/param.h
sys2.o: /usr/include/sys/proc.h /usr/include/sys/reg.h /usr/include/sys/systm.h
sys2.o: /usr/include/sys/user.h
sys3.o: ../sys/sys3.c /usr/include/sys/buf.h /usr/include/sys/conf.h
sys3.o: /usr/include/sys/dir.h /usr/include/sys/file.h
sys3.o: /usr/include/sys/filsys.h /usr/include/sys/flock.h
sys3.o: /usr/include/sys/inline.h /usr/include/sys/ino.h
sys3.o: /usr/include/sys/inode.h /usr/include/sys/ioctl.h
sys3.o: /usr/include/sys/mount.h /usr/include/sys/param.h
sys3.o: /usr/include/sys/reg.h /usr/include/sys/stat.h /usr/include/sys/systm.h
sys3.o: /usr/include/sys/user.h /usr/include/sys/utsname.h
sys4.o: ../sys/sys4.c /usr/include/sys/dir.h /usr/include/sys/errlog.h
sys4.o: /usr/include/sys/inode.h /usr/include/sys/param.h
sys4.o: /usr/include/sys/proc.h /usr/include/sys/reg.h /usr/include/sys/systm.h
sys4.o: /usr/include/sys/timeb.h /usr/include/sys/user.h
sys_berk.o: ../sys/sys_berk.c /usr/include/sys/dir.h /usr/include/sys/inode.h
sys_berk.o: /usr/include/sys/param.h /usr/include/sys/proc.h
sys_berk.o: /usr/include/sys/systm.h /usr/include/sys/user.h
sys_socket.o: ../sys/sys_socket.c /usr/include/net/if.h
sys_socket.o: /usr/include/net/route.h /usr/include/netinet/in_systm.h
sys_socket.o: /usr/include/sys/dir.h /usr/include/sys/file.h
sys_socket.o: /usr/include/sys/ioctl.h /usr/include/sys/mbuf.h
sys_socket.o: /usr/include/sys/param.h /usr/include/sys/protosw.h
sys_socket.o: /usr/include/sys/socket.h /usr/include/sys/socketvar.h
sys_socket.o: /usr/include/sys/stat.h /usr/include/sys/systm.h
sys_socket.o: /usr/include/sys/user.h
sys_v7m.o: ../sys/sys_v7m.c /usr/include/sys/dir.h /usr/include/sys/eflg.h
sys_v7m.o: /usr/include/sys/param.h /usr/include/sys/proc.h
sys_v7m.o: /usr/include/sys/seg.h /usr/include/sys/systm.h
sys_v7m.o: /usr/include/sys/user.h
sysent.o: ../sys/sysent.c /usr/include/sys/param.h /usr/include/sys/socket.h
sysent.o: /usr/include/sys/systm.h
syslocal.o: ../sys/syslocal.c /usr/include/sys/param.h
text.o: ../sys/text.c /usr/include/sys/buf.h /usr/include/sys/dir.h
text.o: /usr/include/sys/inode.h /usr/include/sys/map.h
text.o: /usr/include/sys/param.h /usr/include/sys/proc.h /usr/include/sys/seg.h
text.o: /usr/include/sys/systm.h /usr/include/sys/text.h
text.o: /usr/include/sys/user.h
trap.o: ../sys/trap.c /usr/include/sys/dir.h /usr/include/sys/errlog.h
trap.o: /usr/include/sys/inline.h /usr/include/sys/param.h
trap.o: /usr/include/sys/proc.h /usr/include/sys/reg.h /usr/include/sys/seg.h
trap.o: /usr/include/sys/systm.h /usr/include/sys/user.h
ubmap.o: ../sys/ubmap.c /usr/include/sys/buf.h /usr/include/sys/map.h
ubmap.o: /usr/include/sys/param.h /usr/include/sys/seg.h
ubmap.o: /usr/include/sys/systm.h /usr/include/sys/uba.h
uipc_domain.o: ../sys/uipc_domain.c /usr/include/sys/domain.h
uipc_domain.o: /usr/include/sys/param.h /usr/include/sys/protosw.h
uipc_domain.o: /usr/include/sys/socket.h
uipc_socket.o: ../sys/uipc_socket.c /usr/include/net/if.h
uipc_socket.o: /usr/include/net/route.h /usr/include/netinet/in.h
uipc_socket.o: /usr/include/netinet/in_systm.h /usr/include/sys/buf.h
uipc_socket.o: /usr/include/sys/dir.h /usr/include/sys/file.h
uipc_socket.o: /usr/include/sys/inode.h /usr/include/sys/ioctl.h
uipc_socket.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
uipc_socket.o: /usr/include/sys/proc.h /usr/include/sys/protosw.h
uipc_socket.o: /usr/include/sys/socket.h /usr/include/sys/socketvar.h
uipc_socket.o: /usr/include/sys/stat.h /usr/include/sys/systm.h
uipc_socket.o: /usr/include/sys/user.h
uipc_socket2.o: ../sys/uipc_socket2.c /usr/include/sys/buf.h
uipc_socket2.o: /usr/include/sys/dir.h /usr/include/sys/file.h
uipc_socket2.o: /usr/include/sys/inode.h /usr/include/sys/mbuf.h
uipc_socket2.o: /usr/include/sys/param.h /usr/include/sys/proc.h
uipc_socket2.o: /usr/include/sys/protosw.h /usr/include/sys/seg.h
uipc_socket2.o: /usr/include/sys/socket.h /usr/include/sys/socketvar.h
uipc_socket2.o: /usr/include/sys/systm.h /usr/include/sys/user.h
uipc_syscall.o: ../sys/uipc_syscall.c /usr/include/netinet/in_systm.h
uipc_syscall.o: /usr/include/sys/buf.h /usr/include/sys/dir.h
uipc_syscall.o: /usr/include/sys/file.h /usr/include/sys/inode.h
uipc_syscall.o: /usr/include/sys/mbuf.h /usr/include/sys/param.h
uipc_syscall.o: /usr/include/sys/proc.h /usr/include/sys/protosw.h
uipc_syscall.o: /usr/include/sys/socket.h /usr/include/sys/socketvar.h
uipc_syscall.o: /usr/include/sys/systm.h /usr/include/sys/user.h
ureg.o: ../sys/ureg.c /usr/include/sys/dir.h /usr/include/sys/param.h
ureg.o: /usr/include/sys/proc.h /usr/include/sys/seg.h /usr/include/sys/systm.h
ureg.o: /usr/include/sys/text.h /usr/include/sys/user.h
