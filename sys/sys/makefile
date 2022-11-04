# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
ARCHIVE = LIB1_id
# The order that the files are listed here is the order in which
# the objects get archived. They can also get archived in the
# order listed in OBJECTS, so the two should be the same.
ARCHOBJ=${ARCHIVE}(main.o)     ${ARCHIVE}(trap.o)     ${ARCHIVE}(sig.o) \
	${ARCHIVE}(iget.o)     ${ARCHIVE}(prf.o)      ${ARCHIVE}(slp.o) \
	${ARCHIVE}(subr.o)     ${ARCHIVE}(rdwri.o)    ${ARCHIVE}(clock.o) \
	${ARCHIVE}(fio.o)      ${ARCHIVE}(malloc.o)   ${ARCHIVE}(alloc.o) \
	${ARCHIVE}(machdep.o)  ${ARCHIVE}(ubmap.o)    ${ARCHIVE}(nami.o) \
	${ARCHIVE}(pipe.o)     ${ARCHIVE}(prim.o)     ${ARCHIVE}(errlog.o) \
	${ARCHIVE}(sysent.o)   ${ARCHIVE}(sys3.o)     ${ARCHIVE}(sys1.o) \
	${ARCHIVE}(sys4.o)     ${ARCHIVE}(sys2.o)     ${ARCHIVE}(acct.o) \
	${ARCHIVE}(text.o)     ${ARCHIVE}(ureg.o)     ${ARCHIVE}(ioctl.o) \
	${ARCHIVE}(syslocal.o) ${ARCHIVE}(sys_v7m.o)  ${ARCHIVE}(sys_berk.o) \
	${ARCHIVE}(fakenet.o)  ${ARCHIVE}(select.o)

OBJECTS = main.o trap.o sig.o iget.o prf.o slp.o subr.o rdwri.o \
	clock.o fio.o malloc.o alloc.o machdep.o ubmap.o nami.o \
	pipe.o prim.o errlog.o sysent.o sys3.o sys1.o sys4.o sys2.o \
	acct.o text.o ureg.o ioctl.o syslocal.o sys_v7m.o \
	sys_berk.o fakenet.o select.o

OBJ =	fpsim.o shuffle.o ipc.o sem.o msg.o maus.o flock.o \
	mbuf.o mbuf11.o subr_net.o uipc_domain.o uipc_socket.o \
	uipc_socket2.o uipc_syscall.o sys_socket.o

SRC1 =	main.c trap.c sig.c iget.c prf.c slp.c subr.c \
	rdwri.c clock.c fio.c malloc.c alloc.c machdep.c ubmap.c \
	pipe.c prim.c errlog.c sysent.c sys3.c \
	sys1.c sys4.c sys2.c acct.c text.c ureg.c ioctl.c \
	syslocal.c sys_v7m.c sys_berk.c fakenet.s select.c

SRC2 =	nami.c fpsim.s shuffle.c ipc.c sem.c msg.c maus.c flock.c \
	mbuf.c mbuf11.c subr_net.c uipc_domain.c uipc_socket.c \
	uipc_socket2.c uipc_syscall.c sys_socket.c

SOURCES = ${SRC1} ${SRC2}

CFLAGS = -O -DSEP_ID -DKERNEL
CPP = /lib/cpp -P

all: ${ARCHIVE} ${OBJ}

${ARCHIVE}: ${ARCHOBJ}

foo: ${ARCHOBJ}

archive: ${OBJECTS} ${OBJ}
	rm -f ${ARCHIVE}
	ar cr ${ARCHIVE} ${OBJECTS}
	rm -f ${OBJECTS}

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

tags: ${SOURCES}
	rm -f tags; ctags ${SOURCES}

fpsim.o:
	${CPP} -DSEP_ID -DKERNEL fpsim.s >fpsim.i
	as - -V -o fpsim.o fpsim.i
	rm -f fpsim.i

nami.o: mkuchar
	cc -S ${CFLAGS} nami.c
	mkuchar >>nami.s
	cc -c nami.s
	rm -f nami.s

mkuchar: mkuchar.c /usr/include/sys/param.h /usr/include/sys/user.h \
	/usr/include/sys/dir.h
	cc -O -o mkuchar mkuchar.c

${ARCHIVE}(nami.o): nami.o
	ar r ${ARCHIVE} nami.o

fakenet.o:
	as - -V -o fakenet.o fakenet.s

xtmp:
	-for i in ${SRC1} ; \
	do \
		echo $$i; \
		grep '^#include' $$i /dev/null | \
		sed -e 's/:[^<]*<\([^>]*\)>.*/ \/usr\/include\/\1/' \
		    -e 's/:[^"]*"\([^"]*\)".*/ \1/'; \
	done >xtmp

xtmp2:
	-for i in ${SRC2} ; \
	do \
		echo $$i; \
		grep '^#include' $$i /dev/null | \
		sed -e 's/:[^<]*<\([^>]*\)>.*/ \/usr\/include\/\1/' \
		    -e 's/:[^"]*"\([^"]*\)".*/ \1/'; \
	done >xtmp2

depend: xtmp xtmp2
	cat xtmp | \
	awk ' { if ("" == prev ) { \
			ofile = substr($$1,1,index($$1,".")) "o"; \
			rec = "$${ARCHIVE}(" ofile ") " ofile ": " $$0; \
			prev = $$1; }\
		else { if ($$1 != prev) { print rec; \
			ofile = substr(prev,1,index(prev,".")) "o"; \
			print ""; \
			print "$${ARCHIVE}(" ofile "): "; \
			print "	cc -c $${CFLAGS} " prev; \
			print "	ar r $${ARCHIVE} " ofile; \
			print "	rm -f " ofile; print ""; \
			ofile = substr($$1,1,index($$1,".")) "o"; \
			rec = "$${ARCHIVE}(" ofile ") " ofile ": " $$0; \
			prev = $$1; } \
		else {	if (length(rec $$2) > 70) \
				{ print rec " \\"; rec = "	" $$2; } \
			else rec = rec " " $$2 } } } \
		END {	print rec; \
			ofile = substr(prev,1,index(prev,".")) "o"; \
			print ""; \
			print "$${ARCHIVE}(" ofile "): "; \
			print "	cc -c $${CFLAGS} " prev; \
			print "	ar r $${ARCHIVE} " ofile; \
			print "	rm -f " ofile; \
		} ' > makedep
	cat xtmp2 | \
	awk ' { if ("" == prev ) { \
			ofile = substr($$1,1,index($$1,".")) "o"; \
			rec = ofile ": " $$0; \
			prev = $$1; }\
		else { if ($$1 != prev) { print rec; \
			ofile = substr($$1,1,index($$1,".")) "o"; \
			rec = ofile ": " $$0; \
			prev = $$1; } \
		else {	if (length(rec $$2) > 70) \
				{ print rec " \\"; rec = "	" $$2; } \
			else rec = rec " " $$2 } } } \
		END {	print rec; \
			print ""; \
		} ' >> makedep
	rm -f xtmp xtmp2
	echo '/^# DO NOT DELETE THIS LINE/+1,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp makefile makefile.bak
	ed - makefile < eddep
	rm eddep makedep

# DO NOT DELETE THIS LINE -- make depend uses it
${ARCHIVE}(main.o) main.o: main.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/filsys.h \
	/usr/include/sys/mount.h /usr/include/sys/map.h \
	/usr/include/sys/proc.h /usr/include/sys/inode.h \
	/usr/include/sys/seg.h /usr/include/sys/conf.h /usr/include/sys/buf.h \
	/usr/include/sys/devmaj.h

${ARCHIVE}(main.o): 
	cc -c ${CFLAGS} main.c
	ar r ${ARCHIVE} main.o
	rm -f main.o

${ARCHIVE}(trap.o) trap.o: trap.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/proc.h /usr/include/sys/reg.h \
	/usr/include/sys/seg.h /usr/include/sys/errlog.h \
	/usr/include/sys/inline.h

${ARCHIVE}(trap.o): 
	cc -c ${CFLAGS} trap.c
	ar r ${ARCHIVE} trap.o
	rm -f trap.o

${ARCHIVE}(sig.o) sig.o: sig.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/proc.h \
	/usr/include/sys/inode.h /usr/include/sys/reg.h \
	/usr/include/sys/text.h /usr/include/sys/seg.h

${ARCHIVE}(sig.o): 
	cc -c ${CFLAGS} sig.c
	ar r ${ARCHIVE} sig.o
	rm -f sig.o

${ARCHIVE}(iget.o) iget.o: iget.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/inode.h \
	/usr/include/sys/ino.h /usr/include/sys/filsys.h \
	/usr/include/sys/mount.h /usr/include/sys/conf.h \
	/usr/include/sys/buf.h /usr/include/sys/seg.h \
	/usr/include/sys/inline.h

${ARCHIVE}(iget.o): 
	cc -c ${CFLAGS} iget.c
	ar r ${ARCHIVE} iget.o
	rm -f iget.o

${ARCHIVE}(prf.o) prf.o: prf.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/seg.h /usr/include/sys/buf.h \
	/usr/include/sys/conf.h

${ARCHIVE}(prf.o): 
	cc -c ${CFLAGS} prf.c
	ar r ${ARCHIVE} prf.o
	rm -f prf.o

${ARCHIVE}(slp.o) slp.o: slp.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/proc.h \
	/usr/include/sys/text.h /usr/include/sys/map.h /usr/include/sys/file.h \
	/usr/include/sys/inode.h /usr/include/sys/buf.h \
	/usr/include/sys/inline.h /usr/include/sys/seg.h

${ARCHIVE}(slp.o): 
	cc -c ${CFLAGS} slp.c
	ar r ${ARCHIVE} slp.o
	rm -f slp.o

${ARCHIVE}(subr.o) subr.o: subr.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/conf.h \
	/usr/include/sys/inode.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/buf.h

${ARCHIVE}(subr.o): 
	cc -c ${CFLAGS} subr.c
	ar r ${ARCHIVE} subr.o
	rm -f subr.o

${ARCHIVE}(rdwri.o) rdwri.o: rdwri.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/inode.h \
	/usr/include/sys/file.h /usr/include/sys/dir.h /usr/include/sys/user.h \
	/usr/include/sys/buf.h /usr/include/sys/conf.h

${ARCHIVE}(rdwri.o): 
	cc -c ${CFLAGS} rdwri.c
	ar r ${ARCHIVE} rdwri.o
	rm -f rdwri.o

${ARCHIVE}(clock.o) clock.o: clock.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/callo.h \
	/usr/include/sys/seg.h /usr/include/sys/dir.h /usr/include/sys/user.h \
	/usr/include/sys/proc.h /usr/include/sys/reg.h

${ARCHIVE}(clock.o): 
	cc -c ${CFLAGS} clock.c
	ar r ${ARCHIVE} clock.o
	rm -f clock.o

${ARCHIVE}(fio.o) fio.o: fio.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/filsys.h \
	/usr/include/sys/file.h /usr/include/sys/conf.h \
	/usr/include/sys/inode.h /usr/include/sys/reg.h \
	/usr/include/sys/acct.h /usr/include/sys/seg.h

${ARCHIVE}(fio.o): 
	cc -c ${CFLAGS} fio.c
	ar r ${ARCHIVE} fio.o
	rm -f fio.o

${ARCHIVE}(malloc.o) malloc.o: malloc.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/map.h

${ARCHIVE}(malloc.o): 
	cc -c ${CFLAGS} malloc.c
	ar r ${ARCHIVE} malloc.o
	rm -f malloc.o

${ARCHIVE}(alloc.o) alloc.o: alloc.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/filsys.h \
	/usr/include/sys/mount.h /usr/include/sys/fblk.h \
	/usr/include/sys/conf.h /usr/include/sys/buf.h \
	/usr/include/sys/inode.h /usr/include/sys/ino.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/seg.h

${ARCHIVE}(alloc.o): 
	cc -c ${CFLAGS} alloc.c
	ar r ${ARCHIVE} alloc.o
	rm -f alloc.o

${ARCHIVE}(machdep.o) machdep.o: machdep.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/acct.h \
	/usr/include/sys/dir.h /usr/include/sys/user.h \
	/usr/include/sys/inode.h /usr/include/sys/proc.h \
	/usr/include/sys/seg.h /usr/include/sys/map.h /usr/include/sys/reg.h \
	/usr/include/sys/buf.h /usr/include/sys/uba.h 

${ARCHIVE}(machdep.o): 
	cc -c ${CFLAGS} machdep.c
	ar r ${ARCHIVE} machdep.o
	rm -f machdep.o

${ARCHIVE}(ubmap.o) ubmap.o: ubmap.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/seg.h /usr/include/sys/buf.h \
	/usr/include/sys/uba.h /usr/include/sys/map.h

${ARCHIVE}(ubmap.o): 
	cc -c ${CFLAGS} ubmap.c
	ar r ${ARCHIVE} ubmap.o
	rm -f ubmap.o

${ARCHIVE}(pipe.o) pipe.o: pipe.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/inode.h \
	/usr/include/sys/file.h /usr/include/sys/reg.h

${ARCHIVE}(pipe.o): 
	cc -c ${CFLAGS} pipe.c
	ar r ${ARCHIVE} pipe.o
	rm -f pipe.o

${ARCHIVE}(prim.o) prim.o: prim.c /usr/include/sys/param.h \
	/usr/include/sys/tty.h /usr/include/sys/systm.h \
	/usr/include/sys/conf.h /usr/include/sys/buf.h /usr/include/sys/seg.h

${ARCHIVE}(prim.o): 
	cc -c ${CFLAGS} prim.c
	ar r ${ARCHIVE} prim.o
	rm -f prim.o

${ARCHIVE}(errlog.o) errlog.o: errlog.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/tmscp.h \
	/usr/include/sys/errlog.h /usr/include/sys/buf.h \
	/usr/include/sys/reg.h /usr/include/sys/seg.h /usr/include/sys/dir.h \
	/usr/include/sys/proc.h /usr/include/sys/user.h /usr/include/sys/uba.h

${ARCHIVE}(errlog.o): 
	cc -c ${CFLAGS} errlog.c
	ar r ${ARCHIVE} errlog.o
	rm -f errlog.o

${ARCHIVE}(sysent.o) sysent.o: sysent.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/socket.h

${ARCHIVE}(sysent.o): 
	cc -c ${CFLAGS} sysent.c
	ar r ${ARCHIVE} sysent.o
	rm -f sysent.o

${ARCHIVE}(sys3.o) sys3.o: sys3.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/ino.h /usr/include/sys/reg.h \
	/usr/include/sys/buf.h /usr/include/sys/filsys.h \
	/usr/include/sys/mount.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/inode.h \
	/usr/include/sys/file.h /usr/include/sys/conf.h \
	/usr/include/sys/stat.h /usr/include/sys/inline.h \
	/usr/include/sys/flock.h /usr/include/sys/ioctl.h \
	/usr/include/sys/utsname.h

${ARCHIVE}(sys3.o): 
	cc -c ${CFLAGS} sys3.c
	ar r ${ARCHIVE} sys3.o
	rm -f sys3.o

${ARCHIVE}(sys1.o) sys1.o: sys1.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/map.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/proc.h /usr/include/sys/buf.h \
	/usr/include/sys/reg.h /usr/include/sys/inode.h /usr/include/sys/seg.h \
	/usr/include/sys/acct.h /usr/include/sys/tty.h

${ARCHIVE}(sys1.o): 
	cc -c ${CFLAGS} sys1.c
	ar r ${ARCHIVE} sys1.o
	rm -f sys1.o

${ARCHIVE}(sys4.o) sys4.o: sys4.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/reg.h \
	/usr/include/sys/inode.h /usr/include/sys/proc.h \
	/usr/include/sys/timeb.h /usr/include/sys/errlog.h

${ARCHIVE}(sys4.o): 
	cc -c ${CFLAGS} sys4.c
	ar r ${ARCHIVE} sys4.o
	rm -f sys4.o

${ARCHIVE}(sys2.o) sys2.o: sys2.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/reg.h /usr/include/sys/file.h \
	/usr/include/sys/inode.h /usr/include/sys/proc.h

${ARCHIVE}(sys2.o): 
	cc -c ${CFLAGS} sys2.c
	ar r ${ARCHIVE} sys2.o
	rm -f sys2.o

${ARCHIVE}(acct.o) acct.o: acct.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/acct.h \
	/usr/include/sys/dir.h /usr/include/sys/user.h \
	/usr/include/sys/inode.h /usr/include/sys/proc.h \
	/usr/include/sys/seg.h /usr/include/sys/text.h /usr/include/sys/lock.h

${ARCHIVE}(acct.o): 
	cc -c ${CFLAGS} acct.c
	ar r ${ARCHIVE} acct.o
	rm -f acct.o

${ARCHIVE}(text.o) text.o: text.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/map.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/proc.h \
	/usr/include/sys/text.h /usr/include/sys/inode.h \
	/usr/include/sys/buf.h /usr/include/sys/seg.h

${ARCHIVE}(text.o): 
	cc -c ${CFLAGS} text.c
	ar r ${ARCHIVE} text.o
	rm -f text.o

${ARCHIVE}(ureg.o) ureg.o: ureg.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/proc.h \
	/usr/include/sys/text.h /usr/include/sys/seg.h

${ARCHIVE}(ureg.o): 
	cc -c ${CFLAGS} ureg.c
	ar r ${ARCHIVE} ureg.o
	rm -f ureg.o

${ARCHIVE}(ioctl.o) ioctl.o: ioctl.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/tty.h /usr/include/sys/proc.h \
	/usr/include/sys/inode.h /usr/include/sys/file.h \
	/usr/include/sys/reg.h /usr/include/sys/conf.h /usr/include/sys/mtio.h

${ARCHIVE}(ioctl.o): 
	cc -c ${CFLAGS} ioctl.c
	ar r ${ARCHIVE} ioctl.o
	rm -f ioctl.o

${ARCHIVE}(syslocal.o) syslocal.o: syslocal.c /usr/include/sys/param.h

${ARCHIVE}(syslocal.o): 
	cc -c ${CFLAGS} syslocal.c
	ar r ${ARCHIVE} syslocal.o
	rm -f syslocal.o

${ARCHIVE}(sys_v7m.o) sys_v7m.o: sys_v7m.c /usr/include/sys/param.h \
	/usr/include/sys/dir.h /usr/include/sys/user.h /usr/include/sys/proc.h \
	/usr/include/sys/seg.h /usr/include/sys/systm.h \
	/usr/include/sys/utsname.h /usr/include/sys/eflg.h

${ARCHIVE}(sys_v7m.o): 
	cc -c ${CFLAGS} sys_v7m.c
	ar r ${ARCHIVE} sys_v7m.o
	rm -f sys_v7m.o

${ARCHIVE}(sys_berk.o) sys_berk.o: sys_berk.c /usr/include/sys/param.h \
	/usr/include/sys/proc.h /usr/include/sys/inode.h \
	/usr/include/sys/dir.h /usr/include/sys/systm.h \
	/usr/include/sys/user.h

${ARCHIVE}(sys_berk.o): 
	cc -c ${CFLAGS} sys_berk.c
	ar r ${ARCHIVE} sys_berk.o
	rm -f sys_berk.o

${ARCHIVE}(fakenet.o) fakenet.o: fakenet.s

${ARCHIVE}(fakenet.o): 
	cc -c ${CFLAGS} fakenet.s
	ar r ${ARCHIVE} fakenet.o
	rm -f fakenet.o

${ARCHIVE}(select.o) select.o: select.c /usr/include/sys/param.h \
	/usr/include/sys/dir.h /usr/include/sys/proc.h /usr/include/sys/user.h \
	/usr/include/sys/systm.h /usr/include/sys/inode.h \
	/usr/include/sys/file.h /usr/include/sys/stat.h \
	/usr/include/sys/conf.h /usr/include/sys/seg.h

${ARCHIVE}(select.o): 
	cc -c ${CFLAGS} select.c
	ar r ${ARCHIVE} select.o
	rm -f select.o
nami.o: nami.c /usr/include/sys/param.h /usr/include/sys/systm.h \
	/usr/include/sys/inode.h /usr/include/sys/filsys.h \
	/usr/include/sys/mount.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/buf.h /usr/include/sys/seg.h
fpsim.o: fpsim.s /usr/include/sys/signal.h /usr/include/sys/reg.h
shuffle.o: shuffle.c /usr/include/sys/param.h /usr/include/sys/systm.h \
	/usr/include/sys/acct.h /usr/include/sys/dir.h /usr/include/sys/user.h \
	/usr/include/sys/inode.h /usr/include/sys/proc.h \
	/usr/include/sys/text.h /usr/include/sys/seg.h /usr/include/sys/lock.h
ipc.o: ipc.c /usr/include/errno.h /usr/include/sys/types.h \
	/usr/include/sys/param.h /usr/include/sys/seg.h /usr/include/signal.h \
	/usr/include/sys/proc.h /usr/include/sys/dir.h /usr/include/sys/user.h \
	/usr/include/sys/ipc.h
sem.o: sem.c /usr/include/sys/types.h /usr/include/sys/param.h \
	/usr/include/sys/dir.h /usr/include/sys/map.h /usr/include/errno.h \
	/usr/include/signal.h /usr/include/sys/ipc.h /usr/include/sys/sem.h \
	/usr/include/sys/user.h /usr/include/sys/seg.h /usr/include/sys/proc.h \
	/usr/include/sys/buf.h
msg.o: msg.c /usr/include/sys/types.h /usr/include/sys/param.h \
	/usr/include/sys/dir.h /usr/include/signal.h /usr/include/sys/user.h \
	/usr/include/sys/seg.h /usr/include/sys/proc.h /usr/include/sys/buf.h \
	/usr/include/errno.h /usr/include/sys/map.h /usr/include/sys/ipc.h \
	/usr/include/sys/msg.h /usr/include/sys/systm.h
maus.o: maus.c /usr/include/sys/types.h /usr/include/sys/param.h \
	/usr/include/signal.h /usr/include/sys/systm.h /usr/include/sys/dir.h \
	/usr/include/errno.h /usr/include/sys/user.h /usr/include/sys/inode.h \
	/usr/include/sys/maus.h /usr/include/sys/conf.h /usr/include/sys/seg.h \
	/usr/include/sys/reg.h /usr/include/sys/map.h
flock.o: flock.c /usr/include/sys/types.h /usr/include/sys/param.h \
	/usr/include/errno.h /usr/include/sys/file.h /usr/include/sys/inode.h \
	/usr/include/sys/dir.h /usr/include/signal.h /usr/include/sys/proc.h \
	/usr/include/sys/user.h /usr/include/sys/flock.h
mbuf.o: mbuf.c /usr/include/sys/param.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/proc.h \
	/usr/include/sys/mbuf.h /usr/include/netinet/in_systm.h
mbuf11.o: mbuf11.c /usr/include/sys/param.h /usr/include/sys/seg.h \
	/usr/include/sys/mbuf.h /usr/include/netinet/in_systm.h
subr_net.o: subr_net.c /usr/include/sys/param.h /usr/include/sys/map.h \
	/usr/include/net/netisr.h /usr/include/netinet/in_systm.h \
	/usr/include/sys/mbuf.h
uipc_domain.o: uipc_domain.c /usr/include/sys/param.h \
	/usr/include/sys/socket.h /usr/include/sys/protosw.h \
	/usr/include/sys/domain.h
uipc_socket.o: uipc_socket.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/proc.h \
	/usr/include/sys/file.h /usr/include/sys/inode.h \
	/usr/include/sys/buf.h /usr/include/sys/mbuf.h \
	/usr/include/sys/protosw.h /usr/include/sys/socket.h \
	/usr/include/sys/socketvar.h /usr/include/sys/stat.h \
	/usr/include/sys/ioctl.h /usr/include/net/route.h \
	/usr/include/netinet/in.h /usr/include/netinet/in_systm.h \
	/usr/include/net/if.h /usr/include/sys/domain.h
uipc_socket2.o: uipc_socket2.c /usr/include/sys/param.h \
	/usr/include/sys/seg.h /usr/include/sys/systm.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/proc.h \
	/usr/include/sys/file.h /usr/include/sys/inode.h \
	/usr/include/sys/buf.h /usr/include/sys/mbuf.h \
	/usr/include/sys/protosw.h /usr/include/sys/socket.h \
	/usr/include/sys/socketvar.h
uipc_syscall.o: uipc_syscall.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/proc.h \
	/usr/include/sys/file.h /usr/include/sys/inode.h \
	/usr/include/sys/buf.h /usr/include/sys/mbuf.h \
	/usr/include/sys/protosw.h /usr/include/sys/socket.h \
	/usr/include/sys/socketvar.h /usr/include/netinet/in_systm.h
sys_socket.o: sys_socket.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/file.h \
	/usr/include/sys/mbuf.h /usr/include/sys/protosw.h \
	/usr/include/sys/socket.h /usr/include/sys/socketvar.h \
	/usr/include/sys/ioctl.h /usr/include/sys/stat.h /usr/include/net/if.h \
	/usr/include/netinet/in_systm.h /usr/include/net/route.h

