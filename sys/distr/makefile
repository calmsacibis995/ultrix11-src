# SCCSID: @(#)makefile	2.10	9/20/87
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
#	ULTRIX-11 V2.0 Distribution kit makefile
#	
#	Fred Canter
#
#	The various file system prototypes are 
#	created on hp1 (RM03 drive 1) as follows:
#
#	hp10	bvroot  - ULTRIX-11 binary ROOT file system
#	hp11	bvusr   - ULTRIX-11 binary USR file system
#	hp12	saprog	- file system with s/a progs for magtape
#	hp14	bvos	- magtape kit optional software
#	hp15	scratch	- scratch file system for kit builds
#	hp16	proto	- all prototype files that used to be on RP06
#			  (boots, kernels, sas, machdep, etc)
#

CFLAGS = -O

nuset:	nuset.o
	cc -o nuset nuset.o

fsup:	fsup.o
	cc -o fsup fsup.o

setup:	setup.o
	cc -o setup setup.o
	strip setup

setup_osl:	setup_osl.o
	cc -o setup_osl setup_osl.o
	strip setup_osl

setup_help:	setup_help.o
	cc -o setup_help setup_help.o
	strip setup_help

bvfsup:	fsup setup setup_osl setup_help nuset

# The chmods are to prevent file status mismatch messages from fsup.
	@echo ""
	@echo "****** UPDATING BVROOT/BVUSR/BVOS FILE SYSTEMS ******"
	@echo ""
	/etc/mount /dev/hp11 /bvusr
	cd /bvusr/sys/net; chmod 644 *.o
	cd /bvusr/sys/ovnet; chmod 644 *.o
	cd /bvusr/sys/sys; chmod 644 *.o
	cd /bvusr/sys/ovsys; chmod 644 *.o
	cd /bvusr/sys/ovdev; chmod 644 *.o
	cd /bvusr/sys/sys; chmod 664 LIB1_id
	cd /bvusr/sys/dev; chmod 664 LIB2_id
	/etc/umount /dev/hp11
# Force update of following
	touch /usr/sys/ovsys/machdep.o /usr/sys/sys/LIB1_id
	fsup -i >fsup.log
	@echo ""
	@echo "no print log file for now!"
#	lpr fsup.log
	/etc/mount /dev/hp10 /bvroot
# Because copying a `ranlib'ed library causes `SYSDEV out of date' error
	cd /bvroot/lib; ranlib lib*.a
	/etc/mount /dev/hp11 /bvusr
	cd /bvusr/lib; ranlib lib*.a
	/etc/umount /dev/hp10
	cd /bvusr/sys/net; chmod 444 *.o
	cd /bvusr/sys/ovnet; chmod 444 *.o
	cd /bvusr/sys/sys; chmod 444 *.o
	cd /bvusr/sys/ovsys; chmod 444 *.o
	cd /bvusr/sys/ovdev; chmod 444 *.o; chmod 644 u1.o u2.o u3.o u4.o
	cd /bvusr/sys/sys; chmod 444 LIB1_id
	cd /bvusr/sys/dev; chmod 644 LIB2_id
	/etc/umount /dev/hp11
	/etc/mount /dev/hp14 /bvos
	cd /bvos/lib; ranlib lib*.a
	cp /usr/games/lib/fortunes.KIT /bvos/games/lib/fortunes
	cp /usr/lib/uucp/USERFILE.KIT /bvos/lib/uucp/USERFILE
# Copy all user driver sources and documents to /bvos/src
	rm -f /bvos/src/userdev/*
	cd /usr/sys/dev; cp u1.c u2.c u3.c u4.c /bvos/src/userdev
	cd /usr/sys/net; cp if_n1.c if_n2.c /bvos/src/userdev
	cp /usr/sys/dev/rs04_driver.c /bvos/src/userdev
	cp /usr/doc/userdev.doc /bvos/src/userdev
	cd /bvos/src/userdev; chog bin *; chmod 444 *
	-(cd /bvos/src/userdev; pack *)
	/etc/umount /dev/hp14

confup: bvfsup
	@echo ""
	@echo "****** UPDATING CONFIGUATION/SETUP/PROTO FILES ******"
	@echo ""
	/etc/mount /dev/hp10 /bvroot
	cp /usr/sys/distr/setup /bvroot/.setup/setup
	cp /usr/sys/distr/setup_osl /bvroot/.setup/setup_osl
	cp /usr/sys/distr/setup_help /bvroot/.setup/setup_help
	chmod 744 /bvroot/.setup/setup /bvroot/.setup/setup_osl
	chmod 744 /bvroot/.setup/setup_help
#	cp /etc/gettytab.bv /bvroot/etc/gettytab
	cp /bin/profile.KIT /bvroot/bin/.profile
	cp /cshrc.KIT /bvroot/.cshrc
	cp /login.KIT /bvroot/.login
	cp /etc/passwd.KIT /bvroot/etc/passwd
	cp /etc/group.KIT /bvroot/etc/group
	cp /etc/motd.KIT /bvroot/etc/motd
	cp /etc/rc.KIT /bvroot/etc/rc
	cp /etc/remote.KIT /bvroot/etc/remote
# next 3 are 0 length files (copy in case we add something to then later)
	cp /etc/hosts.KIT /bvroot/etc/hosts
	cp /etc/hosts.equivKIT /bvroot/etc/hosts.equiv
	cp /etc/networks.KIT /bvroot/etc/networks
	/etc/umount /dev/hp10
	/etc/mount /dev/hp11 /bvusr
	cp /usr/lib/aliases.KIT /bvusr/lib/aliases
# next 2 are 0 length files (copy incase we add to them later)
	cp /usr/lib/aliases.dirKIT /bvusr/lib/aliases.dir
	cp /usr/lib/aliases.pagKIT /bvusr/lib/aliases.pag
	cp /usr/lib/crontab.KIT /bvusr/lib/crontab
	cp /usr/lib/sendmail.cfKIT /bvusr/lib/sendmail.cf
	cd /bvusr/sys/conf; cp /usr/sys/conf/mf_bv makefile; chmod 400 makefile
	cd /bvusr/sys/conf; cp /usr/sys/conf/dump.s dump.s; chmod 400 dump.s
	cd /usr/sys/conf; cp mch_id_bv.h mch.h; make mch_id.o; rm mch.h
	cd /bvusr/sys/conf; mv /usr/sys/conf/mch_id.o mch_id.o; chmod 400 mch_id.o
	cd /usr/sys/conf; cp mch_id_bv.h mch.h; make mch_ov.o; rm mch.h
	cd /bvusr/sys/conf; mv /usr/sys/conf/mch_ov.o mch_ov.o; chmod 400 mch_ov.o
	cd /bvusr/sys/conf; cp /usr/sys/conf/mkconf mkconf; chmod 500 mkconf
	/etc/umount /dev/hp11

proto:	confup
	/etc/mkfs /dev/rhp16 10000 rm03 70 proto master
	/etc/mount /dev/hp16 /bvproto
# make directory containing packed man pages (/usr/man/man?/*).
	/etc/mount /dev/hp14 /bvos
	mkdir /bvproto/man
	chmod 755 /bvproto/man
	cd /bvos/man; tar cf - . |(cd /bvproto/man; tar xpf -)
	/etc/umount /dev/hp14
	-(cd /bvproto/man/man1; pack *)
	-(cd /bvproto/man/man2; pack *)
	-(cd /bvproto/man/man3; pack *)
	-(cd /bvproto/man/man4; pack *)
	-(cd /bvproto/man/man5; pack *)
	-(cd /bvproto/man/man6; pack *)
	-(cd /bvproto/man/man7; pack *)
	-(cd /bvproto/man/man8; pack *)
# Save directory files (/usr/sys/distr/bv*.dir) in bvproto/bvdir
	mkdir /bvproto/bvdir
	chmod 755 /bvproto/bvdir
	cp /usr/sys/distr/bvroot.dir /bvproto/bvdir
	cp /usr/sys/distr/bvusr.dir /bvproto/bvdir
	cp /usr/sys/distr/bvos.dir /bvproto/bvdir
# Save makefiles in bvproto/makefiles
	mkdir /bvproto/makefiles
	chmod 755 /bvproto/makefiles
	cp makefile mt2tk /bvproto/makefiles
	cp fsup nuset ready rl_optsw.proto ../sas/rx50.proto /bvproto/makefiles
	cp mkb mkb.* mkb_???? ../sas/m11dist /bvproto/makefiles
	cd /usr/sys/sas; make boot8; mv boot8 /bvproto; mv boot8.nm /bvproto
	cd /usr/sys/sas; make boot16; mv boot16 /bvproto; mv boot16.nm /bvproto
	cd /usr/sys/sas; make boot32; mv boot32 /bvproto; mv boot32.nm /bvproto
	chmod 644 /bvproto/boot*
	/etc/mount /dev/hp11 /bvusr
	mkdir /bvproto/machdep; chmod 700 /bvproto/machdep
	ar xv /bvusr/sys/sys/LIB1_id machdep.o
	nuset 023041
	cp machdep.o /bvproto/machdep/md8_id.o
#	ar rv /bvusr/sys/sys/LIB1_id machdep.o; rm -f machdep.o
	cp /bvusr/sys/ovsys/machdep.o .
	nuset 023041
	cp machdep.o /bvproto/machdep/md8_ov.o
	cp machdep.o /bvusr/sys/ovsys; rm -f machdep.o
	cd /bvusr/sys/conf; mkconf <generic.cf; make unix40
	mv /bvusr/sys/conf/unix_ov /bvproto/unix8; chmod 644 /bvproto/unix8
	ar xv /bvusr/sys/sys/LIB1_id machdep.o
	nuset 035056
	cp machdep.o /bvproto/machdep/md16_id.o
#	ar rv /bvusr/sys/sys/LIB1_id machdep.o; rm -f machdep.o
	cp /bvusr/sys/ovsys/machdep.o .
	nuset 035056
	cp machdep.o /bvproto/machdep/md16_ov.o
	cp machdep.o /bvusr/sys/ovsys; rm -f machdep.o
	cd /bvusr/sys/conf; mkconf <generic.cf; make unix40
	mv /bvusr/sys/conf/unix_ov /bvproto/unix16; chmod 644 /bvproto/unix16
	ar xv /bvusr/sys/sys/LIB1_id machdep.o
	nuset 022045
	cp machdep.o /bvproto/machdep/md32_id.o
#	ar rv /bvusr/sys/sys/LIB1_id machdep.o; rm -f machdep.o
	cp /bvusr/sys/ovsys/machdep.o .
	nuset 022045
	cp machdep.o /bvproto/machdep/md32_ov.o
	cp machdep.o /bvusr/sys/ovsys; rm -f machdep.o
	cd /bvusr/sys/conf; mkconf <generic.cf; make unix40
	mv /bvusr/sys/conf/unix_ov /bvproto/unix32; chmod 644 /bvproto/unix32
	cd /bvusr/sys/conf; rm -f l.s c.c text.sizes ovload dump_id.o dump_ov.o
	mkdir /bvproto/sas; chmod 700 /bvproto/sas
	cd /sas; cp htboot mtboot tkboot /bvproto/sas
	cd /sas; cp maketape contents.bv /bvproto/sas
	cd /sas; cp td1600.bv td800.bv tdtk50.bv /bvproto/sas
	cd /sas; cp bads boot copy dskinit icheck mkfs /bvproto/sas
	cd /sas; cp rabads restor scat sdload syscall /bvproto/sas
	chmod 644 /bvproto/sas/*; chmod 744 /bvproto/sas/maketape
	mkdir /bvproto/sdload; chmod 700 /bvproto/sdload
	cd /usr/sys/sas; make sdload; mv sdload /bvproto/sdload/mtload
	mv /usr/sys/sas/mtload.nm /bvproto/sdload/mtload.nm
	cd /usr/sys/sas; make rlsdload; mv rlload /bvproto/sdload/rlload
	mv /usr/sys/sas/rlload.nm /bvproto/sdload/rlload.nm
	cd /usr/sys/sas; make rcsdload; mv rcload /bvproto/sdload/rcload
	mv /usr/sys/sas/rcload.nm /bvproto/sdload/rcload.nm
	cd /usr/sys/sas; make rxsdload; mv rxload /bvproto/sdload/rxload
	mv /usr/sys/sas/rxload.nm /bvproto/sdload/rxload.nm
	chmod 644 /bvproto/sdload/*
	/etc/umount /dev/hp16
	/etc/umount /dev/hp11
	fsck -t /tmp/mkbproto.tmp /dev/rhp16
