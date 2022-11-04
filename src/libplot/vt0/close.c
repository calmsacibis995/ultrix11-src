
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)close.c	3.0	4/22/86
 */
extern vti;
closevt(){
	close(vti);
}
closepl(){
	close(vti);
}
