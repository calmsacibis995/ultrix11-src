
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)pen.c	3.0	4/22/86
 */
pen(color)
int color;
{
	char pstrng[20];
	int col;

	switch(color){
		case 1:
		default:
			col = 7;
			break;
		case 2:
			col = 1;
			break;
		case 3:
			col = 4;
			break;
		case 4:
			col = 2;
			break;
	}
	sprintf(pstrng, "int,%d", col);
	wrtng(pstrng);
}
