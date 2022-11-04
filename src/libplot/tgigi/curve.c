
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)curve.c	3.0	4/22/86
 */
#include <stdio.h>
extern ggi;

curve(pnt, cnt)
struct{int x; int y} *pnt;
int cnt;
{
	int num,nx,ny;

	fprintf(ggi,"c(s)");
	for(num = 0; num < cnt; num++) {
		nx = xsc(pnt[num].x);
		ny = ysc(pnt[num].y);
		fprintf(ggi,"[%d,%d]\n", nx, ny);
	}
	fprintf(ggi,"[](e)\n");
}
