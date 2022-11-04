
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)open.c	3.0	4/22/86
 */
int xnow;
int ynow;
float boty 0.;
float botx 0.;
float oboty 0.;
float obotx 0.;
float scalex 1.;
float scaley 1.;
int vti -1;

openvt ()
{
		vti = open("/dev/vt0",1);
		return;
}
openpl()
{
	vti = open("/dev/vt0",1);
	return;
}
