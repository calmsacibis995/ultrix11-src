
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static	char	*sccsid = "@(#)mstrcmp.c	3.0	(ULTRIX)	4/21/86";
#endif	lint

/**/
/*
 *
 *	File name:
 *
 *		mstrcmp.c
 *
 *	Source file description:
 *
 *		Compares two strings while expanding META
 *		characters (*, ?, etc).
 *		Used by the Labeled Tape Facility to match
 *		user input file names and file names read
 *		from input device.
 *
 *	Functions:
 *
 *		mstrcmp()	The comparison routine.
 *
 *	Usage:
 *
 *		n/a
 *
 *	Compile:
 *
 *	    cc -O -c mstrcmp.c		<- For Ultrix-32/32m
 *
 *	    cc CFLAGS=-DU11-O mstrcmp.c	<- For Ultrix-11
 *
 *	Modification history:
 *	~~~~~~~~~~~~~~~~~~~~
 *
 *	revision			comments
 *	--------	-----------------------------------------------
 *	  01.0		11-April-85	Ray Glaser
 *			Create initial version.
 *	
 */

 /*
  * ->	Local includes
  */

#include "ltfdefs.h"

/**/
/*
 *
 * Function:
 *
 *	mstrcmp
 *
 * Function Description:
 *
 *	Compares strings 's' and 't' while expanding metacharacters
 *	in 't'. String  's'  is the file name from tape and 't' is
 *	the user input string.
 *
 *
 * Arguments:
 *
 *	char	*s	File name string read from the input device.
 *	char	*t	File name string as input by the user.
 *
 * Return values:
 *
 *	int	0	If the two string are logically equiv.
 *	int	-1	If the two string are NOT equivalent.
 *
 *	int	Wildc	False if no wildcards were seen in the matched
 *			string, else non-zero.
 *
 * Side Effects:
 *
 *	n/a
 *	
 */

mstrcmp(s, t)
	char *s, *t;
{

/*------*\
   Code
\*------*/

/* Default to no wild cards in file name.
 */
Wildc = FALSE;

while(1) {
	if(*s == *t) {
		if(*s == 0)
			return(0);

		s++;
		t++;
		continue;

	}/*E if *s == *t ..*/

	/*
	 * Character *s != character *t.
	 * The following says  "IF" we are not at the end
	 * of user input string (t) return "no-match".
	 * -or- We are at the end of the tape name (s) and
	 * user input is "NOT" a wild *, also say "no-match".
	 */

	if ((! *t) || ((! *s) && *t != '*'))
		 return(-1);

	/*
	 * *s character != *t  character  _and_
	 * there are more characters in at
	 * least one of the two strings.
	 *
	 * Switch off of the character in the user
	 * input string.
	 */

	switch (*t) {

		case '?':
			Wildc++;
			t++;
			s++;
			break;


		case '[':

			while (*++t != ']') {
			    if (*s == *t || *t == '-'  &&
				    *s >= *(t-1) && *s <= *(t+1)) {

				s++;

				while(*t++ != ']')
					;

				break;

			    }/*T if *s == *t ..*/
			}/*E while *++t != [ */

			if (*t == ']')
			    return(-1);

			break;
	

		/* Match any string of chctrs up to the position
		 * of the wild card '*' meta character.
		 */
		 case '*':
			Wildc++;
			/* The following "while" filters out multiple
			 * consecutive occurances of the wild card
			 * "*" meta character.
			 */

			while (*++t == '*')
			    ;

			/* If we have now exhausted the user input
			 * string return a "match".
			 */
			if (! *t )
				return(0);


/* At this point:
 *
 * We have seen an '*' in usr input, and have skiped over it.
 *
 * Discard leading leading chctrs in  "s"  due  to the  "*" 
 * seen in  "t"  & find the next match of real chctrs in the two
 * strings to resume direct comparisons.
 */

			for ( ;; ) {

			    char *ss;

			    while (*s != *t) {

			    	/* If out of tape file name, say
			         * strings .ne. ..
			         */
				if (! *s )
			            return(-1);

				s++;

			    }/*E while *s != *t */

			    /*
			     * Save a pointer to the first
			     * new matching chctrs.
			     */
			    ss = s;

			    while (*s == *t) {

				/* If out of tape name. Say names
				 * are ===.?
				 */
				if (! *s )
					return(0);

				s++;
				t++;

			    }/*E while *s */

			    /* Break out of the for  ;;  loop
			     * if another meta character is seen.
			     */
			    if (*t == '*' || *t == '?' || *t == '[')
				    break;

/* Reset 's' to point to the chctr after the last match chctr
 * in the 2 strings & go back to  top-level of compare logic
 * now that we have seen a  non-match.
 *
 * The previous note and the  next 'break' serve to highlite
 * that "given" the user input of  *.dat, the code was pulling
 * file names off the tape that had any occurance of the
 * string 'dat' in the suffix.   ie. the file names:
 *
 *    elle.update,
 *    file.dat
 *    file.withdatin
 *
 * would all get dragged in from the input device when all that was
 * wanted was the file:
 *
 *    file.dat
 */
			    s = ss+1;

			    /* Break out of the for ;; loop.
			     */
			    break;

			}/*E for ;; ..*/

			/* Break out of "switch (*t)"
			*/
			break;


		/* The default for "switch (*t)" says that the
		 * non-matching character in "t" was NOT a meta
		 * character, therefor, return a non-match.
		 */
		default:
			return(-1);

	}/*E switch (*t) ..*/
}/*E while (1) ..*/

}/*E mstrcmp() ..*/


/**\\**\\**\\**\\**\\**  EOM  mstrcmp.c  **\\**\\**\\**\\**\\*/
/**\\**\\**\\**\\**\\**  EOM  mstrcmp.c  **\\**\\**\\**\\**\\*/
