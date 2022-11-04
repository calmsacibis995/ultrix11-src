# SCCSID: @(#)makefile	3.0 4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile residing in /usr/src/cmd
#
HERE=`pwd`

SOURCES = ac.c accton.c ar.c arcv.c at.c atrun.c badstat.c \
	basename.c bfs.c bufstat.c cal.c cat.c catman.c cb.c \
	cc.c checkeq.c chgrp.c chmod.c chog.c chown.c chroot.c \
	clear.c clri.c cmp.c col.c comm.c cp.c cpio.c cron.c \
	crypt.c csf.c cshprofile csplit.c cu.c cuo.c \
	custat_v7m cut.c date.c dcheck.c dcopy.c dd.c \
	deroff.c df.c diff3.c dmesg.c du.c dump.c dumpdir.c \
	echo.c egrep.y expand.c expr.y factor.s fgrep.c \
	file.c find.c fpsim.c fsck.c \
	fsdb.c getNAME.c getopt.c getty.c getu.c graph.c \
	greek grep.c hostid.c hostname.c init.c \
	iostat.c ipatch.c ipcs.c ipcrm.c join.c kill.c labelit.c \
	lim.s line.c ln.c login.c look.c lookbib lorder lpset.c \
	ls.c mail.c makekey.c makewhatis makewhatis.sed memstat.c \
	mesg.c mkdir.c mkfs.c mknod.c \
	mount.c msf.c mt.c mv.c newfs.c newgrp.c nice.c nl.c nm.c \
	nohup od.c pack.c passwd.c \
	paste.c pr.c primes.s printenv.c prof.c profile ps.c \
	pstat.c ptx.c pwd.c quot.c ranlib.c rasize.c regcmp.c \
	rev.c rm.c rmdir.c \
	rx2fmt.c sa.c sdiff.c size.c sleep.c sort.c spline.c split.c stat.c \
	strip.c stty.c su.c sum.c sumck.c sync.c tabs.c tail.c tc.c ted.c \
	tee.c test.c time.c tk.c touch.c tr.c tsort.c tss.c tty.c \
	ul.c umount.c uname.c unexpand.c uniq.c units.c unpack.c update.c \
	vipw.c volcopy.c wall.c wc.c who.c write.c yes.c zaptty.c

SUBMAKES= subsources subclean subclobber

# Everything should be in the following LOCS list.  Those commands
# that are explicitly listed here are things that don't fit into the
# other categories, ie. shell scripts, things that need -ltermlib, -lPW, etc.

LOCS=	512dumpdir clear cshprofile cu_v7m cu_v7 custat_v7m \
	dd factor false graph greek init login lookbib lorder \
	makewhatis makewhatis.sed nohup primes prof profile \
	random.v7_NS regcmp sp.v7_NS true ul \
	${CMD407} ${CMD410} ${SYS5} ${YACCS}

# These are non-shared text commands that are made via
#	cc -o foo -O foo.c
CMD407 = badstat bufstat cut dcheck dd dump fgrep fpsim \
	fsck fsdb ipatch labelit memstat msf mt paste \
	rx2fmt ted tss vipw dumpdir 

# These are shared text commands that are made via
#	cc -n -o foo -O foo.c
CMD410 = ac accton ar arcv at atrun basename bfs cal \
	cat catman cb cc checkeq chgrp chmod chog chown \
	chroot clri cmp col comm cp cpio cron crypt csf \
	date dcopy deroff df diff3 dmesg du echo expand file \
	find getNAME getopt getty getu grep hostid hostname \
	iostat ipcs ipcrm join kill line ln look \
	lpset ls mail makekey mesg mkdir mkfs mknod \
	mount mv newfs newgrp nice nl nm od pack passwd pr \
	printenv ps pstat ptx pwd quot ranlib rasize \
	rev rm rmdir sa sdiff size sleep sort spline split \
	stat strip stty su sum sync tabs tail tc tee test \
	time tk touch tr tsort tty umount uname unexpand uniq \
	units unpack update volcopy wall wc who write \
	yes zaptty

# These are commands that are made from .y files
YACCS =	egrep expr

SUBS=	adb as awk bc c calendar cpp csh ctrace dc diff decnet \
	ed el eqn ex f77 help icheck ld learn lex lint lpr lprsetup \
	ltf m4 make man mkconf more ncheck neqn nu oc oeqn olx oper \
	otroff otbl pascal pcc plot prep restor ratfor refer roff \
	s5make sccs sed sh sh5 spell struct sysgen tar tbl \
	tip tp troff usat uucp v7tar yacc

# Programs which require the System 5 environment to compile
SYS5=	csplit

all:	${LOCS} ${SUBS}
	@echo All Done

sources: ${SOURCES} subsources
	@make -f subsources sources "SUBS=${SUBS}"

tags:	subtags
	@make -f subtags tags "SUBS=${SUBS}"

sccsinfo: subsccsinfo
	sccs info
	@make -f subsccsinfo sccsinfo "SUBS=${SUBS}"

${SOURCES} ${SUBMAKES}:
	sccs get $@

install: forceit instlocal instsubs

instlocal:
	@install ${LOCS}

instsubs:
	@install ${SUBS}

local:	${LOCS}
	@echo local Done

${CMD410}:
	cc -n -o $@ -O $@.c

${CMD407}:
	cc -O $@.c -o $@

${YACCS}:
	yacc $@.y
	mv y.tab.c $@.c
	cc -n -O $@.c -o $@
	rm $@.c

${SYS5}:
	cc -n -Y -o $@ -O $@.c

subs:	${SUBS}

clean:	subclean
	@(echo)
	@(echo "Cleaning cmd")
	rm -f *.o a.out core log ${LOCS}
	@make -f subclean clean "SUBS=${SUBS}"

clobber: subclobber
	@(echo)
	@(echo "Clobbering cmd")
	rm -f *.o a.out core log ${LOCS}
	sccs clean
	@make -f subclobber clobber "SUBS=${SUBS}"

#	These are programs that don't fall under any
#	of the default compilation rules.

true:
	echo "exit 0" > true
false:
	echo "exit 1" > false

graph:	graph.c
	cc -n -O graph.c -o graph -lplot -lm

prof:	prof.c
	cc -n -O -Dplot prof.c -lplot -o prof

clear:	clear.c
ul:	ul.c
clear ul:
	cc -n -O $@.c -o $@ -ltermlib

cu_v7m:	cu.c
	cc -n -O cu.c -o cu_v7m

cu_v7:	cuo.c
	cc -n -O cuo.c -o cu_v7

factor:	factor.s
	as -o factor factor.s

init:	lim.s init.c sumck
	cc -n -O init.c lim.s -o init
	./sumck init
	strip init

login:	lim.s login.c sumck
	cc -n -O login.c lim.s -o login
	./sumck login
	strip login

sumck:	sumck.c
	cc -O -o sumck sumck.c

primes:	primes.s
	as -o primes /usr/include/sys.s primes.s

# regcmp needs libPW for regcmp, regex
regcmp:	regcmp.c
	cc -n -o regcmp -O regcmp.c -lPW

# These next have no source, there are only executables available
random.v7_NS:
	@(echo "no source file exists for random.v7_NS")
	false
sp.v7_NS:
	@(echo "no source file exists for sp.v7_NS")
	false

# These just specify the source -> destination dependencies
ac:	ac.c
accton: accton.c
ar:	ar.c
arcv:	arcv.c
at:	at.c
atrun:	atrun.c
badstat:	badstat.c
basename:	basename.c
bfs:	bfs.c
bufstat:	bufstat.c
cal:	cal.c
cat:	cat.c
catman:	catman.c
cb:	cb.c
cc:	cc.c
checkeq:	checkeq.c
chgrp:	chgrp.c
chmod:	chmod.c
chog:	chog.c
chown:	chown.c
chroot:	chroot.c
clri:	clri.c
cmp:	cmp.c
col:	col.c
comm:	comm.c
cp:	cp.c
cpio:	cpio.c
cron:	cron.c
crypt:	crypt.c
csf:	csf.c
cshprofile:	cshprofile
csplit:	csplit.c
cut:	cut.c
date:	date.c
dcheck:	dcheck.c
dcopy:	dcopy.c
dd:	dd.c
deroff:	deroff.c
df:	df.c
diff3:	diff3.c
dmesg:	dmesg.c
du:	du.c
dump:	dump.c
dumpdir:	dumpdir.c
512dumpdir:	dumpdir.c
	cc  -O -DREST_512 -o 512dumpdir dumpdir.c
echo:	echo.c
egrep:	egrep.y
expand:	expand.c
expr:	expr.y
fgrep:	fgrep.c
file:	file.c
find:	find.c
fpsim:	fpsim.c
fsck:	fsck.c
fsdb:	fsdb.c
getNAME:	getNAME.c
getopt:	getopt.c
getty:	getty.c
getu:	getu.c
greek:	greek
grep:	grep.c
hostid:	hostid.c
hostname:	hostname.c
iostat:	iostat.c
ipatch:	ipatch.c
ipcs:	ipcs.c
ipcrm:	ipcrm.c
join:	join.c
kill:	kill.c
labelit:	labelit.c
line:	line.c
ln:	ln.c
login:	login.c
look:	look.c
lookbib:	lookbib
lorder:	lorder
lpset:	lpset.c
ls:	ls.c
mail:	mail.c
makekey:	makekey.c
makewhatis:	makewhatis
makewhatis.sed:	makewhatis.sed
memstat:	memstat.c
mesg:	mesg.c
mkdir:	mkdir.c
mkfs:	mkfs.c
mknod:	mknod.c
mount:	mount.c
msf:	msf.c
mt:	mt.c
mv:	mv.c
newfs:	newfs.c
newgrp:	newgrp.c
nice:	nice.c
nl:	nl.c
nm:	nm.c
nohup:	nohup
od:	od.c
pack:	pack.c
passwd:	passwd.c
paste:	paste.c
pr:	pr.c
printenv:	printenv.c
profile:	profile
ps:	ps.c
pstat:	pstat.c
ptx:	ptx.c
pwd:	pwd.c
quot:	quot.c
ranlib:	ranlib.c
rasize:	rasize.c
regcmp:	regcmp.c
rev:	rev.c
rm:	rm.c
rmdir:	rmdir.c
rx2fmt:	rx2fmt.c
sa:	sa.c
sdiff:	sdiff
size:	size.c
sleep:	sleep.c
sort:	sort.c
spline:	spline.c
split:	split.c
stat:	stat.c
strip:	strip.c
stty:	stty.c
su:	su.c
sum:	sum.c
sync:	sync.c
tabs:	tabs.c
tail:	tail.c
tc:	tc.c
ted:	ted.c
tee:	tee.c
test:	test.c
time:	time.c
tk:	tk.c
touch:	touch.c
tr:	tr.c
tsort:	tsort.c
tss:	tss.c
tty:	tty.c
umount:	umount.c
uname:	uname.c
unexpand:	unexpand.c
uniq:	uniq.c
units:	units.c
unpack:	unpack.c
update:	update.c
vipw:	vipw.c
volcopy:	volcopy.c
wall:	wall.c
wc:	wc.c
who:	who.c
write:	write.c
yes:	yes.c
zaptty:	zaptty.c

# Beginning of subdirectory commands

${SUBS}: forceit
	@echo
	@echo Making $@
	@(cd $@; make all)

diffh:
	@echo
	@echo diffh is made with diff in the subdirectory diff

forceit:
	@true
