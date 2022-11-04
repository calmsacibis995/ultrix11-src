
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/


/*
 *	SCCSID: @(#)constants.h	3.0	4/21/86
 *	(System V)  constants.h  1.2
 */
#define IDMAX		8	/* maximum significant identifier chars */
#define INDENTMAX	40	/* maximum left margin indentation chars */
#define LOOPMAX		20	/* statement trace buffer size */
#define SAVEMAX		80	/* parser function header and brace text storage size */
#define	STMTMAX		400	/* maximum statement text length */
#define TOKENMAX	30	/* maximum non-string token length */
#define	TRACE_DFLT	10	/* default number of traced variables */
#define TRACEMAX	20	/* maximum number of traced variables */
#undef	YYLMAX		
#define YYLMAX		STMTMAX + TOKENMAX	/* scanner line buffer size */
#define YYMAXDEPTH	300	/* yacc stack size */

#define NO_LINENO	0
#define	PP_COMMAND	"/lib/cpp -C -DCTRACE"	/* preprocessor command */
#define	RUNTIME		LIB/runtime.c"	/* run-time code package file */
