/*
 * SCCSID: @(#)wrtng.c	3.0	4/22/86 
 */


/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*

 Facility: For the Plot package for REGIS Terminals

 Abstract: Write command to set up REGIS charateristics

 Author:  Jerry Brenner.

*/

#include <stdio.h>
extern	ggi;
extern	multi;

	/* Writing command */
	/* i is color, a is alternate (blink), s is area shading
	   n is negative writing, p is pattern, c is complement
	   e is erase, r is replace, v is overlay, m is vector offset multi */

	/* i can be a number from 0 to 7 or a color letter, or -1 for no cng.
	     (0)d = dark, (1)b = blue, (2)r = red, (3)m = magenta,
	     (4)g = green, (5)c = cyan, (6)y = yellow, (7)w = white

	   a = 0 to turn off blink, 1 to turn it on, -1 for no change

	   s = 0 to turn off shading, 1 to turn it on, -1 for no change
	       ascii shading character, or a shading reference.

	   n = 0 to turn off negative writing, 1 to turn it on, -1 for no change

	   p is a gigi defined pattern number or an 8 bit binary pattern

	   w is writing mode as follows
		   = 'c' to Complement write; XOR writing pattern and background

		   = 'e' to Erase memory to 0 during write operation

		   = 'r' to replace image memory with pattern

		   = 'v' to overlay write; OR writing pattern and image memory pattern

	   m = 0 for no vertor multiplier or a number for the multiplier */

struct wrtng {
	char i; char iv;
	char c0;
	char a; char av;
	char c1;
	char s; char sv;
	char c2;
	char n; char nv;
	char c3;
	char p; char pv;
	char c4;
	char w;
	char c5;
	char m; char mv[3];
}wrt={'i','7',',','a','0',',','s','0',','
	,'n','0',',','p','1',',','r',',','m',"001"};

char target[20], *indx, *argv;

/*	wrtng("int,red blink shading pat,1 replace mult,10"); */

wrtng(argp)
char *argp;
{
	argv = argp;
	while(sscanf(argv, "%s", &target) > 0)
	{
		argv += (strlen(&target))+1;
		if(strncmp("int", &target, 3) == 0
		  || strncmp("col", &target, 3) == 0)
		{	/* writing intensity (color) command */
			if((indx = index(&target, ',')) == 0)
				continue;	/* no comma seperator */
			indx++;
			switch(*indx)
			{
				case 'd':
				case '\0':
					wrt.iv = '0';	     /* dark */
					break;
				case 'b':		/* 'b'lue or 1 */
				case '\01':
					wrt.iv = '1';	     /* blue */
					break;
				case 'r':		/* 'r'ed or 2 */
				case '\02':
					wrt.iv = '2';	     /* red */
					break;
				case 'm':
				case '\03':
					wrt.iv = '3';	     /* magenta */
					break;
				case 'g':
				case '\04':
					wrt.iv = '4';	     /* green */
					break;
				case 'c':
				case '\05':
					wrt.iv = '5';	     /* cyan */
					break;
				case 'y':
				case '\06':
					wrt.iv = '6';	     /* yellow */
					break;
				case 'w':
				case '\07':
				default:
					wrt.iv = '7';	     /* white */
					break;
			}
			continue;
		}
		else if(strncmp("blink", &target, 5) == 0)
			wrt.av = '1';
		else if(strncmp("noblink", &target, 7) == 0)
			wrt.av = '0';
		else if(strncmp("alt", &target, 3) == 0)
		{
			if(indx = index(&target, ','))
			{
				indx++;
				switch(*indx)
				{
					case '0':
						wrt.av = '0';
						break;
					case '1':
						wrt.av = '1';
						break;
					default:
						break;
				}
			}
		}
		else if(strncmp("shade", &target, 5) == 0)
		{
			if(indx = index(&target, ','))
			{
				indx++;
				switch(*indx)
				{
					case '0':
						wrt.sv = '0';
						break;
					case '1':
						wrt.sv = '1';
						break;
					default:
						break;
				}
			}
			else
				wrt.sv = '1';
		}
		else if(strncmp("noshade", &target, 7) == 0
			|| strncmp("-shade", &target, 6) == 0)
			wrt.sv = '0';
		else if(strncmp("negat", &target, 5) == 0)
		{
			if(indx = index(&target, ','))
			{
				indx++;
				switch(*indx)
				{
					case '0':
						wrt.nv = '0';
						break;
					case '1':
						wrt.nv = '1';
						break;
					default:
						break;
				}
			}
		}
		else if(strncmp("nonegat", &target, 7) == 0
			|| strncmp("-negat", &target, 6) == 0)
			wrt.nv = '0';
		else if(strncmp("pat", &target, 3) == 0)
		{
			if(indx = index(&target, ','))
			{
				indx++;
				switch(*indx)
				{
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
						wrt.pv = *indx;
						break;
					default:
						break;
				}
			}
		}
		else if(strncmp("comp", &target, 4) == 0)
			wrt.w = 'c';
		else if(strncmp("eras", &target, 4)== 0)
			wrt.w = 'e';
		else if(strncmp("repl", &target, 4)== 0)
			wrt.w = 'r';
		else if(strncmp("over", &target, 4)== 0)
			wrt.w = 'v';
		else if(strncmp("mult", &target, 4) == 0
			|| strncmp("offs", &target, 4)== 0)
		{
			if(indx = index(&target, ','))
				strncpy(&wrt.mv[0], indx+1, 3);
			multi = atoi(&wrt.mv[0]);
		}
	}
	fprintf(ggi, "w(%s)\n", &wrt);
}

shade(s)
{
	if(s >= 0)
		wrt.sv = s+060;
	fprintf(ggi, "w(s%d)\n", s);
}


