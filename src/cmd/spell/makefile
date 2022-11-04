# SCCSID: @(#)makefile	3.0	4/24/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

SOURCES = spell.h spell.c spell.sh spellin.c spellout.c stop

all:	all40 all70

sources: ${SOURCES}

${SOURCES} american british local:
	 sccs get $@

all40:	spell.sh spell40 hlist40 hlista40 hlistb40 hstop40 spellin40 spellout40

install: cp70 cp40 
	make clean

cp40:	all40 cpcom
	cp spell40 ${DESTDIR}/usr/lib/spell40
	-strip ${DESTDIR}/usr/lib/spell40
	chmod 755 ${DESTDIR}/usr/lib/spell40
	chog bin ${DESTDIR}/usr/lib/spell40
	cp spellin40 ${DESTDIR}/usr/lib/spellin40
	-strip ${DESTDIR}/usr/lib/spellin40
	chmod 755 ${DESTDIR}/usr/lib/spellin40
	chog bin ${DESTDIR}/usr/lib/spellin40
	cp spellout40 ${DESTDIR}/usr/lib/spellout40
	chmod 755 ${DESTDIR}/usr/lib/spellout40
	chog bin ${DESTDIR}/usr/lib/spellout40
	-strip ${DESTDIR}/usr/lib/spellout40
	cp hlist40 ${DESTDIR}/usr/dict/hlist40
	chmod 644 ${DESTDIR}/usr/dict/hlist40
	chog bin ${DESTDIR}/usr/dict/hlist40
	cp hlista40 ${DESTDIR}/usr/dict/hlista40
	chmod 644 ${DESTDIR}/usr/dict/hlista40
	chog bin ${DESTDIR}/usr/dict/hlista40
	cp hlistb40 ${DESTDIR}/usr/dict/hlistb40
	chmod 644 ${DESTDIR}/usr/dict/hlistb40
	chog bin ${DESTDIR}/usr/dict/hlistb40
	cp hstop40 ${DESTDIR}/usr/dict/hstop40
	chmod 644 ${DESTDIR}/usr/dict/hstop40
	chog bin ${DESTDIR}/usr/dict/hstop40

${DESTDIR}/usr/dict/words: /usr/dict/words
	-cp /usr/dict/words ${DESTDIR}/usr/dict/words
	chmod 444 ${DESTDIR}/usr/dict/words
	chog bin ${DESTDIR}/usr/dict/words

cmp40:	spell40
	cmp spell40 ${DESTDIR}/usr/lib/spell40

spell40: spell.c spell.h
	cc -Dsmall -O spell.c -o spell40 -lv7

spellin40: spellin.c
	cc -Dsmall -O spellin.c -o spellin40

spellout40: spellout.c
	cc -Dsmall -O spellout.c -o spellout40

hlist40: ${DESTDIR}/usr/dict/words spellin40
	spellin40 <${DESTDIR}/usr/dict/words >hlist40

hlista40: american local hlist40 spellin40
	(cat american local)|spellin40 hlist40 >hlista40

hlistb40: british local hlist40 spellin40
	(cat british local)|spellin40 hlist40 >hlistb40

hstop40: stop spellin40
	spellin40 <stop >hstop40

all70:	spell.sh spell70 hlist70 hlista70 hlistb70 hstop70 spellin70 spellout70

cp70:	all70 cpcom
	cp local american british stop ${DESTDIR}/usr/dict
	chmod 666 ${DESTDIR}/usr/dict/local
	chmod 444 ${DESTDIR}/usr/dict/american
	chmod 444 ${DESTDIR}/usr/dict/british
	chmod 444 ${DESTDIR}/usr/dict/stop
	cp spell70 ${DESTDIR}/usr/lib/spell70
	-strip ${DESTDIR}/usr/lib/spell70
	chmod 755 ${DESTDIR}/usr/lib/spell70
	chog bin ${DESTDIR}/usr/lib/spell70
	cp spell70 ${DESTDIR}/usr/lib/spell
	-strip ${DESTDIR}/usr/lib/spell
	chmod 755 ${DESTDIR}/usr/lib/spell
	chog bin ${DESTDIR}/usr/lib/spell
	cp spellin70 ${DESTDIR}/usr/lib/spellin70
	-strip ${DESTDIR}/usr/lib/spellin70
	chmod 755 ${DESTDIR}/usr/lib/spellin70
	chog bin ${DESTDIR}/usr/lib/spellin70
	cp spellin70 ${DESTDIR}/usr/lib/spellin
	-strip ${DESTDIR}/usr/lib/spellin
	chmod 755 ${DESTDIR}/usr/lib/spellin
	chog bin ${DESTDIR}/usr/lib/spellin
	cp spellout70 ${DESTDIR}/usr/lib/spellout70
	-strip ${DESTDIR}/usr/lib/spellout70
	chmod 755 ${DESTDIR}/usr/lib/spellout70
	chog bin ${DESTDIR}/usr/lib/spellout70
	cp spellout70 ${DESTDIR}/usr/lib/spellout
	-strip ${DESTDIR}/usr/lib/spellout
	chmod 755 ${DESTDIR}/usr/lib/spellout
	chog bin ${DESTDIR}/usr/lib/spellout
	cp hlist70 ${DESTDIR}/usr/dict/hlist70
	chmod 644 ${DESTDIR}/usr/dict/hlist70
	chog bin ${DESTDIR}/usr/dict/hlist70
	cp hlista70 ${DESTDIR}/usr/dict/hlista70
	chmod 644 ${DESTDIR}/usr/dict/hlista70
	chog bin ${DESTDIR}/usr/dict/hlista70
	cp hlista70 ${DESTDIR}/usr/dict/hlista
	chmod 644 ${DESTDIR}/usr/dict/hlista
	chog bin ${DESTDIR}/usr/dict/hlista
	cp hlistb70 ${DESTDIR}/usr/dict/hlistb70
	chmod 644 ${DESTDIR}/usr/dict/hlistb70
	chog bin ${DESTDIR}/usr/dict/hlistb70
	cp hlistb70 ${DESTDIR}/usr/dict/hlistb
	chmod 644 ${DESTDIR}/usr/dict/hlistb
	chog bin ${DESTDIR}/usr/dict/hlistb
	cp hstop70 ${DESTDIR}/usr/dict/hstop70
	chmod 644 ${DESTDIR}/usr/dict/hstop70
	chog bin ${DESTDIR}/usr/dict/hstop70
	cp hstop70 ${DESTDIR}/usr/dict/hstop
	chmod 644 ${DESTDIR}/usr/dict/hstop
	chog bin ${DESTDIR}/usr/dict/hstop

cpcom:	spell.sh
	cp spell.sh ${DESTDIR}/usr/bin/spell
	chmod 755 ${DESTDIR}/usr/bin/spell
	chog bin ${DESTDIR}/usr/bin/spell

cmp70:	spell70
	cmp spell70 /usr/lib/spell70

spell70: spell.c spell.h
	cc -i -O spell.c -o spell70

spellin70: spellin.c
	cc -i -O spellin.c -o spellin70

spellout70: spellout.c
	cc -i -O spellout.c -o spellout70

hlist70: ${DESTDIR}/usr/dict/words  spellin70
	spellin70 <${DESTDIR}/usr/dict/words >hlist70

hlista70: american local hlist70 spellin70
	(cat american local)|spellin70 hlist70 >hlista70

hlistb70: british local hlist70 spellin70
	(cat british local)|spellin70 hlist70 >hlistb70

hstop70: stop spellin70
	spellin70 <stop >hstop70
clean:
	rm -f *.o log core a.out
	rm -f hlist40 hlist70 hlista40 hlista70 hlistb40 hlistb70 hstop40 \
		hstop70 spell40 spell70 spellin40 spellin70 spellout40 \
		spellout70

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
