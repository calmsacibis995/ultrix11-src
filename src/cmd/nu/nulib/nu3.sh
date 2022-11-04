:
# SCCSID @(#)nu3.sh	3.0	4/21/86
# This shell script is called from /etc/nu to
# purge a users account without removing it from
# /etc/passwd, in order to preserve accounting info.
#
# It is named "nu3.sh" instead of something like
# "deleteacct.sh" to discourage people from trying
# to run it standalone.

case $# in
    4) ;;
    *)  echo "nu3.sh: Bad arg count: $# (expected 4)"
	exit 1
        ;;
esac

echo $0 $1 $2 $3 $4

exuser=$1
logindir=$2
Logfile=$3
debug=$4

N=
if [ $debug = 1 ]
then
    N=:
fi

echo rm -rf $logindir
$N   rm -rf $logindir
echo rm -f /usr/spool/mail/$exuser
$N   rm -f /usr/spool/mail/$exuser

exit 0
