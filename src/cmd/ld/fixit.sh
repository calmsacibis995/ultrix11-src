:
# SCCSID: @(#)fixit.sh	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
for i do
	$CC -O -S -c $i.c

ed - <<EOF $i.s 
g/^[ 	]*\.data/s/data/text/
w
q
EOF
	$AS -o $i.o $i.s
	rm $i.s
done
