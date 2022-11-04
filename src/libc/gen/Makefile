SCCSID = @(#)Makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# This is a dummy makefile. The real one, "makefile", is
# created automatically by the make in '..'.

SOURCES = abort.s abs.c atof.c atoi.c atol.c calloc.c crypt.c ctime.c \
	ctype_.c cuexit.s ecvt.c errlst.c execvp.c fakcu.s fakfp.s \
	frexp11.s getenv.c getlogin.c index.c isatty.c l3.c ldexp11.s \
	ldfps.s malloc.c mktemp.c modf11.s mon.c mpx.c nlist.c perror.c \
	pkon.c qsort.c rand.c rindex.c setjmp.s sleep.c strcat.c strcmp.c \
	strcpy.c strlen.c strncat.c strncmp.c strncpy.c stty.c swab.c \
	tell.c timezone.c ttyname.c ttyslot.c

sources: $(SOURCES)

.DEFAULT:
	sccs get $<
