
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/
#
static char *sccsid = "@(#)output.c 3.0 4/21/86";
/*
 *
 *	UNIX debugger
 *
 */

#include "defs.h"

int 	mkfault;
int 	infile;
int 	outfile = 1;
int 	maxpos;
char 	printbuf[MAXLIN];
char 	*printptr = printbuf;
char 	*digitptr;

eqstr(s1, s2)
register char *s1, *s2;
{
	register char *es1;
	es1 = s1+8;
	while(*s1++ == *s2){
		if(*s2++ == 0 || s1>=es1)
		{
			return(1);
		}
	}
	return(0);
}

length(s)
char *s;
{
	int 	n = 0;
	while(*s++){
		n++; 
	}
	return(n);
}

printc(c)
char 	c;
{
	char 	d;
	char *q;
	int 	posn, tabs, p;

	if(mkfault)
	{
		return;
	}
	else if((*printptr=c)=='\n')
	{
		tabs=0; 
		posn=0; 
		q=printbuf;
		for(p=0; p<printptr-printbuf; p++){
			d=printbuf[p];
			if((p&7)==0 && posn)
			{
				tabs++; 
				posn=0;
			}
			if(d==SP)
			{
				posn++;
			}
			else{
				WHILE tabs>0){
					*q++=TB; 
					tabs--; 
				}
				while(posn>0){
					*q++=SP; 
					posn--; 
				}
				*q++=d;
			}
		}
		*q++='\n';
		write(outfile,printbuf,q-printbuf);
		printptr=printbuf;
	}
	else if(c==TB)
	{
		*printptr++=SP;
		while((printptr-printbuf)&7){
			*printptr++=SP; 
		}
	}
	else if(c)
	{
		printptr++;
	}
}

charpos()
{	
	return(printptr-printbuf);
}

flushbuf()
{	
	if(printptr!=printbuf)
	{
		printc('\n');
	}
}

printf(fmat,a1)
char *fmat;
char **a1;
{
	char *fptr, *s;
	int 	*vptr;
	long 	*dptr;
	double 	*rptr;
	int 	width, prec;
	char 	c, adj;
	int 	x, decpt, n;
	long 	lx;
	char 	digits[64];

	fptr = fmat; 
	vptr = &a1;

	while(c = *fptr++){
		if(c!='%')
		{
			printc(c);
		}
		else{
			if(*fptr=='-')
			{
				adj='l'; 
				fptr++; 
			}
			else{
				adj='r';
			}
			width=convert(&fptr);
			if(*fptr=='.')
			{
				fptr++; 
				prec=convert(&fptr); 
			}
			else{
				prec = -1;
			}
			digitptr=digits;
			dptr=rptr=vptr; 
			lx = *dptr; 
			x = *vptr++;
			s=0;
			switch (c = *fptr++){

			case 'd':
			case 'u':
				printnum(x,c,10); 
				break;
			case 'o':
				printoct(0,x,0); 
				break;
			case 'q':
				lx=x; 
				printoct(lx,-1); 
				break;
			case 'x':
				printdbl(0,x,c,16); 
				break;
			case 'Y':
				printdate(lx); 
				vptr++; 
				break;
			case 'D':
			case 'U':
				printdbl(lx,c,10); 
				vptr++; 
				break;
			case 'O':
				printoct(lx,0); 
				vptr++; 
				break;
			case 'Q':
				printoct(lx,-1); 
				vptr++; 
				break;
			case 'X':
				printdbl(lx,'x',16); 
				vptr++; 
				break;
			case 'c':
				printc(x); 
				break;
			case 's':
				s=x; 
				break;
			case 'f':
			case 'F':
				vptr += 7;
				s=ecvt(*rptr, prec, &decpt, &n);
				*digitptr++=(n?'-':'+');
				*digitptr++ = (decpt<=0 ? '0' : *s++);
				if(decpt>0)
				{
					decpt--;
				}
				*digitptr++ = '.';
				while(*s && prec--){
					*digitptr++ = *s++; 
				}
				while(*--digitptr=='0');

				digitptr += (digitptr-digits>=3 ? 1 : 2);
				if(decpt)
				{
					*digitptr++ = 'e'; 
					printnum(decpt,'d',10);
				}
				s=0; 
				prec = -1; 
				break;
			case 'm':
				vptr--; 
				break;
			case 'M':
				width=x; 
				break;
			case 'T':
			case 't':
				if(c=='T')
				{
					width=x;
				}
				else{
					vptr--;
				}
				if(width)
				{
					width -= charpos()%width;
				}
				break;
			default:
				printc(c); 
				vptr--;
			}
			if(s==0)
			{
				*digitptr=0; 
				s=digits;
			}
			n=length(s);
			n=(prec<n && prec>=0 ? prec : n);
			width -= n;
			if(adj=='r')
			{
				while(width-- > 0){
					printc(SP); 
				}
			}
			while(n--){
				printc(*s++); 
			}
			while(width-- > 0){
				printc(SP); 
			}
			digitptr=digits;
		}
	}
}

printdate(tvec)
long 	tvec;
{
	register int 	i;
	register char *timeptr;
	timeptr = ctime(&tvec);
	for(i=20; i<24; i++){
		*digitptr++ = *(timeptr+i); 
	}
	for(i=3; i<19; i++){
		*digitptr++ = *(timeptr+i); 
	}
} /*printdate*/

prints(s)
char *s;
{	
	printf("%s",s);
}

newline()
{
	printc('\n');
}

convert(cp)
register char **cp;
{
	register char c;
	int 	n;
	n=0;
	while(((c = *(*cp)++)>='0') && (c<='9')){
		n=n*10+c-'0'; 
	}
	(*cp)--;
	return(n);
}

printnum(n,fmat,base)
register int 	n;
{
	register char k;
	register int 	*dptr;
	int 	digs[15];
	dptr=digs;
	if(n<0 && fmat=='d')
	{
		n = -n; 
		*digitptr++ = '-';
	}
	while(n){
		*dptr++ = ((POS)n)%base;
		n=((POS)n)/base;
	}
	if(dptr==digs)
	{
		*dptr++=0;
	}
	while(dptr!=digs){
		k = *--dptr;
		*digitptr++ = (k+(k<=9 ? '0' : 'a'-10));
	}
}

printoct(o,s)
long 	o;
int 	s;
{
	int 	i;
	long 	po = o;
	char 	digs[12];

	if(s)
	{
		if(po<0)
		{
			po = -po; 
			*digitptr++='-';
		}
		else{
			if(s>0)
			{
				*digitptr++='+';
			}
		}
	}
	for(i=0;i<=11;i++){
		digs[i] = po&7; 
		po >>= 3; 
	}
	digs[10] &= 03; 
	digs[11]=0;
	for(i=11;i>=0;i--){
		if(digs[i])
		{
			break; 
		}
	}
	for(i++;i>=0;i--){
		*digitptr++=digs[i]+'0'; 
	}
}

printdbl(lx,ly,fmat,base)
int lx, ly; 
char fmat; 
int base;
{	
	int digs[20]; 
	int *dptr; 
	char k;
	double f ,g; 
	long q;
	dptr=digs;
	if(fmat!='D')
	{
		f=leng(lx); 
		f *= itol(1,0); 
		f += leng(ly);
		if(fmat=='x')
		{
			*digitptr++='#';
		}
	}
	else{
		f=itol(lx,ly);
		if(f<0)
		{
			*digitptr++='-'; 
			f = -f;
		}
	}
	while(f){
		q=f/base; 
		g=q;
		*dptr++ = f-g*base;
		f=q;
	}
	if(dptr==digs)
	{
		*dptr++=0;
	}
	while(dptr!=digs){
		k = *--dptr;
		*digitptr++ = (k+(k<=9 ? '0' : 'a'-10));
	}
}

iclose()
{
	if(infile)
	{
		close(infile); 
		infile=0;
	}
}

oclose()
{
	if(outfile!=1)
	{
		flushbuf(); 
		close(outfile); 
		outfile=1;
	}
}

endline()
{
	if(charpos()>=maxpos)
	{
		printf("\n");
	}
}
