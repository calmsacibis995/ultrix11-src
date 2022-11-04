/ SCCSID: @(#)setjmp.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library -- setjmp, longjmp

/	longjmp(a,v)
/ will generate a "return(v)" from
/ the last call to
/	setjmp(a)
/ by restoring sp, r5, pc from `a'
/ and doing a return.
/

.globl	_setjmp
.globl	_longjmp
.globl	csv, cret
.globl	__ovno

_setjmp:
	mov	2(sp),r0
	mov	r5,(r0)+
	mov	sp,(r0)+
	mov	(sp),(r0)+	/
	mov	__ovno,(r0)	/
	clr	r0
	rts	pc

_longjmp:
	jsr	r5,csv		/ make our stack frame
	mov	4(r5),r1	/ jmp_buf address -> r1
	mov	6(r5),r0	/ return value -> r0
	bne	1f
	mov	$1,r0		/ return value != 0 !!!!
1:
	/ unwind the stack until we hit the frame one above
	/ the context we are returning to.  If we hit 0, we
	/ must have returned from the function that called
	/ setjmp(), or else the stack is thrashed.  If the
	/ stack is thrashed, it's more likely that we'll
	/ memory fault before then...
	cmp	(r5),(r1)
	beq	2f
	mov	(r5),r5
	bne	1b
/ panic -- r2-r4 lost, the stack is thrashed
	br	1f
2:	
	/ ok, we now need to re-create the stack frame so
	/ that we can do a cret.  We'll either have to copy
	/ the frame to another location, or if we are lucky
	/ it'll already be in the right place.
	mov	2(r1),r2	/ grab top of new frame
	tst	(r2)+		/ bump it to new r5 location
	cmp	r5,2(r1)	/ compare addr of old and new frames
	blt 7f
	bgt 8f
	/ real easy, the frame is in the right place!
	mov	4(r1),2(r5)	/ copy saved pc from jmpbuf
	mov	6(r1),-2(r5)	/ copy saved __ovno from jmpbuf
	jmp	cret		/ let cret do the rest.
7:
	/ we are copying the frame up into higher memory
	mov	r5,r2		/ grap old frame pointer and
	tst	-(r2)		/  skip over __ovno
	mov	2(r1),r3	/ grap top of new frame and
	cmp	-(r3),-(r3)	/  skip over where r5 & __ovno will go
	mov	-(r2),-(r3)	/ copy r4
	mov	-(r2),-(r3)	/ copy r3
	mov	-(r2),-(r3)	/ copy r2
1:
	mov	2(r1),r3	/ get top of new frame
	mov	4(r1),(r3)	/ copy saved pc value from jmpbuf
	mov	(r1),-(r3)	/ copy saved r5 from jmpbuf
	mov	6(r1), -2(r3)	/ copy saved __ovno from jmpbuf
	mov	r3, r5		/ set up new r5 value
	jmp	cret		/  and cret does the rest.
8:
	/ we have to move the frame down, (probably only 1 word)
	mov	r5, r2		/ get old frame pointer and bump
	add	8., r2		/   it down to the old r2 value
	mov	2(r1), r3	/ grab top of new frame and bump it
	add	10., r3		/   to where the new r2 value will go
	mov	(r2)+, (r3)+	/ copy r2
	mov	(r2)+, (r3)+	/ copy r3
	mov	(r2)+, (r3)+	/ copy r4
	br	1b
