:
# SCCSID @(#)nu4.sh	3.0	4/21/86
# This shell script is called from /etc/nu to
# purge one's account.
#
# It is named "nu4.sh" instead of something like
# "killacct.sh" to discourage people from trying
# to run it standalone

case $# in
    4) ;;
    *)  echo "nu4.sh: Bad Argument Count: $# (should be 4)"
	exit 1
        ;;
esac

echo $0 $1 $2 $3 $4

exuser=$1
logindir=$2
Logfile=$3
debug=$4
egrepstr="^${exuser}\:"

N=
if [ $debug = 1 ]
then
    N=:
fi

echo rm -rf $logindir;
$N   rm -rf $logindir
echo rm -f /usr/spool/mail/$exuser
$N   rm -f /usr/spool/mail/$exuser;

# directories removed; now get user out of /etc/passwd
$N   echo deleting user $exuser from /etc/passwd file
echo "egrep -v $egrepstr /etc/passwd > /usr/adm/nu.temp"
$N   egrep -v $egrepstr /etc/passwd > /usr/adm/nu.temp

echo cp /usr/adm/nu.temp /etc/passwd
$N   cp /usr/adm/nu.temp /etc/passwd
echo rm -f /usr/adm/nu.temp
$N   rm -f /usr/adm/nu.temp

exit 0
