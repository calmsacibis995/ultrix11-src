
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)line.c	3.0	4/22/86
 */
line(x0,y0,x1,y1){
	move(x0,y0);
	cont(x1,y1);
}
