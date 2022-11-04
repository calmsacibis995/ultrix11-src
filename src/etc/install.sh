SCCSID="@(#)install.sh	3.0	4/22/86"

######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################

W=`whoami`
PATH=:/usr/ucb:/usr/local:/bin:/usr/bin
export PATH
N=
MAKE="make DESTDIR=${DESTDIR}"
case $W in
	root) W=root ;;
	*) W=
esac
for a
do
	I=auto
	M=x
	D=/usr/etc
	U=bin
	G=
	b=
	S=yes
	LN=

	case $a in
		-n) MAKE="make -n DESTDIR=${DESTDIR}"; N=: ; continue ;;
		*) echo ; echo installing $a ;;
	esac

	case $a in
		all) ${MAKE} install DESTDIR=${DESTDIR}
			continue ;;
		arp) D=/etc ;;
		dgated) M=r; S=no ;;
		ftpd) (cd $a; ${MAKE} install DESTDIR=${DESTDIR})
			continue ;;
		ifconfig) D=/etc ;;
		inetd) ;;
		miscd) ;;
		netsetup) ;;
		ping) (cd $a; ${MAKE} install DESTDIR=${DESTDIR})
			continue ;;
		rawfs) D=/etc ;;
		rdate) D=/etc ;;
		rexecd) ;;
		rlogind) ;;
		route) D=/etc ;;
		routed) (cd $a; ${MAKE} install DESTDIR=${DESTDIR})
			continue ;;
		rshd) ;;
		rwhod) ;;
		telnetd) ;;
		tftpd) ;;
		tzname) D=/etc ;;
		uucpsetup) S=no ;;
		*) echo "ETC/install.sh: DON'T KNOW ABOUT $a"
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
	    echo "ETC/install.sh: INSTALL FAILED FOR $a TO ${DESTDIR}$D/$b"
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
	    case ${G}X in
		X) echo chog $U ${DESTDIR}$D/$b; $N chog $U ${DESTDIR}$D/$b ;;
		*) echo chown $U ${DESTDIR}$D/$b; $N chown $U ${DESTDIR}$D/$b
		   echo chgrp $G ${DESTDIR}$D/$a; $N chgrp $G ${DESTDIR}$D/$b
	    esac
	else
	    echo "ETC/install.sh: INSTALL FAILED FOR $a TO ${DESTDIR}$D/$b"
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
