:
# SCCSID: @(#)install.sh	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
PATH=/usr/local:/usr/ucb:/bin:/usr/bin::
export PATH
W=`whoami`
MAKE="make DESTDIR=${DESTDIR}"
N=
case $W in
	root) W=root ;;
	*) W= ;;
esac
for a
do
	D=/bin
	M=x
	I=auto
	U=bin
	G=
	b=
	S=yes
	LN=
	case $a in
		-n) MAKE="make -n DESTDIR=${DESTDIR}"; N=: ; continue;;
		*) echo ; echo installing $a
	esac
	case $a in
		all)	${MAKE} install
			break ;;
		local)	a=instlocal ;;
		subs)	a=instsubs ;;
		ac)	;;
		accton)	D=/etc ;;
		adb)	(cd $a; ${MAKE} install) ; continue ;;
		ar)	;;
		arcv)	;;
		as)	(cd $a; ${MAKE} install) ; continue ;;
		at)	;;
		atrun)	D=/usr/lib ; U=root ; M=s ;;
		awk)	(cd $a; ${MAKE} install) ; continue ;;
		badstat)	U=root ; M=s ;;
		basename)	;;
		bc)	(cd $a; ${MAKE} install) ; continue ;;
		bfs)	D=/usr/bin ;;
		bufstat)	U=root ; M=s ;;
		c)	(cd $a; ${MAKE} install) ; continue ;;
		cal)	;;
		calendar) (cd $a; ${MAKE} install) ; continue ;;
		cat)	;;
		catman)	D=/etc;;
		cb)	;;
		cc)	M=t ;;
		checkeq) D=/usr/bin ;;
		chgrp)	;;
		chmod)	;;
		chog)	;;
		chown)	;;
		chroot)	D=/usr/bin ;;
		clr|clear) a=clear; LN=clr ;;
		clri)	U=root ; M=o ;;
		cmp)	;;
		col)	;;
		comm)	;;
		cp) if ${MAKE} $a
		    then
			echo ./cp cp ${DESTDIR}/bin/cp
			$N ./cp cp ${DESTDIR}/bin/cp
			echo strip ${DESTDIR}/bin/cp
			$N strip ${DESTDIR}/bin/cp
			echo chmod 755 ${DESTDIR}/bin/cp
			$N chmod 755 ${DESTDIR}/bin/cp
			echo chog bin ${DESTDIR}/bin/cp
			$N chog bin ${DESTDIR}/bin/cp
			echo rm cp; $N rm cp
		    else
	    		echo "install.sh: INSTALL FAILED FOR $a TO ${DESTDIR}$D/$a"
		    fi
		    continue ;;
		cpio)	;;
		cpp)	(cd $a; ${MAKE} install) ; continue ;;
		cron)	D=/etc ; I=manually;;
		crypt)	;;
		csf)	D=/etc ; M=o ;;
		csh)	(cd $a; ${MAKE} install) ; continue ;;
		cshprofile) M=x; S=no; D=/etc ;;
		csplit)	D=/usr/bin ;;
		ctrace)	(cd $a; ${MAKE} install) ; continue ;;
		cu_v7m)	D=/usr/orphan/bin ;;
		cu_v7)	D=/usr/orphan/bin ;;
		custat_v7m) S=no; D=/usr/orphan/bin ;;
		cut)	D=/usr/bin ;;
		date)	;;
		dc)	(cd $a; ${MAKE} install) ; continue ;;
		dcheck)	;;
		dcopy)	D=/etc ;;
		dd)	;;
		decnet)	(cd $a; ${MAKE} install) ; continue ;;
		deroff)	;;
		df)	U=root ; M=s ;;
		diff)	(cd $a; ${MAKE} install) ; continue ;;
		diff3)	echo sccs get diff3.sh
			$N sccs get diff3.sh
			echo cp diff3.sh ${DESTDIR}/bin/diff3
			$N cp diff3.sh ${DESTDIR}/bin/diff3
			echo chog bin ${DESTDIR}/bin/diff3
			$N chog bin ${DESTDIR}/bin/diff3
			echo chmod 755 ${DESTDIR}/bin/diff3
			$N chmod 755 ${DESTDIR}/bin/diff3
			D=/usr/lib ;;
		diffh)	echo "diffh is installed with diff"; continue;;
		dmesg)	D=/etc ;;
		du)	;;
		dump)	;;
		dumpdir);;
		512dumpdir);;
		echo)	;;
		ed)	(cd $a; ${MAKE} install) ; continue ;;
		egrep)	;;
		el)	(cd $a; ${MAKE} install) ; continue ;;
		eqn)	(cd $a; ${MAKE} install) ; continue ;;
		ex)	(cd $a; ${MAKE} install) ; continue ;;
		expand)	D=/usr/bin;;
		expr)	;;
		f77)	(cd $a; ${MAKE} install) ; continue ;;
		factor)	;;
		false)	S=no ;;
		fgrep)	;;
		file)	;;
		find)	;;
		fpsim)	D=/etc ;;
		fsck)	;;
		fsdb)	D=/etc ; U=root ; M=o ;;
		getNAME) D=/usr/lib; M=xo ;;
		getopt)	D=/usr/bin ;;
		getty)	D=/etc ; I=manually ;;
		getu)	U=root; D=/usr/crash ;;
		graph)	D=/usr/bin ;;
		greek)	M=x; S=no; D=/usr/bin ;;
		grep)	;;
		hostid) ;;
		hostname) ;;
		help)	(cd $a; ${MAKE} install) ; continue ;;
		icheck)	(cd $a; ${MAKE} install) ; continue ;;
		init)	D=/etc ; M=ro ; I=manually ;;
		iostat)	;;
		ipatch) D=/etc ; U=root ; M=o ;;
		ipcs)	D=/usr/bin ;;
		ipcrm)	D=/usr/bin ;;
		join)	;;
		kill)	;;
		labelit) D=/etc ;;
		ld)	(cd $a; ${MAKE} install) ; continue ;;
		learn)	(cd $a; ${MAKE} install) ; continue ;;
		lex)	(cd $a; ${MAKE} install) ; continue ;;
		line)	D=/usr/bin ;;
		lint)	(cd $a; ${MAKE} install) ; continue ;;
		ln)	;;
		login)	U=root ; I=manually ; M=s ;;
		look)	;;
		lookbib) D=/usr/bin ; S=no ;;
		lorder)	S=no ;;
		lpr)	(cd $a; ${MAKE} install) ; continue ;;
		lpset)	D=/etc ; M=o ;;
		lprsetup) (cd $a; ${MAKE} install) ; continue ;;
		ls)	;;
		ltf)	(cd $a; ${MAKE} install) ; continue ;;
		m4)	(cd $a; ${MAKE} install) ; continue ;;
		mail|rmail) a=mail; U=root ; M=s ; LN=rmail ;;
		make)	(cd $a; ${MAKE} install) ; continue ;;
		makekey) D=/usr/lib ;;
		makewhatis) S=no; M=xo; D=/usr/lib ;;
		makewhatis.sed) D=/usr/lib; S=no; M=ro;;
		man|whatis|apropos) (cd man; ${MAKE} install) ; continue ;;
		memstat) U=root ; M=s ;;
		mesg)	;;
		mkconf)	(cd $a; ${MAKE} install) ; continue ;;
		mkdir)	U=root ; M=s ;;
		mkfs)	D=/etc ;;
		mknod)	D=/etc ;;
		mount)	D=/etc ;;
		more)	(cd $a; ${MAKE} install) ; continue ;;
		msf)	D=/etc ;;
		mt)	;;
		mv)	U=root ; M=s ;;
		ncheck)	(cd $a; ${MAKE} install) ; continue ;;
		neqn)	(cd $a; ${MAKE} install) ; continue ;;
		newfs)	D=/etc ;;
		newgrp)	U=root ; M=s ;;
		nice)	;;
		nl)	D=/usr/bin ;;
		nm)	;;
		nohup)	S=no ;;
		nu)	(cd $a; ${MAKE} install) ; continue ;;
		oc)	(cd $a; ${MAKE} install) ; continue ;;
		od)	;;
		oeqn)	(cd $a; ${MAKE} install) ; continue ;;
		olx)	(cd $a; ${MAKE} install) ; continue ;;
		oneqn)	(cd $a; ${MAKE} install) ; continue ;;
		oper)	(cd $a; ${MAKE} install) ; continue ;;
		otbl)	(cd $a; ${MAKE} install) ; continue ;;
		otroff)	(cd $a; ${MAKE} install) ; continue ;;
		pack)	D=/usr/bin ;;
		pascal)	(cd $a; ${MAKE} install) ; continue ;;
		passwd)	U=root ; M=s ;;
		paste)	D=/usr/bin ;;
		pcc)	(cd $a; ${MAKE} install) ; continue ;;
		plot)	(cd $a; ${MAKE} install) ; continue ;;
		pr)	;;
		prep)	(cd $a; ${MAKE} install) ; continue ;;
		primes)	;;
		printenv) D=/usr/bin ;;
		prof)	;;
		profile) M=x; S=no; D=/etc ;;
		ps)	U=root ; M=s ;;
		pstat)	U=root ; M=s ;;
		ptx)	;;
		pwd)	U=root ; M=s ;;
		quot)	U=root ; M=s ;;
		ranlib) ;;
		random.v7_NS) if ${MAKE} $a
			then
			    echo cp $a ${DESTDIR}/usr/orphan/bin/$a
			    $N cp $a ${DESTDIR}/usr/orphan/bin/$a
			    echo chmod 755 ${DESTDIR}/usr/orphan/bin/$a
			    $N chmod 755 ${DESTDIR}/usr/orphan/bin/$a
			    echo chog bin ${DESTDIR}/usr/orphan/bin/$a
			    $N chog bin ${DESTDIR}/usr/orphan/bin/$a
			else
	    		    echo "install.sh: INSTALL FAILED FOR $a TO ${DESTDIR}/usr/orphan/bin/$a"
			fi
			continue ;;
		rasize) U=root ; M=s ;;
		ratfor)	(cd $a; ${MAKE} install) ; continue ;;
		regcmp)	D=/usr/bin ;;
		refer)	(cd $a; ${MAKE} install) ; continue ;;
		restor)	(cd $a; ${MAKE} install) ; continue ;;
		rev)	;;
		rm)	;;
		rmdir)	U=root ; M=s ;;
		roff)	(cd $a; ${MAKE} install) ; continue ;;
		rx2fmt)	D=/etc ; U=root ; M=s ;;
		s5make)	(cd $a; ${MAKE} install) ; continue ;;
		sa)	;;
		sccs)	(cd $a; ${MAKE} install) ; continue ;;
		sdiff)	D=/usr/bin ;;
		sed)	(cd $a; ${MAKE} install) ; continue ;;
		sh)	(cd $a; ${MAKE} all);
			if [ ${DESTDIR}x = x ]
			then
			    echo sh MUST BE INSTALLED IN /bin/sh MANUALLY
			else
			   (cd $a; ${MAKE} install)
			fi
			continue ;;
		sh5)	(cd $a; ${MAKE} install); continue ;;
		shutdown) D=/etc ; U=root ; M=o ;;
		size)	;;
		sleep)	;;
		sort)	;;
		sp.v7_NS) if ${MAKE} $a
			then
			    echo cp $a ${DESTDIR}/usr/orphan/bin/$a
			    $N cp $a ${DESTDIR}/usr/orphan/bin/$a
			    echo chmod 755 ${DESTDIR}/usr/orphan/bin/$a
			    $N chmod 755 ${DESTDIR}/usr/orphan/bin/$a
			    echo chog bin ${DESTDIR}/usr/orphan/bin/$a
			    $N chog bin ${DESTDIR}/usr/orphan/bin/$a
			else
	    		    echo "install.sh: INSTALL FAILED FOR $a TO ${DESTDIR}/usr/orphan/bin/$a"
			fi
			continue ;;
		spell)	(cd $a; ${MAKE} install) ; continue ;;
		spline)	;;
		split)	;;
		stat)	D=/usr/bin;;
		strip)	if ${MAKE} $a
			then
			    echo cp strip ${DESTDIR}/bin/strip
			    $N cp strip ${DESTDIR}/bin/strip
			    echo ./strip ${DESTDIR}/bin/strip
			    $N ./strip ${DESTDIR}/bin/strip
			    echo chmod 755 ${DESTDIR}/bin/strip
			    $N chmod 755 ${DESTDIR}/bin/strip
			    echo chog bin ${DESTDIR}/bin/strip
			    $N chog bin ${DESTDIR}/bin/strip
			    echo rm strip
			    $N rm strip
			else
	    		    echo "install.sh: INSTALL FAILED FOR $a TO ${DESTDIR}$D/$a"
			fi
			continue ;;
		struct)	(cd $a; ${MAKE} install) ; continue ;;
		stty)	;;
		su)	U=root ; M=s ;;
		sum)	;;
		sync)	;;
		sysgen)	(cd $a; ${MAKE} install) ; continue ;;
		tabs)	;;
		tail)	;;
		tar)	(cd $a; ${MAKE} install) ; continue ;;
		tbl)	(cd $a; ${MAKE} install) ; continue ;;
		tc)	;;
		ted)	;;
		tee)	;;
		test|[)	a=test; LN=[ ;;
		time)	;;
		tip)	(cd $a; ${MAKE} install) ; continue ;;
		tk)	D=/usr/bin ;;
		touch)	;;
		tp)	(cd $a; ${MAKE} install) ; continue ;;
		tr)	;;
		troff)	(cd $a; ${MAKE} install) ; continue ;;
		true)	S=no ;;
		tsort)	;;
		tss)	D=/etc ; M=s ; U=root ;;
		tty)	;;
		ul)	D=/usr/bin ;;
		umount)	D=/etc ;;
		uname)	D=/usr/bin ;;
		unexpand)	D=/usr/bin ;;
		uniq)	;;
		units)	;;
		unpack|pcat)	D=/usr/bin; a=unpack; LN=pcat ;;
		update)	D=/etc ; I=manually ;;
		usat)	(cd $a ; ${MAKE} install) ; continue ;;
	   	uucp)	(cd $a ; ${MAKE} install) ; continue ;;
	   	v7tar)	(cd $a ; ${MAKE} install) ; continue ;;
		vipw)	D=/etc ;;
		volcopy) D=/etc ;;
		wall)	;;
		wc)	;;
		who)	;;
		write)	;;
		yacc)	(cd $a; ${MAKE} install) ; continue ;;
		yes)	;;
		zaptty)	D=/usr/bin ;;
		*)	echo "install.sh: DON'T KNOW ABOUT $a"
			continue ;;
	esac || continue

	if [ X$b = X ]
	then
		b=$a
	fi

	if ${MAKE} $a
	then
		true
	else
	    echo "install.sh: INSTALL FAILED FOR $a TO ${DESTDIR}$D/$b"
	    continue
	fi

	if [ ${DESTDIR}x = x -a $I = manually ]
	then
		echo $a MUST BE INSTALLED IN ${DESTDIR}$D/$b MANUALLY
		continue
	fi

	if [ $U != bin -o X$G != X ]
	then
		if test $W != root
		then
		    echo -n YOU MUST BE ROOT TO INSTALL $a
		    continue
		fi
	fi

	if [ -f $a -o X$N = X: ]
	then
	    echo cp $a ${DESTDIR}$D/$b; $N cp $a ${DESTDIR}$D/$b
	    echo chog $U ${DESTDIR}$D/$b ; $N chog $U ${DESTDIR}$D/$b 
	else
	    echo "install.sh: INSTALL FAILED FOR $a TO ${DESTDIR}$D/$b"
	    continue ;
	fi

	case $S in
	    yes) echo strip ${DESTDIR}$D/$b; $N strip ${DESTDIR}$D/$b ;;
	    *) ;;
	esac

	case $M in
	    t) echo chmod 1755 ${DESTDIR}$D/$b; $N chmod 1755 ${DESTDIR}$D/$b ;;
	    g) echo chmod 2755 ${DESTDIR}$D/$b; $N chmod 2755 ${DESTDIR}$D/$b ;;
	    s) echo chmod 4755 ${DESTDIR}$D/$b; $N chmod 4755 ${DESTDIR}$D/$b ;;
	    o) echo chmod 700 ${DESTDIR}$D/$b ; $N chmod 700 ${DESTDIR}$D/$b ;;
	    r) echo chmod 644 ${DESTDIR}$D/$b ; $N chmod 644 ${DESTDIR}$D/$b ;;
	    ro) echo chmod 600 ${DESTDIR}$D/$b ; $N chmod 600 ${DESTDIR}$D/$b ;;
	    x) echo chmod 755 ${DESTDIR}$D/$b ; $N chmod 755 ${DESTDIR}$D/$b ;;
	    xo) echo chmod 744 ${DESTDIR}$D/$b ; $N chmod 744 ${DESTDIR}$D/$b ;;
	esac

	if [ ${LN}x != x ]
	then
		echo rm -f ${DESTDIR}$D/$LN; $N rm -f ${DESTDIR}$D/$LN;
		echo ln ${DESTDIR}$D/$b ${DESTDIR}$D/$LN
		$N ln ${DESTDIR}$D/$b ${DESTDIR}$D/$LN
	fi
	echo rm $a
	$N rm $a
done
