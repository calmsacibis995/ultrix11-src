
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)move.c	3.0	4/22/86
 */
move(xi,yi){
		movep(xconv(xsc(xi)),yconv(ysc(yi)));
		return;
}
