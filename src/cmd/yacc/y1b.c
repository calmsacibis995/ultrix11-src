
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)y1b.c	3.0	4/22/86";
/* Beginning of stuff added when y1.c was split up */

/* This used to be the tail end of y1.c.  It was moved to here */
/* so that closure() could be put into an overlay, thus making */
/* the base segment small enough to fit in one APR */

#include "dextern"

extern int tbitset;
extern struct looksets clset;
extern struct looksets *pfirst[];
extern int pempty[];
extern int **pres[];
extern struct wset *zzcwp;
extern int nlset;

/* end of stuff added when y1.c was split up */

int cldebug = 0; /* debugging flag for closure */
closure(i){ /* generate the closure of state i */

	int c, ch, work, k;
	register struct wset *u, *v;
	int *pi;
	int **s, **t;
	struct item *q;
	register struct item *p;

	++zzclose;

	/* first, copy kernel of state i to wsets */

	cwp = wsets;
	ITMLOOP(i,p,q){
		cwp->pitem = p->pitem;
		cwp->flag = 1;    /* this item must get closed */
		SETLOOP(k) cwp->ws.lset[k] = p->look->lset[k];
		WSBUMP(cwp);
		}

	/* now, go through the loop, closing each item */

	work = 1;
	while( work ){
		work = 0;
		WSLOOP(wsets,u){
	
			if( u->flag == 0 ) continue;
			c = *(u->pitem);  /* dot is before c */
	
			if( c < NTBASE ){
				u->flag = 0;
				continue;  /* only interesting case is where . is before nonterminal */
				}
	
			/* compute the lookahead */
			aryfil( clset.lset, tbitset, 0 );

			/* find items involving c */

			WSLOOP(u,v){
				if( v->flag == 1 && *(pi=v->pitem) == c ){
					v->flag = 0;
					if( nolook ) continue;
					while( (ch= *++pi)>0 ){
						if( ch < NTBASE ){ /* terminal symbol */
							SETBIT( clset.lset, ch );
							break;
							}
						/* nonterminal symbol */
						setunion( clset.lset, pfirst[ch-NTBASE]->lset );
						if( !pempty[ch-NTBASE] ) break;
						}
					if( ch<=0 ) setunion( clset.lset, v->ws.lset );
					}
				}
	
			/*  now loop over productions derived from c */
	
			c -= NTBASE; /* c is now nonterminal number */
	
			t = pres[c+1];
			for( s=pres[c]; s<t; ++s ){
				/* put these items into the closure */
				WSLOOP(wsets,v){ /* is the item there */
					if( v->pitem == *s ){ /* yes, it is there */
						if( nolook ) goto nexts;
						if( setunion( v->ws.lset, clset.lset ) ) v->flag = work = 1;
						goto nexts;
						}
					}
	
				/*  not there; make a new entry */
				if( cwp-wsets+1 >= WSETSIZE ) error( "working set overflow" );
				cwp->pitem = *s;
				cwp->flag = 1;
				if( !nolook ){
					work = 1;
					SETLOOP(k) cwp->ws.lset[k] = clset.lset[k];
					}
				WSBUMP(cwp);
	
			nexts: ;
				}
	
			}
		}

	/* have computed closure; flags are reset; return */

	if( cwp > zzcwp ) zzcwp = cwp;
	if( cldebug && (foutput!=NULL) ){
		fprintf( foutput, "\nState %d, nolook = %d\n", i, nolook );
		WSLOOP(wsets,u){
			if( u->flag ) fprintf( foutput, "flag set!\n");
			u->flag = 0;
			fprintf( foutput, "\t%s", writem(u->pitem));
			prlook( &u->ws );
			fprintf( foutput,  "\n" );
			}
		}
	}

struct looksets *flset( p )   struct looksets *p; {
	/* decide if the lookahead set pointed to by p is known */
	/* return pointer to a perminent location for the set */

	register struct looksets *q;
	int j, *w;
	register *u, *v;

	for( q = &lkst[nlset]; q-- > lkst; ){
		u = p->lset;
		v = q->lset;
		w = & v[tbitset];
		while( v<w) if( *u++ != *v++ ) goto more;
		/* we have matched */
		return( q );
		more: ;
		}
	/* add a new one */
	q = &lkst[nlset++];
	if( nlset >= LSETSIZE )error("too many lookahead sets" );
	SETLOOP(j){
		q->lset[j] = p->lset[j];
		}
	return( q );
	}
