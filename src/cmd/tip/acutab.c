
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static char sccsid[] = "@(#)acutab.c	3.0	4/22/86";
#endif

#include "tip.h"

extern int df02_dialer(), 
	   df03_dialer(), df_disconnect(), df_abort(),
	   d112p_dialer(), d112t_dialer(),
	   d224p_dialer(), d224t_dialer(), dff_dconnect(), dff_abort(),
	   biz31f_dialer(), biz31_disconnect(), biz31_abort(),
	   biz31w_dialer(),
	   biz22f_dialer(), biz22_disconnect(), biz22_abort(),
	   biz22w_dialer(),
	   ven_dialer(), ven_disconnect(), ven_abort(),
	   v3451ialer(), v3451isconnect(), v3451_abort(),
	   v831dialer(), v831disconnect(), v831_abort(),
	   dn_dialer(), dn_disconnect(), dn_abort();

acu_t acutable[] = {
#if BIZ1031
	"biz31f", biz31f_dialer, biz31_disconnect,	biz31_abort,
	"biz31w", biz31w_dialer, biz31_disconnect,	biz31_abort,
#endif
#if BIZ1022
	"biz22f", biz22f_dialer, biz22_disconnect,	biz22_abort,
	"biz22w", biz22w_dialer, biz22_disconnect,	biz22_abort,
#endif
#if DF02
	"df02",	df02_dialer,	df_disconnect,		df_abort,
#endif
#if DF03
	"df03",	df03_dialer,	df_disconnect,		df_abort,
#endif
#if DF112
	"df112", d112p_dialer,	dff_dconnect,		dff_abort,
	"df112p", d112p_dialer, dff_dconnect,		dff_abort,
	"df112t", d112t_dialer, dff_dconnect,		dff_abort,
#endif
#if DF224
	"df224", d224p_dialer,	dff_dconnect,		dff_abort,
	"df224p", d224p_dialer, dff_dconnect,		dff_abort,
	"df224t", d224t_dialer, dff_dconnect,		dff_abort,
#endif
#if DN11
	"dn11",	dn_dialer,	dn_disconnect,		dn_abort,
#endif
#ifdef VENTEL
	"ventel",ven_dialer,	ven_disconnect,		ven_abort,
#endif
#ifdef V3451
#ifndef V831
	"vadic",v3451ialer,	v3451isconnect,	v3451_abort,
#endif
	"v3451",v3451ialer,	v3451isconnect,	v3451_abort,
#endif
#ifdef V831
#ifndef V3451
	"vadic",v831dialer,	v831disconnect,	v831_abort,
#endif
	"v831",v831dialer,	v831disconnect,	v831_abort,
#endif
	0,	0,		0,			0
};
