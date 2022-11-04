
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)condefs.c	3.0	4/22/86";

/************************* 
 * definitions for dialer routines
 *************************/


#include "uucp.h"
extern int nulldev(), nodev(), Acuopn(), diropn(), dircls();

#ifdef DATAKIT
int dkopn();
#endif
#ifdef DN11
int dnopn(), dncls();
#endif
#ifdef HAYES
int hysopn(), hyscls();
#endif
#ifdef HAYESQ
int hysqopn(), hysqcls();  /* a version of hayes that doesn't use ret codes */
#endif
#ifdef DF0
int df0opn(), df0cls();
#endif
#ifdef DF1
int df1opn(), df1cls();
#endif
#ifdef PNET
int pnetopn();
#endif
#ifdef VENTEL
int ventopn(), ventcls();
#endif
#ifdef	UNET
#include <UNET/unetio.h>
#include <UNET/tcp.h>
int unetopn(), unetcls();
#endif UNET
#ifdef VADIC
int vadopn(), vadcls();
#endif VADIC
#ifdef	RVMACS
int rvmacsopn(), rvmacscls();
#endif
#ifdef MICOM
int micopn(), miccls();
#endif MICOM

struct condev condevs[] = {
{ "DIR", "direct", diropn, nulldev, dircls },
#ifdef DATAKIT
{ "DK", "datakit", dkopn, nulldev, nulldev },
#endif
#ifdef PNET
{ "PNET", "pnet", pnetopn, nulldev, nulldev },
#endif
#ifdef	UNET
{ "UNET", "UNET", unetopn, nulldev, unetcls },
#endif UNET
#ifdef MICOM
{ "MICOM", "micom", micopn, nulldev, miccls },
#endif MICOM
#ifdef DN11
{ "ACU", "dn11", Acuopn, dnopn, dncls },
#endif
#ifdef HAYES
{ "ACU", "hayes", Acuopn, hysopn, hyscls },
#endif HAYES
#ifdef HAYESQ	/* a version of hayes that doesn't use result codes */
{ "ACU", "hayesq", Acuopn, hysqopn, hysqcls },
#endif HAYESQ
#ifdef DF0
{ "ACU", "DF02", Acuopn, df0opn, df0cls },
{ "ACU", "DF03", Acuopn, df0opn, df0cls },
#endif
#ifdef DF1
{ "ACU", "DF112", Acuopn, df1opn, df1cls },
{ "ACU", "DF224", Acuopn, df1opn, df1cls },
#endif
#ifdef VENTEL
{ "ACU", "ventel", Acuopn, ventopn, ventcls },
#endif VENTEL
#ifdef VADIC
{ "ACU", "vadic", Acuopn, vadopn, vadcls },
#endif VADIC
#ifdef RVMACS
{ "ACU", "rvmacs", Acuopn, rvmacsopn, rvmacscls },
#endif RVMACS

/* Insert new entries before this line */
{ NULL, NULL, NULL, NULL, NULL } };
