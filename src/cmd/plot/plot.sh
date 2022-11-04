:
# SCCSID: @(#)plot.sh	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
PATH=/bin:/usr/bin
case $1 in
-T*)	t=$1
	shift ;;
*)	t=-T$TERM
esac
case $t in
-T450)	exec t450 $*;;
-T300)	exec t300 $*;;
-Tregis|-Tgigi|-Tvt125|-Tvt240|-Tvt241)	exec tregis $*;;
-T300S|-T300s)	exec t300s $*;;
-Tla50)	exec tla50 $*;;
-Tla100)	exec tla100 $*;;
-Tla210)	exec tregis $*;;
-Tver)	exec vplot $*;;
-Ttek|-T4014|-T)	exec tek $* ;;
*)  echo plot: terminal type $t not known 1>&2; exit 1
esac
