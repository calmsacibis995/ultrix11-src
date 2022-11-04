
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifdef KEEPSCCSID
static char Sccsid[] = "@(#)error.c	3.0	4/21/86";
#endif
#include "defs"

#ifndef DOSTRINGS	/* was OVOPT */

warn1(s,t)
char *s, *t;
{
char buff[100];
sprintf(buff, s, t);
warn(buff);
}


warn(s)
char *s;
{
if(nowarnflag)
	return;
fprintf(diagfile, "Warning on line %d of %s: %s\n", lineno, infname, s);
++nwarn;
}


errstr(s, t)
char *s, *t;
{
char buff[100];
sprintf(buff, s, t);
err(buff);
}



erri(s,t)
char *s;
int t;
{
char buff[100];
sprintf(buff, s, t);
err(buff);
}


err(s)
char *s;
{
fprintf(diagfile, "Error on line %d of %s: %s\n", lineno, infname, s);
++nerr;
}


yyerror(s)
char *s;
{ err(s); }



dclerr(s, v)
char *s;
struct Nameblock *v;
{
char buff[100];

if(v)
	{
	sprintf(buff, "Declaration error for %s: %s", varstr(VL, v->varname), s);
	err(buff);
	}
else
	errstr("Declaration error %s", s);
}



execerr(s, n)
char *s, *n;
{
char buf1[100], buf2[100];

sprintf(buf1, "Execution error %s", s);
sprintf(buf2, buf1, n);
err(buf2);
}


fatal(t)
char *t;
{
fprintf(diagfile, "Compiler error line %d of %s: %s\n", lineno, infname, t);
fflush(diagfile);
if(debugflag)
	abort();
done(3);
exit(3);
}




fatalstr(t,s)
char *t, *s;
{
char buff[100];
sprintf(buff, t, s);
fatal(buff);
}



fatali(t,d)
char *t;
int d;
{
char buff[100];
sprintf(buff, t, d);
fatal(buff);
}


many(s, c)
char *s, c;
{
char buff[25];	/* ??? */

sprintf(buff, "Too many %s.  Try the -N%c option", s, c);
fatal(buff);
}


err66(s)
char *s;
{
errstr("Fortran 77 feature used: %s", s);
}



errext(s)
char *s;
{
errstr("F77 Compiler extension used: %s", s);
}
#else DOSTRINGS		/* was OVOPT */

char efilname[] = "/usr/lib/f77_strings";
int  efil = -1;


error(index,type,a,b)
int index;
{
	char buf[101];
	long lseek();
	struct Nameblock *v;

	if (efil < 0) {
		efil = open(efilname, 0);
		if (efil < 0) {
			badfile("cannot open");
		}
	}
	if (lseek(efil, (long) index, 0) < 0) {
		badfile("Bad seek in");
	}

	if (read(efil, buf, 100) <= 0) {
		badfile("Bad read in");
	}

	/* 
	 * This next is not really an error, just a way
	 * of extracting the debug statements into the
	 * external f77_strings file so they don't take up
	 * valuable data space.
	 */
	if(type == E_DEBUG){
		fprintf(diagfile,buf,a,b);
		return;
	}

	/* else find out what type of error: */
	else if(type == E_WARN){
		if(nowarnflag)
			return;
	/* fprintf(diagfile, "Warning on line %d of %s: ",lineno, infname); */
		saywhere("Warning on");
		++nwarn;
	}

	else if(type == E_ERREXT){
		Lineno(0);
		fprintf(diagfile, "F77 Compiler extension used: ");
		++nerr;
	}

	else if(type == E_ERR){
		Lineno(0);
		++nerr;
	}

	else if(type == E_MANY) {
		if (debugflag)
			debug_table();	/* static table space used so far */
		Lineno(0);
		fprintf(diagfile, "Too many %s. Try the -N%c option", buf,a);
		goto fatalout;
	}

	else if(type == E_FATAL) {
		Lineno(1);
	}

	else if(type == E_EXECERR) {
		Lineno(0);
		fprintf(diagfile, "Execution error ");
		++nerr;
	}

	else if(type == E_DCLERR) {
		Lineno(0);
		fprintf(diagfile, "Declaration error");
		if(v = a) {
		/*	fprintf(diagfile, "Declaration error for %s: ", */
			fprintf(diagfile, " for %s: ",
				varstr(VL, v->varname));
		}
		else
		/*	fprintf(diagfile, "Declaration error: "); */
			fprintf(diagfile, ": ");
	}

	else if(type == E_ERR66) {
		Lineno(0);
		fprintf(diagfile, "Fortran 77 feature used: ");
	}
	else {
		Lineno(0);
		fprintf(diagfile, "unrecognized error type: (%d)\n", type);
		exit(1);
	}
	fprintf(diagfile,buf,a);
fatalout:
	fprintf(diagfile,"\n");
	if(type == E_FATAL || type == E_MANY) {
		if(debugflag)
			abort();
		exit(3);
	}
	return(0);	/* OK */
}

/*
 * Print the offending line number and filename
 */
Lineno(n)
{
    if (n == 0) {
    /* PROGRAM ERROR */
/*  fprintf(diagfile, "Error on line %d of %s: ",lineno,infname); */
	saywhere("Error on");
    }
    else if (n == 1) {
    /* FATAL COMPILER ERROR */
/*  fprintf(diagfile, "F77 Compiler error line %d of %s: ",lineno,infname); */
	saywhere("F77 Compiler error");
    }
    return;
}

saywhere(s)
char *s;
{
	fprintf(diagfile,"%s line %d of %s: ", s, lineno, infname);
}

/*
 * Print message like "Bad seek in /usr/lib/f77_strings, cannot...", and exit. 
 */
badfile(s)
char *s;
{
	fprintf(stderr, "%s %s, cannot get error message string!\n",s,efilname);
	exit(1);
}
#endif DOSTRINGS	/* was OVOPT */
