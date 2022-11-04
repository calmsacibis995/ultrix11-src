:
# SCCSID @(#)nu2.sh	3.0	4/21/86
# This shell script is called from /etc/nu to
# initialize the contents of a newly-created user's
# directory.
#
# It is named "nu2.sh" instead of something like
# "addfiles.sh" to discourage people from trying to
# run it standalone

case $# in
    5) ;;
    *)  echo "nu2.sh: Bad Argument Count: $# (should be 5)"
	exit 1
        ;;
esac

echo $0 $1 $2 $3 $4 $5

uid=$1
gid=$2
logindir=$3
debug=$4
sysV=$5

N=
if [ $debug = 1 ]
then
    N=:
fi

echo cd $logindir
$N   cd $logindir

if test $? != 0
then
    echo "nu2.sh: cannot cd to new user's login directory ($logindir)!"
    exit 2
fi

test -d /usr/skel
if test $? != 0
then
    echo "nu2.sh: cannot find prototype directory (/usr/skel)."
    exit 3
fi

echo cp /usr/skel/.[a-z]* .
$N   cp /usr/skel/.[a-z]* .
if test $? != 0
then
    echo "nu2.sh: cannot copy /usr/skel/.[a-z]* to new user's home directory."
    exit 4
fi

if [ $sysV = "yes" ]
then
    echo "cat /usr/skel/profileV >> .profile"
$N  cat /usr/skel/profileV >> .profile
    echo "cat /usr/skel/loginV >> .login"
$N  cat /usr/skel/loginV >> .login
fi

echo chog $uid .[a-z]*
$N   chog $uid .[a-z]*
if test $? != 0
then
    echo "nu2.sh: cannot chog $uid the .* files in new user's home directory."
    exit 5
fi
exit 0
