:
# SCCSID: @(#)struct.sh	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
trap "rm -f /tmp/struct*$$" 0 1 2 3 13 15
files=no
for i
do
	case $i in
	-*)	;;
	*)	files=yes
	esac
done

case $files in
yes)
	/usr/lib/struct/structure $* >/tmp/struct$$
	;;
no)
	cat >/tmp/structin$$
	/usr/lib/struct/structure /tmp/structin$$ $* >/tmp/struct$$
esac &&
	/usr/lib/struct/beautify</tmp/struct$$
