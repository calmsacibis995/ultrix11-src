# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

ARCHIVE = LIB2_id

# The order that the files are listed here is the order in which
# the objects get archived. They can also get archived in the
# order listed in OBJECTS, so the two should be the same.
ARCHOBJ=${ARCHIVE}(bio.o)   ${ARCHIVE}(ct.o)     ${ARCHIVE}(dhdm.o) \
	${ARCHIVE}(dh.o)    ${ARCHIVE}(dhfdm.o)  ${ARCHIVE}(dn.o) \
	${ARCHIVE}(du.o)    ${ARCHIVE}(hp.o)     ${ARCHIVE}(hk.o) \
	${ARCHIVE}(ht.o)    ${ARCHIVE}(ra.o) \
	${ARCHIVE}(rl.o)    ${ARCHIVE}(kl.o)     ${ARCHIVE}(lp.o) \
	${ARCHIVE}(mem.o)   ${ARCHIVE}(partab.o) ${ARCHIVE}(pty.o) \
	${ARCHIVE}(rk.o)    ${ARCHIVE}(sys.o)    ${ARCHIVE}(tk.o) \
	${ARCHIVE}(tm.o)    ${ARCHIVE}(tty.o)    ${ARCHIVE}(ttynew.o) \
	${ARCHIVE}(dsort.o) ${ARCHIVE}(dz.o)     ${ARCHIVE}(rp.o) \
	${ARCHIVE}(ts.o)    ${ARCHIVE}(hx.o)     ${ARCHIVE}(u1.o) \
	${ARCHIVE}(u2.o)    ${ARCHIVE}(u3.o)     ${ARCHIVE}(u4.o) \
	${ARCHIVE}(uh.o)

OBJECTS = bio.o ct.o dhdm.o dh.o dhfdm.o dn.o du.o hp.o \
	hk.o ht.o ra.o rl.o kl.o lp.o mem.o \
	partab.o pty.o rk.o sys.o tk.o tm.o tty.o ttynew.o \
	dsort.o dz.o rp.o ts.o hx.o u1.o u2.o u3.o u4.o uh.o

SOURCES = bio.c ct.c dhdm.c dh.c dhfdm.c dn.c du.c hp.c \
	hk.c ht.c ra.c rl.c kl.c lp.c mem.c \
	partab.c pty.c rk.c sys.c tk.c tm.c tty.c ttynew.c \
	dsort.c dz.c rp.c ts.c hx.c u1.c u2.c u3.c u4.c uh.c

CFLAGS = -O -DSEP_ID -DKERNEL

all: ${ARCHIVE}

${ARCHIVE}: ${ARCHOBJ}

archive: ${OBJECTS}
	rm -f ${ARCHIVE}
	ar cr ${ARCHIVE} ${OBJECTS}
	rm -f ${OBJECTS}

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

cctest: ${OBJECTS}
	rm -f ${OBJECTS}

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	@sccs info

depend:
	-for i in ${SOURCES} ; \
	do \
		echo $$i; \
		grep '^#include' $$i /dev/null | grep -v devmaj | \
		sed -e 's/:[^<]*<\([^>]*\)>.*/ \/usr\/include\/\1/' \
		    -e 's,\.\./[a-zA-Z]*/,,' ; \
	done | \
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
	echo '/^# DO NOT DELETE THIS LINE/+1,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp makefile makefile.bak
	ed - makefile < eddep
	rm eddep makedep

# DO NOT DELETE THIS LINE -- make depend uses it
${ARCHIVE}(bio.o) bio.o: bio.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/buf.h /usr/include/sys/conf.h \
	/usr/include/sys/proc.h /usr/include/sys/seg.h

${ARCHIVE}(bio.o): 
	cc -c ${CFLAGS} bio.c
	ar r ${ARCHIVE} bio.o
	rm -f bio.o

${ARCHIVE}(ct.o) ct.o: ct.c /usr/include/sys/param.h \
	/usr/include/sys/dir.h /usr/include/sys/user.h /usr/include/sys/tty.h

${ARCHIVE}(ct.o): 
	cc -c ${CFLAGS} ct.c
	ar r ${ARCHIVE} ct.o
	rm -f ct.o

${ARCHIVE}(dhdm.o) dhdm.o: dhdm.c /usr/include/sys/param.h \
	/usr/include/sys/tty.h /usr/include/sys/conf.h /usr/include/sys/file.h

${ARCHIVE}(dhdm.o): 
	cc -c ${CFLAGS} dhdm.c
	ar r ${ARCHIVE} dhdm.o
	rm -f dhdm.o

${ARCHIVE}(dh.o) dh.o: dh.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/conf.h \
	/usr/include/sys/dir.h /usr/include/sys/user.h /usr/include/sys/tty.h

${ARCHIVE}(dh.o): 
	cc -c ${CFLAGS} dh.c
	ar r ${ARCHIVE} dh.o
	rm -f dh.o

${ARCHIVE}(dhfdm.o) dhfdm.o: dhfdm.c /usr/include/sys/param.h \
	/usr/include/sys/tty.h /usr/include/sys/conf.h

${ARCHIVE}(dhfdm.o): 
	cc -c ${CFLAGS} dhfdm.c
	ar r ${ARCHIVE} dhfdm.o
	rm -f dhfdm.o

${ARCHIVE}(dn.o) dn.o: dn.c /usr/include/sys/param.h \
	/usr/include/sys/dir.h /usr/include/sys/user.h

${ARCHIVE}(dn.o): 
	cc -c ${CFLAGS} dn.c
	ar r ${ARCHIVE} dn.o
	rm -f dn.o

${ARCHIVE}(du.o) du.o: du.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/buf.h

${ARCHIVE}(du.o): 
	cc -c ${CFLAGS} du.c
	ar r ${ARCHIVE} du.o
	rm -f du.o

${ARCHIVE}(hp.o) hp.o: hp.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/buf.h \
	/usr/include/sys/conf.h /usr/include/sys/dir.h /usr/include/sys/user.h \
	/usr/include/sys/seg.h /usr/include/sys/errlog.h \
	/usr/include/sys/bads.h /usr/include/sys/hpbad.h \
	/usr/include/sys/hp_info.h

${ARCHIVE}(hp.o): 
	cc -c ${CFLAGS} hp.c
	ar r ${ARCHIVE} hp.o
	rm -f hp.o

${ARCHIVE}(hk.o) hk.o: hk.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/buf.h \
	/usr/include/sys/conf.h /usr/include/sys/dir.h /usr/include/sys/user.h \
	/usr/include/sys/seg.h /usr/include/sys/errlog.h \
	/usr/include/sys/bads.h /usr/include/sys/hkbad.h

${ARCHIVE}(hk.o): 
	cc -c ${CFLAGS} hk.c
	ar r ${ARCHIVE} hk.o
	rm -f hk.o

${ARCHIVE}(ht.o) ht.o: ht.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/buf.h \
	/usr/include/sys/conf.h /usr/include/sys/dir.h /usr/include/sys/file.h \
	/usr/include/sys/user.h /usr/include/sys/errlog.h

${ARCHIVE}(ht.o): 
	cc -c ${CFLAGS} ht.c
	ar r ${ARCHIVE} ht.o
	rm -f ht.o

${ARCHIVE}(ra.o) ra.o: ra.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/buf.h \
	/usr/include/sys/conf.h /usr/include/sys/dir.h /usr/include/sys/user.h \
	/usr/include/sys/seg.h /usr/include/sys/errlog.h \
	/usr/include/sys/ra_info.h

${ARCHIVE}(ra.o): 
	cc -c ${CFLAGS} ra.c
	ar r ${ARCHIVE} ra.o
	rm -f ra.o

${ARCHIVE}(rl.o) rl.o: rl.c /usr/include/sys/param.h \
	/usr/include/sys/buf.h /usr/include/sys/dir.h /usr/include/sys/user.h \
	/usr/include/sys/systm.h /usr/include/sys/errlog.h

${ARCHIVE}(rl.o): 
	cc -c ${CFLAGS} rl.c
	ar r ${ARCHIVE} rl.o
	rm -f rl.o

${ARCHIVE}(kl.o) kl.o: kl.c /usr/include/sys/param.h \
	/usr/include/sys/conf.h /usr/include/sys/dir.h /usr/include/sys/user.h \
	/usr/include/sys/tty.h /usr/include/sys/systm.h

${ARCHIVE}(kl.o): 
	cc -c ${CFLAGS} kl.c
	ar r ${ARCHIVE} kl.o
	rm -f kl.o

${ARCHIVE}(lp.o) lp.o: lp.c /usr/include/sys/param.h \
	/usr/include/sys/dir.h /usr/include/sys/user.h /usr/include/sys/tty.h

${ARCHIVE}(lp.o): 
	cc -c ${CFLAGS} lp.c
	ar r ${ARCHIVE} lp.o
	rm -f lp.o

${ARCHIVE}(mem.o) mem.o: mem.c /usr/include/sys/param.h \
	/usr/include/sys/dir.h /usr/include/sys/user.h /usr/include/sys/conf.h \
	/usr/include/sys/seg.h

${ARCHIVE}(mem.o): 
	cc -c ${CFLAGS} mem.c
	ar r ${ARCHIVE} mem.o
	rm -f mem.o

${ARCHIVE}(partab.o) partab.o: partab.c

${ARCHIVE}(partab.o): 
	cc -c ${CFLAGS} partab.c
	ar r ${ARCHIVE} partab.o
	rm -f partab.o

${ARCHIVE}(pty.o) pty.o: pty.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/tty.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/conf.h /usr/include/sys/buf.h \
	/usr/include/sys/file.h /usr/include/sys/proc.h

${ARCHIVE}(pty.o): 
	cc -c ${CFLAGS} pty.c
	ar r ${ARCHIVE} pty.o
	rm -f pty.o

${ARCHIVE}(rk.o) rk.o: rk.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/buf.h \
	/usr/include/sys/conf.h /usr/include/sys/dir.h /usr/include/sys/user.h \
	/usr/include/sys/errlog.h

${ARCHIVE}(rk.o): 
	cc -c ${CFLAGS} rk.c
	ar r ${ARCHIVE} rk.o
	rm -f rk.o

${ARCHIVE}(sys.o) sys.o: sys.c /usr/include/sys/param.h \
	/usr/include/sys/conf.h /usr/include/sys/dir.h /usr/include/sys/user.h \
	/usr/include/sys/tty.h /usr/include/sys/proc.h

${ARCHIVE}(sys.o): 
	cc -c ${CFLAGS} sys.c
	ar r ${ARCHIVE} sys.o
	rm -f sys.o

${ARCHIVE}(tk.o) tk.o: tk.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/buf.h \
	/usr/include/sys/conf.h /usr/include/sys/dir.h /usr/include/sys/user.h \
	/usr/include/sys/seg.h /usr/include/sys/errlog.h \
	/usr/include/sys/file.h /usr/include/sys/tk_info.h

${ARCHIVE}(tk.o): 
	cc -c ${CFLAGS} tk.c
	ar r ${ARCHIVE} tk.o
	rm -f tk.o

${ARCHIVE}(tm.o) tm.o: tm.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/buf.h /usr/include/sys/dir.h \
	/usr/include/sys/conf.h /usr/include/sys/file.h \
	/usr/include/sys/user.h /usr/include/sys/errlog.h

${ARCHIVE}(tm.o): 
	cc -c ${CFLAGS} tm.c
	ar r ${ARCHIVE} tm.o
	rm -f tm.o

${ARCHIVE}(tty.o) tty.o: tty.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/tty.h /usr/include/sys/proc.h \
	/usr/include/sys/inode.h /usr/include/sys/file.h \
	/usr/include/sys/reg.h /usr/include/sys/conf.h

${ARCHIVE}(tty.o): 
	cc -c ${CFLAGS} tty.c
	ar r ${ARCHIVE} tty.o
	rm -f tty.o

${ARCHIVE}(ttynew.o) ttynew.o: ttynew.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/tty.h /usr/include/sys/proc.h \
	/usr/include/sys/inode.h /usr/include/sys/file.h \
	/usr/include/sys/reg.h /usr/include/sys/conf.h /usr/include/sys/buf.h

${ARCHIVE}(ttynew.o): 
	cc -c ${CFLAGS} ttynew.c
	ar r ${ARCHIVE} ttynew.o
	rm -f ttynew.o

${ARCHIVE}(dsort.o) dsort.o: dsort.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/buf.h /usr/include/sys/seg.h \
	/usr/include/sys/bads.h

${ARCHIVE}(dsort.o): 
	cc -c ${CFLAGS} dsort.c
	ar r ${ARCHIVE} dsort.o
	rm -f dsort.o

${ARCHIVE}(dz.o) dz.o: dz.c /usr/include/sys/param.h \
	/usr/include/sys/conf.h /usr/include/sys/dir.h /usr/include/sys/user.h \
	/usr/include/sys/tty.h /usr/include/sys/file.h

${ARCHIVE}(dz.o): 
	cc -c ${CFLAGS} dz.c
	ar r ${ARCHIVE} dz.o
	rm -f dz.o

${ARCHIVE}(rp.o) rp.o: rp.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/buf.h /usr/include/sys/dir.h \
	/usr/include/sys/conf.h /usr/include/sys/user.h \
	/usr/include/sys/errlog.h

${ARCHIVE}(rp.o): 
	cc -c ${CFLAGS} rp.c
	ar r ${ARCHIVE} rp.o
	rm -f rp.o

${ARCHIVE}(ts.o) ts.o: ts.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/buf.h \
	/usr/include/sys/conf.h /usr/include/sys/dir.h /usr/include/sys/file.h \
	/usr/include/sys/user.h /usr/include/sys/errlog.h \
	/usr/include/sys/ts_info.h

${ARCHIVE}(ts.o): 
	cc -c ${CFLAGS} ts.c
	ar r ${ARCHIVE} ts.o
	rm -f ts.o

${ARCHIVE}(hx.o) hx.o: hx.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/buf.h /usr/include/sys/dir.h \
	/usr/include/sys/user.h /usr/include/sys/conf.h /usr/include/sgtty.h \
	/usr/include/sys/errlog.h

${ARCHIVE}(hx.o): 
	cc -c ${CFLAGS} hx.c
	ar r ${ARCHIVE} hx.o
	rm -f hx.o

${ARCHIVE}(u1.o) u1.o: u1.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/buf.h /usr/include/sys/tty.h \
	/usr/include/sys/dir.h /usr/include/sys/user.h

${ARCHIVE}(u1.o): 
	cc -c ${CFLAGS} u1.c
	ar r ${ARCHIVE} u1.o
	rm -f u1.o

${ARCHIVE}(u2.o) u2.o: u2.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/buf.h /usr/include/sys/tty.h \
	/usr/include/sys/dir.h /usr/include/sys/user.h

${ARCHIVE}(u2.o): 
	cc -c ${CFLAGS} u2.c
	ar r ${ARCHIVE} u2.o
	rm -f u2.o

${ARCHIVE}(u3.o) u3.o: u3.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/buf.h /usr/include/sys/tty.h \
	/usr/include/sys/dir.h /usr/include/sys/user.h

${ARCHIVE}(u3.o): 
	cc -c ${CFLAGS} u3.c
	ar r ${ARCHIVE} u3.o
	rm -f u3.o

${ARCHIVE}(u4.o) u4.o: u4.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/buf.h /usr/include/sys/tty.h \
	/usr/include/sys/dir.h /usr/include/sys/user.h

${ARCHIVE}(u4.o): 
	cc -c ${CFLAGS} u4.c
	ar r ${ARCHIVE} u4.o
	rm -f u4.o

${ARCHIVE}(uh.o) uh.o: uh.c /usr/include/sys/param.h \
	/usr/include/sys/systm.h /usr/include/sys/conf.h \
	/usr/include/sys/dir.h /usr/include/sys/user.h /usr/include/sys/tty.h \
	/usr/include/sys/file.h

${ARCHIVE}(uh.o): 
	cc -c ${CFLAGS} uh.c
	ar r ${ARCHIVE} uh.o
	rm -f uh.o
