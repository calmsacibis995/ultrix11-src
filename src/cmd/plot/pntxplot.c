
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)pntxplot.c	3.0	4/22/86";
/*
 * Reads standard graphics input
 * Makes a plot on a 792x792 printronix printer
 * (60 dpi x 72 dpi)
 *
 * Creates and leaves /usr/tmp/raster (1000 blocks)
 * which is the bitmap
 */
#include "stdio.h"
#include <signal.h>

/*
 * MAXX and MAXY should be multiples of 8.
 */
#define BSIZ	512
#define	MAXY	792		/* # of dots horizontally */
#define	MAXX	792		/* # of dots vertically */
#define	X_TO_Y	(5.0/6.0)	/* ratio: (dots per inch horz)/(dpi vert) */
#define	NB	80		/* number of incore buffers */

#define	BPL	MAXX/8		/* bytes per line */
#define	LPB	(BSIZ/NB)
#define	END	(MAXY/LPB)
#define	SLOP	(BSIZ-(LPB*BSIZ))

#define	mapx(x)	((MAXX*X_TO_Y*((x)-botx))/del)
#define	mapy(y)	(MAXY*(topy-(y))/del)
#define SOLID -1
#define DOTTED 014
#define SHORTDASHED 034
#define DOTDASHED 054
#define LONGDASHED 074
#define	SETSTATE	(('v'<<8)+1)

int	linmod	= SOLID;
int	again;
int	done1;
char	chrtab[][16];
char	blocks	[NB][BSIZ];
int	lastx;
int	lasty;
double	topy	= MAXY;
double	botx	= 0;
double	del	= 1;

struct	buf {
	int	bno;
	char	*block;
};
struct	buf	bufs[NB];

int	in, out;
char *picture = "/usr/tmp/raster";

main(argc, argv)
char **argv;
{
	extern int onintr();
	register i;

	if (argc>1) {
		in = open(argv[1], 0);
		putpict();
		exit(0);
	}
	signal(SIGTERM, onintr);
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, onintr);
another:
	for (i=0; i<NB; i++) {
		bufs[i].bno = -1;
		bufs[i].block = blocks[i];
	}
	out = creat(picture, 0666);
	in = open(picture, 0);
	blkseek(out, END);
	write(out, blocks[0], BSIZ);
/*delete following code when filsys deals properly with
holes in files*/
	for(i=0;i<512;i++)
		blocks[0][i] = 0;
	blkseek(out, 0);
	for(i=0;i<END;i++)
		write(out,blocks[0],BSIZ-SLOP);
/**/
	getpict();
	for (i=0; i<NB; i++)
		if (bufs[i].bno != -1) {
			blkseek(out, bufs[i].bno);
			write(out, bufs[i].block, BSIZ-SLOP);
		}
	putpict();
	if (again) {
		close(in);
		close(out);
		goto another;
	}
	exit(0);
}

getpict()
{
	register x1, y1;
	double topx, boty;

	again = 0;
	for (;;) switch (x1 = getc(stdin)) {

	case 's':
		botx = getw(stdin);
		boty = getw(stdin);
		topx = getw(stdin);
		topy = getw(stdin);
		if ((topy-boty)/(topx-botx) > (MAXY/(MAXX/X_TO_Y))) {
			del = MAXY/(topy-boty);
		} else {
			del = (MAXX/X_TO_Y)/(topx-botx);
			topy = MAXY/del - boty;
		}
		continue;

	case 'l':
		done1 |= 01;
		x1 = mapx(getw(stdin));
		y1 = mapy(getw(stdin));
		lastx = mapx(getw(stdin));
		lasty = mapy(getw(stdin));
		line(x1, y1, lastx, lasty);
		continue;

	case 'm':
		lastx = mapx(getw(stdin));
		lasty = mapy(getw(stdin));
		continue;

	case 't':
		done1 |= 01;
		while ((x1 = getc(stdin)) != '\n')
			plotch(x1);
		continue;

	case 'e':
		if (done1) {
			again++;
			return;
		}
		continue;

	case 'p':
		done1 |= 01;
		lastx = mapx(getw(stdin));
		lasty = mapy(getw(stdin));
		point(lastx, lasty);
		point(lastx+1, lasty);
		point(lastx, lasty+1);
		point(lastx+1, lasty+1);
		continue;

	case 'n':
		done1 |= 01;
		x1 = mapx(getw(stdin));
		y1 = mapy(getw(stdin));
		line(lastx, lasty, x1, y1);
		lastx = x1;
		lasty = y1;
		continue;

	case 'f':
		getw(stdin);
		getc(stdin);
		switch(getc(stdin)) {
		case 't':
			linmod = DOTTED;
			break;
		default:
		case 'i':
			linmod = SOLID;
			break;
		case 'g':
			linmod = LONGDASHED;
			break;
		case 'r':
			linmod = SHORTDASHED;
			break;
		case 'd':
			linmod = DOTDASHED;
			break;
		}
		while((x1=getc(stdin))!='\n')
			if(x1==-1) return;
		continue;

	case 'd':
		getw(stdin);
		getw(stdin);
		getw(stdin);
		x1 = getw(stdin);
		while (--x1 >= 0)
			getw(stdin);
		continue;

	case -1:
		return;

	default:
		printf("Botch\n");
		return;
	}
}

plotch(c)
register c;
{
	register j;
	register char *cp;
	int i;

	if (c<' ' || c >0177)
		return;
	cp = chrtab[c-' '];
	for (i = -16; i<16; i += 2) {
		c = *cp++;
		for (j=7; j>=0; --j)
			if ((c>>j)&1) {
				point(lastx+6-j*2, lasty+i);
				point(lastx+7-j*2, lasty+i);
				point(lastx+6-j*2, lasty+i+1);
				point(lastx+7-j*2, lasty+i+1);
			}
	}
	lastx += 16;
}

int	f = 1; /* output file desrciptor */
char	ibuf[BPL];
char	obuf[134];
putpict()
{
	register x;
	register char *ip, *op;
	int y;

/*
	if (f==0){
		f = open("/dev/rlp", 1);
		if (f < 0) {
			printf("Cannot open rlp\n");
			exit(1);
		}
	}
*/
	lseek(in, 0L, 0);
	for (y=0; y<MAXY; y++) {
		read(in, ibuf, BPL);
		op = obuf;
		/*
		 * the 0100 bit is ignored, we set it so
		 * that we don't generate any control codes.
		 */
		for (ip = ibuf; ip < &ibuf[BPL]; ip += 3)  {
			*op++ = 0100|((ip[0]>>2)&077);
			*op++ = 0100|((ip[0]<<4)&060)|((ip[1]>>4)&017);
			*op++ = 0100|((ip[1]<<2)&074)|((ip[2]>>6)&003);
			*op++ = 0100|(ip[2]&077);
		}
		obuf[132] = '\05';
		obuf[133] = '\012';
		write(f, obuf, 134);
	}
}

line(x0, y0, x1, y1)
register x0, y0;
{
	int dx, dy;
	int xinc, yinc;
	register res1;
	int res2;
	int slope;

	xinc = 1;
	yinc = 1;
	if ((dx = x1-x0) < 0) {
		xinc = -1;
		dx = -dx;
	}
	if ((dy = y1-y0) < 0) {
		yinc = -1;
		dy = -dy;
	}
	slope = xinc*yinc;
	res1 = 0;
	res2 = 0;
	if (dx >= dy) while (x0 != x1) {
		if((x0+slope*y0)&linmod)
			point(x0, y0);
		if (res1 > res2) {
			res2 += dx - res1;
			res1 = 0;
			y0 += yinc;
		}
		res1 += dy;
		x0 += xinc;
	} else while (y0 != y1) {
		if((x0+slope*y0)&linmod)
			point(x0, y0);
		if (res1 > res2) {
			res2 += dy - res1;
			res1 = 0;
			x0 += xinc;
		}
		res1 += dx;
		y0 += yinc;
	}
	if((x1+slope*y1)&linmod)
		point(x1, y1);
}

point(x, y)
register x, y;
{
	register bno;

	if (x < 0 || x > MAXX || y < 0 || y >MAXY)
		return;
	bno = (y%LPB);
	if (bno != bufs[0].bno) {
		if (bno < 0 || bno >= END)
			return;
		getblk(bno);
	}
	bufs[0].block[((y%LPB)*BPL)+(x&~07)>>3] |= 1 << (7-(x&07));
}

getblk(b)
register b;
{
	register struct buf *bp1, *bp2;
	register char *tp;

loop:
	for (bp1 = bufs; bp1 < &bufs[NB]; bp1++) {
		if (bp1->bno == b || bp1->bno == -1) {
			tp = bp1->block;
			for (bp2 = bp1; bp2>bufs; --bp2) {
				bp2->bno = (bp2-1)->bno;
				bp2->block = (bp2-1)->block;
			}
			bufs[0].bno = b;
			bufs[0].block = tp;
			return;
		}
	}
	blkseek(out, bufs[NB-1].bno);
	write(out, bufs[NB-1].block, BSIZ-SLOP);
	blkseek(in, b);
	read(in, bufs[NB-1].block, BSIZ-SLOP);
	bufs[NB-1].bno = b;
	goto loop;
}

onintr()
{
	exit(1);
}

blkseek(a, b)
{
	return(lseek(a, (long)b*(BSIZ-SLOP), 0));
}
