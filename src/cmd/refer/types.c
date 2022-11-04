
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
	SCCSID: @(#)types.c	3.0	4/22/86
 */
# if LONG
# define ptr long
# define uptr long
# define getp getl
# define putp putl
# define MONE -1L
extern long getl();
# else
# define ptr int
# define uptr unsigned
# define getp getw
# define putp putw
# define MONE -1
# endif
