# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
#	ULTRIX-11 /usr/sys/conf/makefile
# ***** IF THIS FILE CHANGED, mf_bv MAY ALSO NEED CHANGES *****
#
#	This makefile makes the separate I & D space and overlay
#	kernels for the source & USEP versions of ULTRIX-11.
#	This makefile can be used standalone to make ULTRIX-11 or in
#	conjunction with the sysgen program, the latter is the
#	recommended method.

OPTS = -DKERNEL
CPP = /lib/cpp -P
# ASFLAGS used by System V make
ASFLAGS = -

unix unix44 unix45 unix53 unix55 unix70 unix73 unix83 unix84:	kernel_id

kernel_id: l.o dump_id.o mch_id.o c.o dds.o tds.o nds.o dksizes.o ec.o ../sys/LIB1_id ../dev/LIB2_id
	@echo ""
	@echo "The output file will be named unix_id!"
	@echo ""
	ovload
	@echo ""
	@echo "The unix_id sizes must be within the following limits:"
	@echo ""
	@echo "root text segment > 49152 but <= 57344"
	@echo "overlay text segments <= 8192, 15 overlays maximum"
	@echo "bss + data segments <= 49088 total"
	@echo ""
	@echo "root+(overlay 1, overlay 2,...overlay n)+data+bss = root+data = (total)"
	@echo ""
	size unix_id
	rm l.o dump_id.o c.o dds.o tds.o nds.o dksizes.o ec.o

all:	all40 all70

all44 all45 all55 all70: allsys_id alldev_id allnet_id

allsys_id:
	cd ../sys; rm -rf *.o LIB1_id; make archive

alldev_id:
	cd ../dev; rm -rf *.o LIB2_id; make archive

allnet_id:
	cd ../net; rm -rf *.o; make all

c.o:	c.c
	cc -c -O -DSEP_ID ${OPTS} c.c

ec.o:	dds.h ec.c
	cc -S ${OPTS} ec.c
	ed - ec.s < :comm-to-bss
	as - -o ec.o ec.s
	rm ec.s

dds.o:	dds.h dds.c
	cc -c -O -DSEP_ID ${OPTS} dds.c

tds.o:	dds.h tds.c
	cc -c -O -DSEP_ID ${OPTS} tds.c

nds.o:	dds.h nds.c
	cc -c -O -DSEP_ID ${OPTS} nds.c

dksizes.o:	dds.h dksizes.c
	cc -c -O -DSEP_ID -DUEG_LOCAL ${OPTS} dksizes.c

mch_id.o:	mch.s mch.h
	${CPP} -DSEP_ID -DK_OV ${OPTS} mch.s >mch.i
	as - -o mch_id.o mch.i
	rm -f mch.i

dump_id.o:	dump.s mch.h
	${CPP} -DSEP_ID -DK_OV ${OPTS} dump.s >dump.i
	as - -o dump_id.o dump.i
	rm -f dump.i

unix23 unix24 unix34 unix40 unix60:	kernel_ov

kernel_ov:	l.o dump_ov.o mch_ov.o c_ov.o dds_ov.o tds_ov.o nds_ov.o dksizes_ov.o ec.o
	@echo ""
	@echo "The output file will be named unix_ov!"
	@echo ""
	ovload
	@echo ""
	@echo "The unix_ov sizes must be within the following limits:"
	@echo ""
	@echo "root text segment > 8192 but <= 16384"
	@echo "overlay text segments <= 8192, 15 overlays maximum"
	@echo "bss + data segments <= 24576 total"
	@echo ""
	@echo "root+(overlay 1, overlay 2,...overlay n)+data+bss = root+data = (total)"
	@echo ""
	size unix_ov
	@echo ""
	rm -f l.o dump_ov.o c_ov.o dds_ov.o tds_ov.o nds_ov.o dksizes_ov.o ec.o

all23 all24 all34 all40 all60: allsys_ov alldev_ov allnet_ov

allsys_ov:
	cd ../ovsys; rm -f *.o; make all

alldev_ov:
	cd ../ovdev; rm -f *.o; make all

allnet_ov:
	cd ../ovnet; rm -f *.o; make all

c_ov.o:	c.c
	cc -c -O -DK_OV ${OPTS} c.c
	mv c.o c_ov.o

dds_ov.o:	dds.h dds.c
	cc -c -O -DK_OV ${OPTS} dds.c
	mv dds.o dds_ov.o

tds_ov.o:	dds.h tds.c
	cc -c -O -DK_OV ${OPTS} tds.c
	mv tds.o tds_ov.o

nds_ov.o:	dds.h nds.c
	cc -c -O -DK_OV ${OPTS} nds.c
	mv nds.o nds_ov.o

dksizes_ov.o:	dds.h dksizes.c
	cc -c -O -DK_OV -DUEG_LOCAL ${OPTS} dksizes.c
	mv dksizes.o dksizes_ov.o

mch_ov.o:	mch.s mch.h
	${CPP} -DK_OV ${OPTS} mch.s >mch.i
	as - -o mch_ov.o mch.i
	rm -f mch.i

dump_ov.o:	dump.s mch.h
	${CPP} -DK_OV ${OPTS} dump.s >dump.i
	as - -o dump_ov.o dump.i
	rm -f dump.i
