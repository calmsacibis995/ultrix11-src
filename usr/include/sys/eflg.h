 
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985.	      *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/include/COPYRIGHT" for applicable restrictions.  *
 **********************************************************************/

/*
 * SCCSID: @(#)eflg.h	3.0	5/21/86
 */

/*
 * Event flag header file
 * ULTRIX-11 
 * Bill Burns		2/20/84
 *
 */
/*
 * The modes below line up with the 
 * file protection modes for sanity.
 * 
 * unspecififed bits are ignored.
 * a mode of zero is illegal and will
 *    cause "invalid arg".
 */

/* modes */
#define WOW	2	/* allow world write */
#define WOR	4	/* allow world read  */
#define GRW	16	/* allow group write */
#define GRR	32	/* allow group read  */
#define OWW	128	/* allow owner write */
#define OWR	256	/* allow owner read  */

/* event flag syscall codes */
#define	EFREQ	01	/* request a set of 32 event flags */
#define	EFRD	02	/* read a set of event flags */
#define	EFWRT	04	/* write a set of event flags */
#define	EFREL	010	/* release a set of event flags */
#define	EFCLR	020	/* clear a bit */
#define	EFSET	040	/* set a bit */
