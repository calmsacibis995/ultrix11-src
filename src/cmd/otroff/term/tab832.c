
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)tab832.c	3.0	4/22/86
 */
#define INCH 240
/*
Anderson Jacobson 832
nroff driving tables
width and code tables
*/

struct {
	int bset;
	int breset;
	int Hor;
	int Vert;
	int Newline;
	int Char;
	int Em;
	int Halfline;
	int Adj;
	char *twinit;
	char *twrest;
	char *twnl;
	char *hlr;
	char *hlf;
	char *flr;
	char *bdon;
	char *bdoff;
	char *ploton;
	char *plotoff;
	char *up;
	char *down;
	char *right;
	char *left;
	char *codetab[256-32];
	int zzz;
	} t {
/*bset*/	0,
/*breset*/	0177420,
/*Hor*/		INCH/60,
/*Vert*/	INCH/48,
/*Newline*/	INCH/6,
/*Char*/	INCH/10,
/*Em*/		INCH/10,
/*Halfline*/	INCH/12,
/*Adj*/		INCH/10,
/*twinit*/	"\033N",
/*twrest*/	"\033N",
/*twnl*/	"\015\n",
/*hlr*/		"\0338",
/*hlf*/		"\0339",
/*flr*/		"\0337",
/*bdon*/	"",
/*bdoff*/	"",
/*ploton*/	"\033P",
/*plotoff*/	"\033N",
/*up*/		"\013",
/*down*/	"\n",
/*right*/	" ",
/*left*/	"\b",
/*codetab*/
#include "code.300"
