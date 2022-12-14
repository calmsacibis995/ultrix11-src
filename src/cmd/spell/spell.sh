SCCSID="@(#)spell.sh	3.0	4/24/86"
: B flags, F files, V data for -v
: SPELL_D dictionary, SPELL_H history, SPELL_S stop,
SPELL_H=${SPELL_H-/usr/dict/spellhist}
T=/tmp/spell.$$
V=/dev/null
F= B=
trap "rm -f $T*; exit" 0 1 2 13 15
for A in $*
do
	case $A in
	-v)	B="$B -v"
		V=${T}a ;;
	-a)	;;
	-b) 	SPELL_D=${SPELL_D-/usr/dict/hlistb}
		B="$B -b" ;;
	*)	F="$F $A"
	esac
	done
deroff -w $F |\
  sort -u |\
  /usr/lib/spell ${SPELL_S-/usr/dict/hstop} $T |\
  /usr/lib/spell ${SPELL_D-/usr/dict/hlista} $V $B |\
  sort -u +0f +0 - $T |\
  tee -a $SPELL_H
who am i >>$SPELL_H 2>/dev/null
case $V in
/dev/null)	exit
esac
sed '/^\./d' $V | sort -u +1f +0
