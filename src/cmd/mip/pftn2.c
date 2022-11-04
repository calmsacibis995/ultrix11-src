
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)pftn2.c	3.0	4/21/86";
# include "mfile1"

extern int ddebug;

uclass( class ) register class; {
	/* give undefined version of class */
	if( class == SNULL ) return( EXTERN );
	else if( class == STATIC ) return( USTATIC );
	else if( class == FORTRAN ) return( UFORTRAN );
	else return( class );
	}

fixclass( class, type ) TWORD type; {

	/* first, fix null class */

	if( class == SNULL ){
		if( instruct&INSTRUCT ) class = MOS;
		else if( instruct&INUNION ) class = MOU;
		else if( blevel == 0 ) class = EXTDEF;
		else if( blevel == 1 ) class = PARAM;
		else class = AUTO;

		}

	/* now, do general checking */

	if( ISFTN( type ) ){
		switch( class ) {
		default:
			uerror( "function has illegal storage class" );
		case AUTO:
			class = EXTERN;
		case EXTERN:
		case EXTDEF:
		case FORTRAN:
		case TYPEDEF:
		case STATIC:
		case UFORTRAN:
		case USTATIC:
			;
			}
		}

	if( class&FIELD ){
		if( !(instruct&INSTRUCT) ) uerror( "illegal use of field" );
		return( class );
		}

	switch( class ){

	case MOU:
		if( !(instruct&INUNION) ) uerror( "illegal class" );
		return( class );

	case MOS:
		if( !(instruct&INSTRUCT) ) uerror( "illegal class" );
		return( class );

	case MOE:
		if( instruct & (INSTRUCT|INUNION) ) uerror( "illegal class" );
		return( class );

	case REGISTER:
		if( blevel == 0 ) uerror( "illegal register declaration" );
		else if( regvar >= MINRVAR && cisreg( type ) ) return( class );
		if( blevel == 1 ) return( PARAM );
		else return( AUTO );

	case AUTO:
	case LABEL:
	case ULABEL:
		if( blevel < 2 ) uerror( "illegal class" );
		return( class );

	case PARAM:
		if( blevel != 1 ) uerror( "illegal class" );
		return( class );

	case UFORTRAN:
	case FORTRAN:
# ifdef NOFORTRAN
			NOFORTRAN;    /* a condition which can regulate the FORTRAN usage */
# endif
		if( !ISFTN(type) ) uerror( "fortran declaration must apply to function" );
		else {
			type = DECREF(type);
			if( ISFTN(type) || ISARY(type) || ISPTR(type) ) {
				uerror( "fortran function has wrong type" );
				}
			}
	case STNAME:
	case UNAME:
	case ENAME:
	case EXTERN:
	case STATIC:
	case EXTDEF:
	case TYPEDEF:
	case USTATIC:
		return( class );

	default:
		cerror( "illegal class: %d", class );
		/* NOTREACHED */

		}
	}

lookup( name, s) char *name; { 
	/* look up name: must agree with s w.r.t. SMOS and SHIDDEN */

	register char *p, *q;
	int i, j, ii;
	register struct symtab *sp;

	/* compute initial hash index */
	if( ddebug > 2 ){
		printf( "lookup( %s, %d ), stwart=%d, instruct=%d\n", name, s, stwart, instruct );
		}

	i = 0;
	for( p=name, j=0; *p != '\0'; ++p ){
		i += *p;
		if( ++j >= NCHNAM ) break;
		}
	i = i%SYMTSZ;
	sp = &stab[ii=i];

	for(;;){ /* look for name */

		if( sp->stype == TNULL ){ /* empty slot */
			p = sp->sname;
			sp->sflags = s;  /* set SMOS if needed, turn off all others */
			for( j=0; j<NCHNAM; ++j ) if( *p++ = *name ) ++name;
			sp->stype = UNDEF;
			sp->sclass = SNULL;
			return( i );
			}
		if( (sp->sflags & (SMOS|SHIDDEN)) != s ) goto next;
		p = sp->sname;
		q = name;
		for( j=0; j<NCHNAM;++j ){
			if( *p++ != *q ) goto next;
			if( !*q++ ) break;
			}
		return( i );
	next:
		if( ++i >= SYMTSZ ){
			i = 0;
			sp = stab;
			}
		else ++sp;
		if( i == ii ) cerror( "symbol table full" );
		}
	}

#ifndef checkst
/* if not debugging, make checkst a macro */
checkst(lev){
	register int s, i, j;
	register struct symtab *p, *q;

	for( i=0, p=stab; i<SYMTSZ; ++i, ++p ){
		if( p->stype == TNULL ) continue;
		j = lookup( p->sname, p->sflags&SMOS );
		if( j != i ){
			q = &stab[j];
			if( q->stype == UNDEF ||
			    q->slevel <= p->slevel ){
				cerror( "check error: %.8s", q->sname );
				}
			}
		else if( p->slevel > lev ) cerror( "%.8s check at level %d", p->sname, lev );
		}
	}
#endif

struct symtab *
relook(p) register struct symtab *p; {  /* look up p again, and see where it lies */

	register struct symtab *q;

	/* I'm not sure that this handles towers of several hidden definitions in all cases */
	q = &stab[lookup( p->sname, p->sflags&(SMOS|SHIDDEN) )];
	/* make relook always point to either p or an empty cell */
	if( q->stype == UNDEF ){
		q->stype = TNULL;
		return(q);
		}
	while( q != p ){
		if( q->stype == TNULL ) break;
		if( ++q >= &stab[SYMTSZ] ) q=stab;
		}
	return(q);
	}

clearst( lev ){ /* clear entries of internal scope  from the symbol table */
	register struct symtab *p, *q, *r;
	register int temp, rehash;

	temp = lineno;
	aobeg();

	/* first, find an empty slot to prevent newly hashed entries from
	   being slopped into... */

	for( q=stab; q< &stab[SYMTSZ]; ++q ){
		if( q->stype == TNULL )goto search;
		}

	cerror( "symbol table full");

	search:
	p = q;

	for(;;){
		if( p->stype == TNULL ) {
			rehash = 0;
			goto next;
			}
		lineno = p->suse;
		if( lineno < 0 ) lineno = - lineno;
		if( p->slevel>lev ){ /* must clobber */
			if( p->stype == UNDEF || ( p->sclass == ULABEL && lev < 2 ) ){
				lineno = temp;
				uerror( "%.8s undefined", p->sname );
				}
			else aocode(p);
			if (ddebug) printf("removing %8s from stab[ %d], flags %o level %d\n",
				p->sname,p-stab,p->sflags,p->slevel);
			if( p->sflags & SHIDES ) unhide(p);
			p->stype = TNULL;
			rehash = 1;
			goto next;
			}
		if( rehash ){
			if( (r=relook(p)) != p ){
				movestab( r, p );
				p->stype = TNULL;
				}
			}
		next:
		if( ++p >= &stab[SYMTSZ] ) p = stab;
		if( p == q ) break;
		}
	lineno = temp;
	aoend();
	}

movestab( p, q ) register struct symtab *p, *q; {
	int k;
	/* structure assignment: *p = *q; */
	p->stype = q->stype;
	p->sclass = q->sclass;
	p->slevel = q->slevel;
	p->offset = q->offset;
	p->sflags = q->sflags;
	p->dimoff = q->dimoff;
	p->sizoff = q->sizoff;
	p->suse = q->suse;
	for( k=0; k<NCHNAM; ++k ){
		p->sname[k] = q->sname[k];
		}
	}

hide( p ) register struct symtab *p; {
	register struct symtab *q;
	for( q=p+1; ; ++q ){
		if( q >= &stab[SYMTSZ] ) q = stab;
		if( q == p ) cerror( "symbol table full" );
		if( q->stype == TNULL ) break;
		}
	movestab( q, p );
	p->sflags |= SHIDDEN;
	q->sflags = (p->sflags&SMOS) | SHIDES;
	if( hflag ) werror( "%.8s redefinition hides earlier one", p->sname );
	if( ddebug ) printf( "	%d hidden in %d\n", p-stab, q-stab );
	return( idname = q-stab );
	}

unhide( p ) register struct symtab *p; {
	register struct symtab *q;
	register s, j;

	s = p->sflags & SMOS;
	q = p;

	for(;;){

		if( q == stab ) q = &stab[SYMTSZ-1];
		else --q;

		if( q == p ) break;

		if( (q->sflags&SMOS) == s ){
			for( j =0; j<NCHNAM; ++j ) if( p->sname[j] != q->sname[j] ) break;
			if( j == NCHNAM ){ /* found the name */
				q->sflags &= ~SHIDDEN;
				if( ddebug ) printf( "unhide uncovered %d from %d\n", q-stab,p-stab);
				return;
				}
			}

		}
	cerror( "unhide fails" );
	}
