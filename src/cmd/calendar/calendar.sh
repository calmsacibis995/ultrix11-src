:
# SCCSID: @(#)calendar.sh	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
PATH=/bin:/usr/bin
tmp=/tmp/cal$$
tmp2=/tmp/cal_$$
trap "rm -f $tmp $tmp2; exit" 0 1 2 13 15
/usr/lib/calendar >$tmp
case $1 in
-)
	sed '
		s/\([^:]*\):.*:\(.*\):[^:]*$/y=\2 z=\1/
	' /etc/passwd \
	| while read x
	do
		eval $x
		if test -r $y/calendar
		then
			egrep -f $tmp $y/calendar > $tmp2 2>/dev/null
			if test -s $tmp2
			then
				/bin/mail $z < $tmp2
			fi
		fi

	done;;
*)
	egrep -f $tmp calendar
esac
