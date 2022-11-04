
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static	char	*sccsid = "@(#)odm.c	3.0	(ULTRIX)	4/21/86";
#endif	lint
  
/**/
/*
 *
 *	File name:
 *
 *	    odm.c
 *
 *	Source file description:
 *
 *		This file contains the various routines used for
 *		Output Device Management by the Labeled Tape Facility (LTF).
 *
 *		It is used by the LTF in order to centralize those routines
 *		which are "system" dependant. ie. Ultrix-11 systems do not
 *		provide the same set of i/o control systems calls as does
 *		an Ultrix-32 system. 
 *
 *
 *	Functions:
 *
 *		ceot()		Check if hit end of tape
 *		bsf()		Backspace 'n' tape/file mark(s)
 *		dfsf()		Dummy fsf routine for non-tape
 *				devices..
 *		fsf()		Forwardspace 'n' tape/file mark(s)
 *		rew()		Rewinds the output device
 *		tape()		Determine if device is tape or not
 *		tape_mark()	Determines if a dummy tape mark
 *				was detected
 *		weof()		Write an eof on output device
 *				(eof === a tape mark)
 *
 *
 *	Usage:
 *
 *		n/a
 *
 *	Compile:
 *
 *	    cc -O -c odm.c		<- For Ultrix-32/32m
 *
 *	    cc CFLAGS=-DU11-O odm.c	<- For Ultrix-11
 *
 *
 *	Modification history:
 *	~~~~~~~~~~~~~~~~~~~~
 *
 *	revision			comments
 *	--------	-----------------------------------------------
 *	  01.0		09-Apr-85	Ray Glaser
 *			Create orginal version.
 *	
 */
  
/**/
/*
 * ->	 Defintions required  of / for  this routine
 */
  
#define	ODMC	/* Flag our compile so that our routines are not declared
		 * external by the include file "ltfefs.h". */
/* 
 * ->	Local includes
 */
  
#include "ltfdefs.h"	/* Common GLOBAL definitions */
  
/*
 * ->	Module GLOBALS
 */
  
struct	mtop	Mtop;	/* Mag tape operation  ioctl  structure */
  
/**/
/*
 *
 * Function:
 *
 *	ceot
 *
 * Function Description:
 *
 *	Check if end of tape encountered when receive errno
 *	from a read or write = -1.  
 *
 * Arguments:
 *
 *	none
 *
 * Return values:
 *
 *	none
 *
 * Side Effects:
 *
 *	If eot is encountered, an error message is output 
 *	to stderr & the routine exits back to system control.
 *	If not eot, exits anyways due to read or write error.
 */
  
ceot()
{

/* 
 * ->	Local includes
 */

struct	mtget Mtstat;		/* ioctl status structure */
  
if (Tape) {
    if (ioctl(fileno(Magtfp), MTIOCGET, &Mtstat) < 0) {
	PERROR "\n%s: %s %s\n", Progname, CANTCGET, Magtdev);
	perror(Magtdev);
	exit(FAIL);
    }
    else
	if (Mtstat.mt_softstat & MT_EOT) {
	    PERROR "\n%s: %s %s\n", Progname, ERREOT, Magtdev);
	    perror(Magtdev);
	    exit(FAIL);
	}
}
exit(FAIL);
}/*E ceot() */
/**/
/*
 *
 * Function:
 *
 *	bsf
 *
 * Function Description:
 *
 *	Backspace the output device 'n' file mark(s).
 *
 * Arguments:
 *
 *	int	n	Number of tape/file mark(s) to backspace.
 *
 * Return values:
 *
 *	none
 *
 * Side Effects:
 *
 *	If the device cannot be backspaced,
 *	an error message is output to  stderr  &
 *	the routine exits back to system control.
 */
  
bsf(n)
	int	n;
{
  
if (!Tape) return;
  
Mtop.mt_count	= n;
Mtop.mt_op	= MTBSF;
  
if (ioctl(fileno(Magtfp), MTIOCTOP, &Mtop) < 0) {
  
	PERROR "\n%s: %s %s\n", Progname, CANTBSF, Magtdev);
	perror(Magtdev);
	exit(FAIL);
}
}/*E bsf() */
/**/
/*
 *
 * Function:
 *
 *	dfsf
 *
 * Function Description:
 *
 *	This function "dummies" the Forward Space File operation
 *	for non-tape devices by reading thru the input data until
 *	it finds an LTF "dummy" tape mark.
 *
 * Arguments:
 *
 *	none
 *
 * Return values:
 *
 *	none
 *
 * Side Effects:
 *
 *	If an error is encountered while trying to find the
 *	dummy tape mark, an message is output to stderr and
 *	the function exits to system control.	
 */
dfsf()
{
  
FOREVER {
nottm:
	if ((read(fileno(Magtfp), Dummy, BUFSIZE))<=0) {
		PERROR "\n%s: %s %s\n", Progname, CANTFSF, Magtdev);
		perror(Magtdev);
		exit(FAIL);
	}
	if (!tape_mark(Dummy))
		goto nottm;
	else return;
  
}/*E FOREVER */	
}/*E dfsf();
/**/
/*
 *
 * Function:
 *
 *	fsf
 *
 * Function Description:
 *
 *	Forward space the output device 'n' file/tape mark(s).
 *
 * Arguments:
 *
 *	int	n		Desired number of tape/file marks to
 *				be forward spaced.
 *
 * Return values:
 *
 *	int	value		The number of actual tape/file marks
 *				skipped is returned.
 *
 * Side Effects:
 *
 *	If the device cannot be forward-spaced,
 *	an error message is output to  stderr  &
 *	the routine exits back to system control.
 *
 *	It may be necessary to back-space, in which case the
 *	bsf() routine will be called and it may push out an
 *	error message of its own.
 */
  
fsf(n)
	int	n;
{
  
/*------*\
   Code
\*------*/
  
Mtop.mt_count	= 1;
Mtop.mt_op	= MTFSF;
  
if (!Tape) {
	for (i=0; i != n; i++) {
		dfsf();
	}
	return(i);
}
for (i = 0; i != n; i++) {
	if (ioctl(fileno(Magtfp), MTIOCTOP, &Mtop) < 0) {
		PERROR "\n%s: %s %s\n", Progname, CANTFSF, Magtdev);
		perror(Magtdev);
		exit(FAIL);
	}
}/*E for i = 0; i != n; i++ */
}/*E fsf() */
/**/
/*
 *
 * Function:
 *
 *	rew
 *
 * Function Description:
 *
 *	Rewinds the output device.
 *
 * Arguments:
 *
 *	none
 *
 * Return values:
 *
 *	none
 *
 *
 * Side Effects:
 *
 *	If the device cannot be opened for reading/writing,
 *	or the rewind fails, an error message is output to
 *	stderr & the routine exits back to system control.
 */
  
rew(n)
    int n;
{
  
/*------*\
   Code
\*------*/
  
if (!Tape)
    if (Func == CREATE) 
	return;
if (n)
    if (fclose(Magtfp) < 0) {
	PERROR "\n%s: %s %s\n\n", Progname, CANTCLS, Magtdev);
	perror(Magtdev);
	exit(FAIL);
    }  
if (!(Magtfp = fopen(Magtdev, "r"))) {
    PERROR "\n%s: %s %s\n\n", Progname, CANTOPEN, Magtdev);
    perror(Magtdev);
    exit(FAIL);
}  
if (!Tape)
    return;
  
Mtop.mt_op	= MTREW;
Mtop.mt_count	= 1;

if (ioctl( fileno(Magtfp), MTIOCTOP, &Mtop) < 0) {
  
	PERROR "\n%s: %s %s\n", Progname, CANTREW, Magtdev);
	perror(Magtdev);
	exit(FAIL);
}
if (Func == CREATE)
    if (fclose(Magtfp) < 0) {
	PERROR "\n%s: %s %s\n\n", Progname, CANTCLS, Magtdev);
	perror(Magtdev);
	exit(FAIL);
    }  
}/*E rew() */
/**/
/*
 *
 * Function:
 *
 *	tape
 *
 * Function Description:
 *
 *	This function determines if the given device is,
 *	or is NOT a mag tape device.
 *
 * Arguments:
 *
 *	char	*special	Pointer to the special device name
 *
 *
 * Return values:
 *
 *	TRUE  -or-  FALSE
 *
 * Side Effects:
 *
 *	none
 */
  
tape(special)
	char	*special;
{  
struct	mtget Mtstat;		/* ioctl status structure */
  
if (!(Magtfp = fopen(special, "r"))) {
	Blocksize = BUFSIZE;
	return(FALSE);
}
if (ioctl(fileno(Magtfp), MTIOCGET, &Mtstat) < 0) {
	switch(errno) {
		case ETPL:
		case ETOL:
		case ETWL:
		case ETO:
		    break;
	 	default:
		    fclose(Magtfp);
		    Blocksize = BUFSIZE;
		    return(FALSE);
	}
}
fclose(Magtfp);
return(TRUE);
}/* tape */
/**/
/*
 *
 * Function:
 *
 *	tape_mark	
 *
 * Function Description:
 *
 *	This function tests to see if the buffer pointed to
 *	on input contains a dummy TAPE_MARK used when
 *	processing non-tape devices.
 *
 * Arguments:
 *
 *	char	*bp	Address of the buffer to test.
 *
 * Return values:
 *
 *	Returns  TRUE  or  FALSE - states whether or not the
 *	buffer contained a dummy tape mark.
 *
 * Side Effects:
 *
 *	none
 */
tape_mark(bp)
	char	*bp;
{
/*
 * If this is one of our dummy tape marks,
 * the first chctr should be a control-s.
 */
if (*bp != TM)
		return(FALSE);
	else {
		/* 
		 * The next 'n' characters should read
		 *	TAPE_MARK
		 */
		int i,j;
  
		for (++bp,i=1,j=0; j<9; bp++,i++,j++) 
			if (*bp != Tape_Mark[j])
				return(FALSE);	
		/*
		 * The remaining bytes should all be
		 * control-s characters.
		 */
		for (; i<BUFSIZE; i++,bp++)
			if (*bp != TM)
				return(FALSE);
	}
/*
 * If we got this far, we must have seen
 * one of our dummy tape marks.
 */
return(TRUE);
  
}/*E tape_mark() */
/**/
/*
 *
 * Function:
 *
 *	weof
 *
 * Function Description:
 *
 *	Writes an eof (End-Of-File) on the output device
 *
 * Arguments:
 *
 *	none
 *
 * Return values:
 *
 *	none
 *
 * Side Effects:
 *
 *	If an error is encountered writing the EOF,
 *	an error message is written to  stderr  &
 *	the routine exits to system control.
 */
  
weof()
{
  
if (!Tape) {
	int i,j;
  
	/* Write a dummy tape mark on out device so that
	 * input functions can.
	 */
	Dummy[0] = TM;
  
	for (i=1,j=0; j<9; i++,j++) 
		Dummy[i] = Tape_Mark[j];
	
	for ( ; i<=BUFSIZE; i++)
		Dummy[i] = TM;
  
	if ((write(fileno(Magtfp), Dummy, BUFSIZE))<=0)
		goto weoferr;
  
	/* Clear the buffer so that if read back our
	 * own image, we won't see ourselves as
	 * a pesudo tape mark.
	 */
	for (i=0; i <= BUFSIZE; i++)
		Dummy[i] = Spaces[i];
  
	return;
  
}/*E if !Tape */
  
/*	For  REAL  tape devices..
 */
Mtop.mt_count = 1;
Mtop.mt_op = MTWEOF;
  
if (ioctl(fileno(Magtfp), MTIOCTOP, &Mtop) < 0) {
weoferr:
	PERROR "\n%s: %s %s\n", Progname, CANTWEOF, Magtdev);
	perror(Magtdev);
	ceot();
}
}/*E weof() */
  
  
/**\\**\\**\\**\\**\\**  EOM  odm.c  **\\**\\**\\**\\**\\*/
/**\\**\\**\\**\\**\\**  EOM  odm.c  **\\**\\**\\**\\**\\*/
