:
# SCCSID @(#)nu1.sh	3.0	4/21/86
# This shell script is called from /etc/nu to create
# a new directory for a new user, and to make the
# necessary links and permissions for it.
# 
# It is named "nu1.sh" instead of something like
# "makeuser.sh" to discourage people from trying to
# run it standalone.

case $# in
    5) ;;
    *)  echo "nu1.sh: Bad Argument Count: $# (should be 5)"
	exit 1
        ;;
esac

echo $0 $1 $2 $3 $4 $5
uid=$1
gid=$2
logindir=$3
clobber=$4
debug=$5

N=
if [ $debug = 1 ]
then
    N=:
fi

if [ $clobber = 1 ]
then
    echo rm -rf $logindir
$N       rm -rf $logindir
    echo mkdir $logindir
$N       mkdir $logindir
    if test $? != 0
    then
	echo "nu1.sh: cannot make directory $logindir!"
	exit 2
    fi
    echo chog $uid $logindir
$N       chog $uid $logindir
fi
exit 0
