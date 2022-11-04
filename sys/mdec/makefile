# SCCSID:="@(#)makefile	3.0	4/21/86"
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
#	ULTRIX-11 block zero bootstrap makefile
#
#	Fred Canter 3/22/83
#
SOURCES= rkuboot.s mluboot.s rluboot.s hkuboot.s rpuboot.s rauboot.s hpuboot.s

all: sumsg hkub hpub rlub rpub mlub rkub raub

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

sumsg:
	@echo ""
	@echo "Must be super-user or this will not work !!!!"
	@echo ""
	@echo "??uboot.s MUST BE EDITED TO SELECT FILE SYSTEM BLOCK SIZE!"
	@echo ""

hpuboot: sumsg hpub

hpub:
	as hpuboot.s
	-strip a.out
	dd if=a.out of=${DESTDIR}/mdec/hpuboot bs=16 skip=1
	chmod 644 ${DESTDIR}/mdec/hpuboot
	rm -f a.out

rauboot: sumsg raub

raub:
	as rauboot.s
	-strip a.out
	dd if=a.out of=${DESTDIR}/mdec/rauboot bs=16 skip=1
	chmod 644 ${DESTDIR}/mdec/rauboot
	rm -f a.out

rpuboot: sumsg rpub

rpub:
	as rpuboot.s
	-strip a.out
	dd if=a.out of=${DESTDIR}/mdec/rpuboot bs=16 skip=1
	chmod 644 ${DESTDIR}/mdec/rpuboot
	rm -f a.out

hkuboot: sumsg hkub

hkub:
	as hkuboot.s
	-strip a.out
	dd if=a.out of=${DESTDIR}/mdec/hkuboot bs=16 skip=1
	chmod 644 ${DESTDIR}/mdec/hkuboot
	rm -f a.out

rluboot: sumsg rlub

rlub:
	as rluboot.s
	-strip a.out
	dd if=a.out of=${DESTDIR}/mdec/rluboot bs=16 skip=1
	chmod 644 ${DESTDIR}/mdec/rluboot
	rm -f a.out

mluboot: sumsg mlub

mlub:
	as mluboot.s
	-strip a.out
	dd if=a.out of=${DESTDIR}/mdec/mluboot bs=16 skip=1
	chmod 644 ${DESTDIR}/mdec/mluboot
	rm -f a.out

rkuboot: sumsg rkub

rkub:
	as rkuboot.s
	-strip a.out
	dd if=a.out of=${DESTDIR}/mdec/rkuboot bs=16 skip=1
	chmod 644 ${DESTDIR}/mdec/rkuboot
	rm -f a.out

clean:
	rm -f a.out

clobber: clean
	sccs clean

