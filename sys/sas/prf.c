/*
 * SCCSID: @(#)prf.c	3.0	5/12/86
 */
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * Scaled down version of C Library printf.
 * Only %s %u %d (==%u) %o %x %D %O are recognized.
 * Used to print diagnostic information
 * directly on console tty.
 * Since it is not interrupt driven,
 * all system activities are pretty much
 * suspended.
 * Printf should not be used for chit-chat.
 */

#include "sa_defs.h"

printf(fmt, x1)
register char *fmt;
unsigned x1;
{
	register c;
	register unsigned int *adx;
	char *s;

	adx = &x1;
loop:
	while((c = *fmt++) != '%') {
		if(c == '\0')
			return;
		putchar(c);
	}
	c = *fmt++;
	if(c == 'd' || c == 'u' || c == 'o' || c == 'x')
		printn((long)*adx, c=='o'? 8: (c=='x'? 16:10));
	else if(c == 's') {
		s = (char *)*adx;
		while(c = *s++)
			putchar(c);
	} else if (c == 'D') {
		printn(*(long *)adx, 10);
		adx += (sizeof(long) / sizeof(int)) - 1;
	} else if (c == 'O') {
		printn(*(long *)adx, 8);
		adx += (sizeof(long) / sizeof(int)) - 1;
	} else if (c == 'c')
		putchar((char *)*adx);
	adx++;
	goto loop;
}

/*
 * Print an unsigned integer in base b.
 */
printn(n, b)
long n;
{
	register long a;

	if (n<0) {	/* shouldn't happen */
		putchar('-');
		n = -n;
	}
	if(a = n/b)
		printn(a, b);
	putchar("0123456789ABCDEF"[(int)(n%b)]);
}



struct	device	{
	int	rcsr,rbuf;
	int	tcsr,tbuf;
};
struct	device	*KLADDR	{0177560};
putchar(c)
register c;
{
	register s;
	register unsigned timo;
	int chkc;

	if(KLADDR->rcsr&0200) {
		if((chkc = KLADDR->rbuf&0177) == '\023') {
			while(1){
				while((KLADDR->rcsr&0200)==0) ;
				if((chkc = KLADDR->rbuf&0177) == '\021')
					break;
			}
		}
	}
	timo = 60000;
	/*
	 * Try waiting for the console tty to come ready,
	 * otherwise give up after a reasonable time.
	 */
	while((KLADDR->tcsr&0200) == 0)
		if(--timo == 0)
			break;
	if(c == 0)
		return;
	s = KLADDR->tcsr;
	KLADDR->tcsr = 0;
	KLADDR->tbuf = c;
	if(c == '\n') {
		putchar('\r');
		putchar(0177);
		putchar(0177);
	}
	putchar(0);
	KLADDR->tcsr = s;
}

/*
 * Argument flag and buffer, see srt0.s and sa_defs.h.
 */
int	argflag;
char	argbuf[ARGSIZ-2];
char	*argp = &argbuf;
int	segflag;

getchar()
{
	register c, s;

#ifndef	BIGKERNEL
	if(argflag && (segflag!=2)) {	/* get input from arg buffer and */
#else	BIGKERNEL
	if(argflag && (segflag!=3)) {	/* get input from arg buffer and */
#endif	BIGKERNEL
		c = *argp++;		/* not in boot or sdload */
		goto notty;	/* no, ignore input */
	}
/*	KLADDR->rcsr = 1;	*/
	while((KLADDR->rcsr&0200)==0);
/*
	c = KLADDR->rbuf&0177;
 */
	if((c = KLADDR->rbuf&0177) == '\023'){
		while(1){
			while((KLADDR->rcsr&0200)==0);
			if((c = KLADDR->rbuf&0177) == '\021')
				break;
		}
		while((KLADDR->rcsr&0200)==0);
		c = KLADDR->rbuf&0177;
	}
notty:
	if (c=='\r')
		c = '\n';
	putchar(c);
	return(c);
}

gets(buf)
char	*buf;
{
register char *lp;
register c;
register lastc;

	lp = buf;
	lastc = 0;
	for (;;) {
		c = getchar() & 0177;
		if (c>='A' && c<='Z')
			c -= 'A' - 'a';
		if (lp != buf && *(lp-1) == '\\') {
			lp--;
			if (c>='a' && c<='z') {
				c += 'A' - 'a';
				goto store;
			}
			switch ( c) {
			case '(':
				c = '{';
				break;
			case ')':
				c = '}';
				break;
			case '!':
				c = '|';
				break;
			case '^':
				c = '~';
				break;
			case '\'':
				c = '`';
				break;
			}
		}
	store:
		switch(c) {
		case '\n':
		case '\r':
			c = '\n';
			*lp++ = '\0';
			return;
		case '\b':
		case '#':
		case 0177:	/* delete */
			if((lastc!='\b')&&(lastc!='#')&&(lastc!=0177))
				putchar('\\');
			lp--;
			putchar(*lp);
			if (lp < buf) {
				lp = buf;
				putchar('\n');
			}
			lastc = 0177;
			continue;
		case '@':
		case 025:	/* control U */
			lp = buf;
			putchar('\n');
			lastc = 0;
			continue;
		case 03:	   /* CTRL/C - ignored */
			continue;  /* help flush ^C from auto-boot cancel */
		default:
			*lp++ = c;
			lastc = c;
		}
	}
}
