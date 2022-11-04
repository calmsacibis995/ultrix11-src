SCCSID="@(#)makefile	2.4	3/4/86"
# objects that it is ok to extract strings from and place into text space
OBJXSTR = bio.o ct.o dh.o dhdm.o dhfdm.o dn.o dsort.o \
	du.o dz.o hk.o hp.o ht.o hx.o kl.o lp.o mem.o partab.o \
	pty.o ra.o rk.o rl.o rp.o sys.o tk.o tm.o ts.o tty.o \
	u1.o u2.o u3.o u4.o uh.o

# objects that want their strings to stay in data space
OBJNXSTR = ttynew.o

OBJECTS = ${OBJXSTR} ${OBJNXSTR}

CFILES	= ../dev/bio.c ../dev/ct.c ../dev/dh.c ../dev/dhdm.c \
	../dev/dhfdm.c ../dev/dn.c ../dev/dsort.c \
	../dev/du.c ../dev/dz.c ../dev/hk.c ../dev/hp.c \
	../dev/ht.c ../dev/hx.c ../dev/kl.c \
	../dev/lp.c ../dev/mem.c ../dev/partab.c ../dev/pty.c \
	../dev/ra.c ../dev/rk.c ../dev/rl.c ../dev/rp.c \
	../dev/sys.c ../dev/tk.c ../dev/tm.c ../dev/ts.c \
	../dev/tty.c ../dev/ttynew.c ../dev/u1.c ../dev/u2.c \
	../dev/u3.c ../dev/u4.c ../dev/uh.c

CFLAGS = -O -N -DK_OV -DKERNEL

all: ${OBJECTS}

${OBJXSTR}:
	cc -E ${CFLAGS} ../dev/`basename $@ .o`.c | xstr -
	cc -S xs.c
	ed - xs.s < asmfix1
	cc -S ${CFLAGS} x.c
	ed - x.s < asmfix2
	as - -V -o $@ x.s xs.s
	rm -f x.c x.s xs.c xs.s
	@echo

${OBJNXSTR}:
	cc -c ${CFLAGS} ../dev/`basename $@ .o`.c

depend:
	(grep '^#include' ${CFILES} | grep -v devmaj | \
	sed -e 's/:[^<]*<\([^>]*\)>.*/: \/usr\/include\/\1/' \
	    -e 's/\.c/.o/' \
	    -e 's,../[a-zA-Z]*/,,' ; \
	for i in ${CFILES}; do echo `basename $$i .c`.o: $$i; done ) | \
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

bio.o: ../dev/bio.c /usr/include/sys/buf.h /usr/include/sys/conf.h
bio.o: /usr/include/sys/dir.h /usr/include/sys/param.h /usr/include/sys/proc.h
bio.o: /usr/include/sys/seg.h /usr/include/sys/systm.h /usr/include/sys/user.h
ct.o: ../dev/ct.c /usr/include/sys/dir.h /usr/include/sys/param.h
ct.o: /usr/include/sys/tty.h /usr/include/sys/user.h
dh.o: ../dev/dh.c /usr/include/sys/conf.h /usr/include/sys/dir.h
dh.o: /usr/include/sys/param.h /usr/include/sys/systm.h /usr/include/sys/tty.h
dh.o: /usr/include/sys/uba.h /usr/include/sys/user.h
dhdm.o: ../dev/dhdm.c /usr/include/sys/conf.h /usr/include/sys/file.h
dhdm.o: /usr/include/sys/param.h /usr/include/sys/tty.h
dhfdm.o: ../dev/dhfdm.c /usr/include/sys/conf.h /usr/include/sys/param.h
dhfdm.o: /usr/include/sys/tty.h
dn.o: ../dev/dn.c /usr/include/sys/dir.h /usr/include/sys/param.h
dn.o: /usr/include/sys/user.h
dsort.o: ../dev/dsort.c /usr/include/sys/bads.h /usr/include/sys/buf.h
dsort.o: /usr/include/sys/param.h /usr/include/sys/seg.h
dsort.o: /usr/include/sys/systm.h
du.o: ../dev/du.c /usr/include/sys/buf.h /usr/include/sys/dir.h
du.o: /usr/include/sys/param.h /usr/include/sys/systm.h /usr/include/sys/user.h
dz.o: ../dev/dz.c /usr/include/sys/conf.h /usr/include/sys/dir.h
dz.o: /usr/include/sys/file.h /usr/include/sys/param.h /usr/include/sys/tty.h
dz.o: /usr/include/sys/user.h
hk.o: ../dev/hk.c /usr/include/sys/bads.h /usr/include/sys/buf.h
hk.o: /usr/include/sys/conf.h /usr/include/sys/dir.h /usr/include/sys/errlog.h
hk.o: /usr/include/sys/hkbad.h /usr/include/sys/param.h /usr/include/sys/seg.h
hk.o: /usr/include/sys/systm.h /usr/include/sys/uba.h /usr/include/sys/user.h
hp.o: ../dev/hp.c /usr/include/sys/bads.h /usr/include/sys/buf.h
hp.o: /usr/include/sys/conf.h /usr/include/sys/dir.h /usr/include/sys/errlog.h
hp.o: /usr/include/sys/hp_info.h /usr/include/sys/hpbad.h
hp.o: /usr/include/sys/param.h /usr/include/sys/seg.h /usr/include/sys/systm.h
hp.o: /usr/include/sys/uba.h /usr/include/sys/user.h
ht.o: ../dev/ht.c /usr/include/sys/buf.h /usr/include/sys/conf.h
ht.o: /usr/include/sys/dir.h /usr/include/sys/errlog.h /usr/include/sys/file.h
ht.o: /usr/include/sys/param.h /usr/include/sys/systm.h /usr/include/sys/user.h
hx.o: ../dev/hx.c /usr/include/sgtty.h /usr/include/sys/buf.h
hx.o: /usr/include/sys/conf.h /usr/include/sys/dir.h /usr/include/sys/errlog.h
hx.o: /usr/include/sys/param.h /usr/include/sys/systm.h /usr/include/sys/user.h
kl.o: ../dev/kl.c /usr/include/sys/conf.h /usr/include/sys/dir.h
kl.o: /usr/include/sys/param.h /usr/include/sys/systm.h /usr/include/sys/tty.h
kl.o: /usr/include/sys/user.h
lp.o: ../dev/lp.c /usr/include/sys/dir.h /usr/include/sys/param.h
lp.o: /usr/include/sys/tty.h /usr/include/sys/user.h
mem.o: ../dev/mem.c /usr/include/sys/conf.h /usr/include/sys/dir.h
mem.o: /usr/include/sys/maus.h /usr/include/sys/param.h /usr/include/sys/seg.h
mem.o: /usr/include/sys/user.h
partab.o: ../dev/partab.c
pty.o: ../dev/pty.c /usr/include/sys/buf.h /usr/include/sys/conf.h
pty.o: /usr/include/sys/dir.h /usr/include/sys/file.h /usr/include/sys/param.h
pty.o: /usr/include/sys/proc.h /usr/include/sys/systm.h /usr/include/sys/tty.h
pty.o: /usr/include/sys/user.h
ra.o: ../dev/ra.c /usr/include/sys/buf.h /usr/include/sys/conf.h
ra.o: /usr/include/sys/dir.h /usr/include/sys/errlog.h /usr/include/sys/param.h
ra.o: /usr/include/sys/ra_info.h /usr/include/sys/seg.h
ra.o: /usr/include/sys/systm.h /usr/include/sys/user.h
rk.o: ../dev/rk.c /usr/include/sys/buf.h /usr/include/sys/conf.h
rk.o: /usr/include/sys/dir.h /usr/include/sys/errlog.h /usr/include/sys/param.h
rk.o: /usr/include/sys/systm.h /usr/include/sys/user.h
rl.o: ../dev/rl.c /usr/include/sys/buf.h /usr/include/sys/dir.h
rl.o: /usr/include/sys/errlog.h /usr/include/sys/param.h
rl.o: /usr/include/sys/systm.h /usr/include/sys/user.h
rp.o: ../dev/rp.c /usr/include/sys/buf.h /usr/include/sys/conf.h
rp.o: /usr/include/sys/dir.h /usr/include/sys/errlog.h /usr/include/sys/param.h
rp.o: /usr/include/sys/systm.h /usr/include/sys/user.h
sys.o: ../dev/sys.c /usr/include/sys/conf.h /usr/include/sys/dir.h
sys.o: /usr/include/sys/param.h /usr/include/sys/proc.h /usr/include/sys/tty.h
sys.o: /usr/include/sys/user.h
tk.o: ../dev/tk.c /usr/include/sys/buf.h /usr/include/sys/conf.h
tk.o: /usr/include/sys/dir.h /usr/include/sys/errlog.h /usr/include/sys/file.h
tk.o: /usr/include/sys/param.h /usr/include/sys/seg.h /usr/include/sys/systm.h
tk.o: /usr/include/sys/tk_info.h /usr/include/sys/user.h
tm.o: ../dev/tm.c /usr/include/sys/buf.h /usr/include/sys/conf.h
tm.o: /usr/include/sys/dir.h /usr/include/sys/errlog.h /usr/include/sys/file.h
tm.o: /usr/include/sys/param.h /usr/include/sys/systm.h /usr/include/sys/user.h
ts.o: ../dev/ts.c /usr/include/sys/buf.h /usr/include/sys/conf.h
ts.o: /usr/include/sys/dir.h /usr/include/sys/errlog.h /usr/include/sys/file.h
ts.o: /usr/include/sys/param.h /usr/include/sys/systm.h /usr/include/sys/user.h
ts.o: /usr/include/sys/ts_info.h
tty.o: ../dev/tty.c /usr/include/sys/conf.h /usr/include/sys/dir.h
tty.o: /usr/include/sys/file.h /usr/include/sys/inode.h
tty.o: /usr/include/sys/param.h /usr/include/sys/proc.h /usr/include/sys/reg.h
tty.o: /usr/include/sys/systm.h /usr/include/sys/tty.h /usr/include/sys/user.h
ttynew.o: ../dev/ttynew.c /usr/include/sys/buf.h /usr/include/sys/conf.h
ttynew.o: /usr/include/sys/dir.h /usr/include/sys/file.h
ttynew.o: /usr/include/sys/inode.h /usr/include/sys/param.h
ttynew.o: /usr/include/sys/proc.h /usr/include/sys/reg.h
ttynew.o: /usr/include/sys/systm.h /usr/include/sys/tty.h
ttynew.o: /usr/include/sys/user.h
u1.o: ../dev/u1.c /usr/include/sys/buf.h /usr/include/sys/dir.h
u1.o: /usr/include/sys/param.h /usr/include/sys/systm.h /usr/include/sys/tty.h
u1.o: /usr/include/sys/user.h
u2.o: ../dev/u2.c /usr/include/sys/buf.h /usr/include/sys/dir.h
u2.o: /usr/include/sys/param.h /usr/include/sys/systm.h /usr/include/sys/tty.h
u2.o: /usr/include/sys/user.h
u3.o: ../dev/u3.c /usr/include/sys/buf.h /usr/include/sys/dir.h
u3.o: /usr/include/sys/param.h /usr/include/sys/systm.h /usr/include/sys/tty.h
u3.o: /usr/include/sys/user.h
u4.o: ../dev/u4.c /usr/include/sys/buf.h /usr/include/sys/dir.h
u4.o: /usr/include/sys/param.h /usr/include/sys/systm.h /usr/include/sys/tty.h
u4.o: /usr/include/sys/user.h
uh.o: ../dev/uh.c /usr/include/sys/conf.h /usr/include/sys/dir.h
uh.o: /usr/include/sys/file.h /usr/include/sys/param.h /usr/include/sys/systm.h
uh.o: /usr/include/sys/tty.h /usr/include/sys/uba.h /usr/include/sys/user.h
