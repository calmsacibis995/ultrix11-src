/* SCCSID: @(#)llib-lmall.c	3.0	4/22/86	*/

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* Based on:	@(#)llib-lmall.c	1.1	(System V)	*/
#include "malloc.h"
/*	Lint Library for Malloc(3x)	*/
/*	MALLOC(3X)	*/
/*	malloc, calloc, realloc and free are checked
/*	by the c library lint file
*/
int mallopt (cmd, value) int cmd, value; { return cmd+value; }
struct mallinfo mallinfo () { struct mallinfo s; return (s); }
