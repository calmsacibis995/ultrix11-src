
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)ureg.c	3.2	3/4/87
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/text.h>
#include <sys/seg.h>

/*
 * Load the user hardware segmentation
 * registers from the software prototype.
 * The software registers must have
 * been setup prior by estabur.
 */
sureg()
{
	register *udp, *uap, *rdp;
	int *rap, *limudp;
	int taddr, daddr;
	struct text *tp;

	taddr = daddr = u.u_procp->p_addr;
	if ((tp=u.u_procp->p_textp) != NULL)
		taddr = tp->x_caddr;
	limudp = &u.u_uisd[16];
	if (!sepid)
		limudp = &u.u_uisd[8];
	rap = (int *)UISA;
	rdp = (int *)UISD;
	uap = &u.u_uisa[0];
	for (udp = &u.u_uisd[0]; udp < limudp;) {
		*rap++ = *uap++ + (*udp&TX? taddr: (*udp&ABS? 0: daddr));
		*rdp++ = *udp++;
	}
}

/*
 * Set up software prototype segmentation
 * registers to implement the 3 pseudo
 * text,data,stack segment sizes passed
 * as arguments.
 * The argument sep specifies if the
 * text and data+stack segments are to
 * be separated.
 * The last argument determines whether the text
 * segment is read-write or read-only.
 */
estabur(nt, nd, ns, sep, xrw)
unsigned nt, nd, ns;
{
	register a, *ap, *dp;
	register unsigned ts;
	register novlseg;
	int last,first;
	int *limudp;
	char bitm;

	if(u.u_ovdata.uo_ovbase && nt)
		ts = u.u_ovdata.uo_dbase - 1;
	else
		ts = nt;
	last = 0;
	first = 8;
	for(bitm=u.u_mbitm; bitm; bitm <<= 1 ) {
		--first;
		if(bitm < 0 && last == 0)
			last = first;
	}

	if(sep) {
#ifndef	NONSEPERATE
		if (!sepid)
			goto err;
		if(ctos(nd) > first || 8-ctos(ns) <= last) { 
			goto err;
		}
		if(ctos(ts) > 8 || ctos(nd)+ctos(ns) > 8)
#endif	NONSEPERATE
			goto err;
	} else {
		if (ctos(ts) + ctos(nd) > first || 8-ctos(ns) <= last) {
			goto err;
		}
		if(ctos(ts)+ctos(nd)+ctos(ns) > 8)
			goto err;
	}
	if(u.u_ovdata.uo_ovbase && nt)
		ts = u.u_ovdata.uo_ov_offst[7];
	if(ts+nd+ns+USIZE > maxmem)
		goto err;
	a = 0;
	ap = &u.u_uisa[0];
	dp = &u.u_uisd[0];
	while(nt >= 128) {
		*dp++ = (127<<8) | xrw|TX;
		*ap++ = a;
		a += 128;
		nt -= 128;
	}
	if(nt) {
		*dp++ = ((nt-1)<<8) | xrw|TX;
		*ap++ = a;
	}
	if(u.u_ovdata.uo_ovbase && nt){ /* overlay process adjust accord */
		novlseg = 0;
		if(u.u_ovdata.uo_curov != 0){ /* map in current overlay */
			a = u.u_ovdata.uo_ov_offst[u.u_ovdata.uo_curov-1];
			nt = u.u_ovdata.uo_ov_offst[u.u_ovdata.uo_curov] - a;
			while(nt >= 128){
				*dp++ = (127<<8) | xrw|TX;
				*ap++ = a;
				a += 128;
				nt -= 128;
				novlseg++;
			}
			if(nt){
				*dp++ = ((nt-1)<<8)|xrw|TX;
				*ap++ = a;
				novlseg++;
			}
		}
#ifndef	NONSEPERATE
		if(!sep)
#endif NONSEPERATE
			for(; novlseg < u.u_ovdata.uo_nseg; novlseg++){
				*ap++ = 0;
				*dp++ = 0;
			}
	}
#ifndef	NONSEPERATE
	if(sep)
	while(ap < &u.u_uisa[8]) {
		*ap++ = 0;
		*dp++ = 0;
	}
#endif	NONSEPERATE
	a = USIZE;
	while(nd >= 128) {
		*dp++ = (127<<8) | RW;
		*ap++ = a;
		a += 128;
		nd -= 128;
	}
	if(nd) {
		*dp++ = ((nd-1)<<8) | RW;
		*ap++ = a;
		a += nd;
	}
	while(ap < &u.u_uisa[8]) {
		if(*dp &ABS) {
			dp++;
			ap++;
			continue;
		}
		*dp++ = 0;
		*ap++ = 0;
	}
#ifndef	NONSEPERATE
	if(sep)
	while(ap < &u.u_uisa[16]) {
		if(*dp & ABS) {
			dp++;
			ap++;
			continue;
		}
		*dp++ = 0;
		*ap++ = 0;
	}
#endif	NONSEPERATE
	a += ns;
	while(ns >= 128) {
		a -= 128;
		ns -= 128;
		*--dp = (127<<8) | RW;
		*--ap = a;
	}
	if(ns) {
		*--dp = ((128-ns)<<8) | RW | ED;
		*--ap = a-128;
	}
	if(!sep) {
		ap = &u.u_uisa[0];
		dp = &u.u_uisa[8];
		while(ap < &u.u_uisa[8])
			*dp++ = *ap++;
		ap = &u.u_uisd[0];
		dp = &u.u_uisd[8];
		while(ap < &u.u_uisd[8])
			*dp++ = *ap++;
	}
	sureg();
	return(0);

err:
	u.u_error = ENOMEM;
	return(-1);
}
